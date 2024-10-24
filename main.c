#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
//#include <time.h>
#include <sys/time.h>

#include "parser.h"


#define STACK_SIZE 1024
#define STACK_TYPE size_t // Should be big enough to store machines ptr

#define HEAP_SIZE 128

STACK_TYPE stack[STACK_SIZE];
STACK_TYPE stackSize = 0;

const size_t stackCapacity = STACK_SIZE;

#define OK         0
#define ERROR     -1

#define DEBUG 1

STACK_TYPE pushStack(STACK_TYPE value){
	if(stackSize + 1 >= stackCapacity){
		return ERROR; 
	}
	stack[stackSize++] = value;
	return OK;
}

STACK_TYPE popStack() {
	if (stackSize == 0) {
	    printf("Stack underflow error!\n");
	    exit(-1);
	}
	STACK_TYPE temp = stack[--stackSize];
	stack[stackSize] = 0;
	return temp;
}


STACK_TYPE* makePtr(STACK_TYPE value){
	STACK_TYPE* ptr = (STACK_TYPE*)malloc(sizeof(STACK_TYPE));
	*ptr = value;
	return ptr;
}


STACK_TYPE fromPtr(STACK_TYPE ptrValue){
	STACK_TYPE* t = (STACK_TYPE*)ptrValue;
	return *t;
}


/* INSTRUCTION SET */
void add(){
	pushStack(popStack() + popStack());
}

void sub(){

    pushStack(popStack() - popStack());
}

void mult(){
	pushStack(popStack() * popStack());
}


void not(){
	pushStack(!popStack());
}

void cmp(){
	sub();
	not();
}

void dup(){
	STACK_TYPE temp = popStack();
	pushStack(temp);
	pushStack(temp);
}

void sswitch(){
	STACK_TYPE a = popStack();
	STACK_TYPE b = popStack();
	pushStack(a);
	pushStack(b);
}

void inc(){
	pushStack(popStack()+1);
}

void dec(){
	pushStack(popStack()-1);
}

void rot(){
	// two pointer method so theres no need for second array
	size_t lptr = 0;
	size_t rptr = stackSize-1;
    while (lptr < rptr) {
        STACK_TYPE temp = stack[lptr];
        stack[lptr] = stack[rptr];
        stack[rptr] = temp;
        lptr++;
        rptr--;
    }
}

void over(){
	STACK_TYPE a = popStack();
	STACK_TYPE b = popStack();
	pushStack(a);
	pushStack(b);
	pushStack(a);
}

/* INSTRUCTION SET END */
int64_t findInHeap(HeapObject* heap, size_t heapSize, char* toFind){
	for(size_t i = 0; i < heapSize; i++){
		if(strcmp(heap[i].data, toFind) == 0){
			return i;
		}
	}
	return -1;
}

void interpreter(Token* tokens, int* tokenCount){
	size_t stackPointer = 0;
	struct timeval  tv1, tv2;
	STACK_TYPE cache;
	Token t;
	size_t iters = 0;
	HeapObject* heap = (HeapObject*)malloc(HEAP_SIZE*sizeof(HeapObject));
	size_t heapSize = 0;

	gettimeofday(&tv1, NULL);
	while(stackPointer < *tokenCount){
		/*printf("[ ");
		for(int i = 0; i < stackSize; i++){
			printf("%d, ", stack[i]);
		}
		printf("]\n");*/
		//printf("%d\n", stackPointer);

		iters++;
		t = tokens[stackPointer];
		if(t.keywordType == PUSH){
			assert(tokens[stackPointer+1].type == TOKEN_NUMBER && "Programming error: next token isnt a number!");
			pushStack(atoi(tokens[stackPointer+1].data));
			stackPointer+=2;
			continue;
		}
		if(t.keywordType == POP){popStack();}
		if(t.keywordType == ADD){add();}
		if(t.keywordType == SUB){sub();}
		if(t.keywordType == CMP){cmp();}
		if(t.keywordType == DEC){dec();}
		if(t.keywordType == INC){inc();}
		if(t.keywordType == NOT){not();}

		if(t.keywordType == LOAD){
			int64_t index = findInHeap(heap, heapSize, tokens[stackPointer+1].data);
			assert(index != -1 && "Programming error: cannot load value that hasn't been stored");
			pushStack(*((STACK_TYPE*)heap[index].ptr));
		}
		if(t.keywordType == LOADF){
    		int64_t index = findInHeap(heap, heapSize, tokens[stackPointer + 1].data);
    		assert(index != -1 && "Programming error: cannot loadf value that hasn't been stored");
    		pushStack(*((STACK_TYPE*)heap[index].ptr));
		    free((STACK_TYPE*)heap[index].ptr);
		    free(heap[index].data);
		    for (size_t i = index; i < heapSize - 1; i++) {
    		    heap[i] = heap[i + 1];
    		}
		    heapSize--;
		}
		if(t.keywordType == STORE){
			int64_t index = findInHeap(heap, heapSize, tokens[stackPointer+1].data);
			if(index == -1){
				STACK_TYPE* ptr = makePtr(popStack());
				HeapObject ho;
				ho.data = tokens[stackPointer+1].data;
				ho.ptr = (STACK_TYPE) ptr;
				heap[heapSize++] = ho;
			}else{
				*((STACK_TYPE*)heap[index].ptr) = popStack();
			}
		}

		if(t.keywordType == SWITCH){sswitch();}
		if(t.keywordType == ROT){rot();}
		if(t.keywordType == OVER){over();}
		if(t.keywordType == DUP){dup();}
		if(t.keywordType == OUT){
			printf("%d\n", popStack());
		}
		if(t.keywordType == OUTC){
			printf("%c", popStack());
		}
		if(t.keywordType == JNZ){
			assert(tokens[stackPointer+1].type == TOKEN_NUMBER && "Programming error: next token isnt a number!");
			if(popStack() != 0){
				stackPointer = atoi(tokens[stackPointer+1].data);
				continue;
			}
		}
		if(t.keywordType == JMP){
			assert(tokens[stackPointer+1].type == TOKEN_NUMBER && "Programming error: next token isnt a number!");
			stackPointer = atoi(tokens[stackPointer+1].data);
			continue;
		}
		stackPointer++;
	}
	gettimeofday(&tv2, NULL);
	if(DEBUG == 1){
		double seconds = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
		printf("Total time = %f seconds\n",seconds);
		printf("program took %d iterations\n", iters);
		printf("program ran in the rate of %f it/s\n", iters/seconds);
	}
}



int main(int argc, char* argv[]){
	assert(argc == 2 && "You must pass stc file in arguments");
	char tokens[512][256];
    int tokenCount = 0;
	parseSrc(readSrcFile(argv[argc-1]), tokens, &tokenCount);
	Token* values = tokenize(tokens, &tokenCount);
	if(DEBUG == 1){
		for (int i = 0; i < tokenCount; i++) {
    		printf("Token %d: Type = %d, Data = %s keywordType=%d\n", i, (values + i)->type, (values + i)->data, (values + i)->keywordType);
		}
	}
	interpreter(values, &tokenCount);
	/*
	printf("%d %d %d %d %d %d %d %d",
	 stack[0],
	 stack[1],
	 stack[2],
	 stack[3],
	 stack[4],
	 stack[5],
	 stack[6],
	 stack[7]);
	 */
	return 0;
}