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

#ifdef _WIN32
    #include <conio.h>  // For Windows _getch() function
#elif __unix__
    #include <termios.h>
    #include <unistd.h>
#else
    #error "Platform not supported"
#endif


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
	size_t toFindLength = strlen(toFind);
	for(size_t i = 0; i < heapSize; i++){
		if (strncmp(heap[i].data, toFind, toFindLength) == 0)
			return i;
	}
	return -1;
}

// Gets character without the need of enter like base function requires
char get_single_key() {
	#ifdef _WIN32
	    return _getch();
	#elif __unix__
	    // console could probably be set to have disabled canonical mode and echo
	    // becuase we dont use input anywhere else
	    // but ill leave it as is for now (probably better performance)
    	struct termios oldt, newt;
    	char c;
    	tcgetattr(STDIN_FILENO, &oldt);
    	newt = oldt;
    	newt.c_lflag &= ~(ICANON | ECHO);
    	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    	c = getchar();
	    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    	return c;
	#endif
}

void interpreter(Token* tokens, int* tokenCount){
	size_t stackPointer = 0;
	STACK_TYPE cache;
	Token t;
	size_t iters = 0;
	HeapObject* heap = (HeapObject*)malloc(HEAP_SIZE*sizeof(HeapObject));
	size_t heapSize = 1;
	// We make ret object that will work as flag when we enter "functions"
	// jumps will now be able to jump to ret
	// also call will store IP+2 to ret
	HeapObject ret;
	ret.data = "ret";
	ret.ptr = (size_t)makePtr(0);
	heap[0] = ret;
	#if DEBUG == 1
		struct timeval  tv1, tv2;
		gettimeofday(&tv1, NULL);
	#endif
	while(stackPointer < *tokenCount){
		iters++;
		t = tokens[stackPointer];
		if(t.keywordType == END){break;}
		else if(t.keywordType == PUSH){
			if(tokens[stackPointer+1].type == TOKEN_NUMBER){
				pushStack(atoi(tokens[stackPointer+1].data));
			}else{
				// pushes null byte and all characters to the stack
				pushStack(0);
				for (int i = strlen(tokens[stackPointer+1].data) - 2; i >= 1; i--) {
					if(tokens[stackPointer+1].data[i] == 'n' && tokens[stackPointer+1].data[i-1] == '\\'){
						pushStack(10); // newline character
						i--;
					}else{
						pushStack(tokens[stackPointer+1].data[i]);
					}
    			}
			}
			stackPointer+=2;
			continue;
		}
		else if(t.keywordType == POP){popStack();}
		else if(t.keywordType == ADD){add();}
		else if(t.keywordType == SUB){sub();}
		else if(t.keywordType == CMP){cmp();}
		else if(t.keywordType == DEC){dec();}
		else if(t.keywordType == INC){inc();}
		else if(t.keywordType == NOT){not();}

		else if(t.keywordType == LOAD){
			int64_t index = findInHeap(heap, heapSize, tokens[stackPointer+1].data);
			assert(index != -1 && "Programming error: cannot load value that hasn't been stored");
			pushStack(*((STACK_TYPE*)heap[index].ptr));
		}
		else if(t.keywordType == LOADF){
    		int64_t index = findInHeap(heap, heapSize, tokens[stackPointer + 1].data);
    		assert(index != -1 && "Programming error: cannot loadf value that hasn't been stored");
    		pushStack(*((STACK_TYPE*)heap[index].ptr));
		    free((STACK_TYPE*)heap[index].ptr);
		    free(heap[index].data);
		    if (index < heapSize - 1) {
			    memmove(&heap[index], &heap[index + 1], (heapSize - index - 1) * sizeof(heap[0]));
			}
			heapSize--;
		}
		else if(t.keywordType == STORE){
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

		else if(t.keywordType == SWITCH){sswitch();}
		else if(t.keywordType == ROT){rot();}
		else if(t.keywordType == OVER){over();}
		else if(t.keywordType == DUP){dup();}
		else if(t.keywordType == OUT){
			printf("%d\n", popStack());
		}
		else if(t.keywordType == OUTC){
			printf("%c", popStack());
		}
		else if(t.keywordType == JNZ){
			if(popStack() != 0){
				if(strcmp(tokens[stackPointer+1].data, "ret") == 0){
					stackPointer = *((STACK_TYPE*)heap[0].ptr);
					continue;
				}
				stackPointer = atoi(tokens[stackPointer+1].data);
				continue;
			}
		}
		else if(t.keywordType == JMP){
			if(strcmp(tokens[stackPointer+1].data, "ret") == 0){
				stackPointer = *((STACK_TYPE*)heap[0].ptr);
				continue;
			}
			stackPointer = atoi(tokens[stackPointer+1].data);
			continue;
		}
		else if(t.keywordType == CALL){
			// jmp that stores the IP where it was called from to ret
			*((STACK_TYPE*)heap[0].ptr) = stackPointer+2;
			stackPointer = atoi(tokens[stackPointer+1].data);
		}
		else if(t.keywordType == INCH){
			pushStack(get_single_key());
		}
		stackPointer++;
	}
	#if DEBUG == 1
		gettimeofday(&tv2, NULL);
		double seconds = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
		printf("Total time = %f seconds\n",seconds);
		printf("program took %d iterations\n", iters);
		printf("program ran in the rate of %f it/s\n", iters/seconds);
	#endif

}


double benchmark(Token* tokens, int* tokenCount, size_t runs){
	double sumSeconds = 0;

	struct timeval  tv1, tv2;
	for(size_t i = 0; i < runs; i++){
		gettimeofday(&tv1, NULL);
		interpreter(tokens, tokenCount);
		gettimeofday(&tv2, NULL);
		double seconds = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
		sumSeconds += seconds;
	}
	return sumSeconds/runs;
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
	typeCheck(values, &tokenCount);
	interpreter(values, &tokenCount);
	return 0;
}