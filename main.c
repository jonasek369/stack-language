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


#define STACK_SIZE 256
#define STACK_TYPE size_t

STACK_TYPE stack[STACK_SIZE];
STACK_TYPE stackSize = 0;

const size_t stackCapacity = STACK_SIZE;

#define OK         0
#define ERROR     -1


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


STACK_TYPE makePtr(STACK_TYPE value){
	STACK_TYPE* ptr = (STACK_TYPE*)malloc(sizeof(STACK_TYPE));
	*ptr = value;
	return (STACK_TYPE)ptr;
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
    STACK_TYPE a = popStack();
    STACK_TYPE b = popStack();
    pushStack(a - b);
}

void mult(){
	pushStack(popStack() * popStack());
}

void cmp(){
	sub();
	pushStack(!popStack());
}

void dup(){
	STACK_TYPE temp = popStack();
	pushStack(temp);
	pushStack(temp);
}

void store(){
	pushStack((STACK_TYPE)makePtr(popStack()));
}

void load(){
	pushStack(fromPtr(popStack()));
}

// load free (we free the ptr)
void loadf(){
	STACK_TYPE ptr = popStack();
	pushStack(fromPtr(ptr));
	free((STACK_TYPE*)ptr);
}

void sswitch(){
	STACK_TYPE a = popStack();
	STACK_TYPE b = popStack();
	pushStack(a);
	pushStack(b);
}

void not(){
	pushStack(!popStack());
}

void inc(){
	pushStack(popStack()+1);
}

void dec(){
	pushStack(popStack()-1);
}

void rot(){
	STACK_TYPE aux[stackSize];
	STACK_TYPE currentStackSize = stackSize;
	for(int i = 0; i < currentStackSize; i++){
		aux[i] = popStack();
	}
	for (int i = 0; i < currentStackSize; i++) {
        pushStack(aux[i]);
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


void interpreter(Token* tokens, int* tokenCount){
	size_t stackPointer = 0;
	struct timeval  tv1, tv2;
	gettimeofday(&tv1, NULL);
	while(stackPointer < *tokenCount){
		Token t = tokens[stackPointer];
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
		if(t.keywordType == LOAD){load();}
		if(t.keywordType == LOADF){loadf();}
		if(t.keywordType == SWITCH){sswitch();}
		if(t.keywordType == STORE){store();}
		if(t.keywordType == ROT){rot();}
		if(t.keywordType == OVER){over();}
		if(t.keywordType == DUP){dup();}
		if(t.keywordType == JNZ){
			assert(tokens[stackPointer+1].type == TOKEN_NUMBER && "Programming error: next token isnt a number!");
			STACK_TYPE v = popStack();
			pushStack(v);
			if(v != 0){
				stackPointer = atoi(tokens[stackPointer+1].data);
				continue;
			}
		}
		stackPointer++;
	}
	gettimeofday(&tv2, NULL);

	printf ("Total time = %f seconds\n",
         (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec));
}



int main(){	
	char tokens[128][256];
    int tokenCount = 0;
	parseSrc(readSrcFile("main.stc"), tokens, &tokenCount);
	Token* values = tokenize(tokens, &tokenCount);
	for (int i = 0; i < tokenCount; i++) {
    	printf("Token %d: Type = %d, Data = %s keywordType=%d\n", i, (values + i)->type, (values + i)->data, (values + i)->keywordType);
	}
	interpreter(values, &tokenCount);
	printf("%d %d %d %d %d %d %d %d",
	 stack[0],
	 stack[1],
	 stack[2],
	 stack[3],
	 stack[4],
	 stack[5],
	 stack[6],
	 stack[7]);
	return 0;
}