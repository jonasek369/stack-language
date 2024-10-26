const uint8_t TOKEN_NONE                = 0;
const uint8_t TOKEN_KEYWORD             = 1;
const uint8_t TOKEN_NUMBER              = 2;
const uint8_t TOKEN_LABEL_NAME          = 3;
const uint8_t TOKEN_LABEL_NOT_DEFINE    = 4;
const uint8_t TOKEN_VAR_NAME            = 5;
const uint8_t TOKEN_STRING              = 6;

enum KEYWORDS {
    NOP=0,
    PUSH,
    POP,
    ADD,
    SUB,
    CMP,
    STORE, 
    LOADF,
    LOAD, 
    JMP,
    JNZ,
    INC,
    DEC,
    SWITCH,
    NOT,
    LABEL, 
    ROT,
    OVER,
    DUP,
    OUT,
    OUTC,
    CALL, // TESTING
    END
};

#define KEYWORD_SIZE 22 // testing 21
#define MAX_LABEL_SIZE 64

const char keywords[KEYWORD_SIZE][8] = {
    "push",
    "pop",
    "add",
    "sub",
    "cmp",
    "store",
    "loadf",
    "load",
    "jmp",
    "jnz",
    "inc",
    "dec",
    "switch",
    "not",
    "label",
    "rot",
    "over",
    "dup",
    "out",
    "outc",
    "call", // TESTING
    "end"
};

// const uint32_t keywords_sizes[KEYWORD_SIZE] = {4,3,3,3,3,5,5,4,3,3,3,3,6,3,5};

void parseSrc(char* src, char tokens[][256], int* tokenCount) {
    char token[256];
    int i = 0;
    *tokenCount = 0;
    bool inString = false; // support for string 

    while (*src != '\0') {
        if(*src == '"'){
            inString = !inString;
        }
        if (!inString && (*src == ' ' || *src == ';')) {
            if (i > 0) {
                token[i] = '\0';
                strcpy(tokens[*tokenCount], token);
                (*tokenCount)++;
                i = 0;
            }
        } else {
            token[i] = *src;
            i++;
        }
        src++;
    }
    if (i > 0) {
        token[i] = '\0';
        strcpy(tokens[*tokenCount], token);
        (*tokenCount)++;
    }
}

char* readSrcFile(const char *filename) {
    FILE *file;
    char ch;
    int size = 1024;
    int length = 0; 
    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file\n");
        return NULL;
    }
    char *content = (char *)malloc(size * sizeof(char));
    if (content == NULL) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    bool inComment = false;
    while ((ch = fgetc(file)) != EOF) {
        // really basic comment function. 
        // DO NOT USE TABS IN CODE
        if(ch == '#'){
            inComment = true;
        }
        if(ch == '\n' && inComment){
            inComment = false;
        }
        if (!inComment && ch != '\n') {
            if (length >= size - 1) {
                size *= 2;  // Double the buffer size
                content = (char *)realloc(content, size * sizeof(char));
                if (content == NULL) {
                    printf("Error: Memory reallocation failed\n");
                    fclose(file);
                    return NULL;
                }
            }
            content[length++] = ch;
        }
    }
    content[length] = '\0';
    fclose(file);
    return content;
}

bool is_num(char* src){
    while(*src != '\0'){
        if(*src >= 48 && *src <= 57){
            src++;
        }else{
            return false;
        }
    }
    return true;
}

struct Token{
    uint8_t type;
    uint8_t keywordType;
    char* data;
};

struct Label{
    char* data;
    size_t ptr;
};



typedef struct Token Token;
typedef struct Label Label;
typedef struct Label HeapObject;



Token* tokenize(char tokens[][256], int* tokenCount) {
    Token* parsedTokens = malloc(sizeof(Token) * (*tokenCount));
    Label* labels = malloc(sizeof(Label) * (*tokenCount));
    size_t labelCount = 0;


    if (parsedTokens == NULL || labels == NULL) {
        printf("Memory allocation failed!\n");
        free(parsedTokens);
        free(labels);
        return NULL;
    }
    for (int i = 0; i < *tokenCount; i++) {
        Token tkn;
        tkn.type = 0;
        tkn.keywordType = 0;
        tkn.data = NULL;
        if (is_num(tokens[i]) == true) {
            tkn.type = TOKEN_NUMBER;
            tkn.keywordType = NOP;
        } else {
            for (int kw = 0; kw < KEYWORD_SIZE; kw++) {
                if (strcmp(keywords[kw], tokens[i]) == 0) {
                    tkn.type = TOKEN_KEYWORD;
                    tkn.keywordType = kw+1;
                    break;
                }
            }
            if (tkn.type != TOKEN_KEYWORD && strlen(tokens[i]) > 1 && tokens[i][0] == '"' && tokens[i][strlen(tokens[i]) - 1] == '"') {
                tkn.type = TOKEN_STRING;
            }
            else if(tkn.type != TOKEN_KEYWORD){
                int labelFound = 0;
                for (size_t j = 0; j < labelCount; j++) {
                    if (strcmp(labels[j].data, tokens[i]) == 0) {
                        tkn.type = TOKEN_NUMBER;
                        tkn.keywordType = NOP;
                        char* toStr = (char*)malloc(MAX_LABEL_SIZE);
                        itoa(labels[j].ptr, toStr, 10);
                        tkn.data = toStr;
                        labelFound = 1;
                        break;
                    }
                }
                // sets variable name
                if( parsedTokens[i-1].keywordType == STORE || 
                    parsedTokens[i-1].keywordType == LOAD ||
                    parsedTokens[i-1].keywordType == LOADF){
                    tkn.type = TOKEN_VAR_NAME;
                    tkn.data = malloc(strlen(tokens[i]) + 1);
                    strcpy(tkn.data, tokens[i]);
                    parsedTokens[i] = tkn;
                    continue;
                }
                // sets undefined label name
                else if(!labelFound && parsedTokens[i-1].keywordType != LABEL){
                    tkn.type = TOKEN_LABEL_NOT_DEFINE;
                    tkn.data = malloc(strlen(tokens[i]) + 1);
                    strcpy(tkn.data, tokens[i]);
                    parsedTokens[i] = tkn;
                    continue;
                }
                else if (!labelFound /*&& parsedTokens[i-1].keywordType == LABEL*/) {
                    tkn.type = TOKEN_LABEL_NAME;
                    labels[labelCount].data = malloc(strlen(tokens[i]) + 1);
                    if (labels[labelCount].data == NULL) {
                        printf("Memory allocation failed for label %d!\n", i);
                        for (int j = 0; j < i; j++) {
                            free(parsedTokens[j].data);
                        }
                        free(parsedTokens);
                        return NULL;
                    }
                    strcpy(labels[labelCount].data, tokens[i]);
                    labels[labelCount].ptr = i;
                    labelCount++;
                }else{
                    parsedTokens[i] = tkn;
                    continue;
                }
            }
        }
        tkn.data = malloc(strlen(tokens[i]) + 1);
        if (tkn.data == NULL) {
            printf("Memory allocation failed for token %d!\n", i);
            for (int j = 0; j < i; j++) {
                free(parsedTokens[j].data);
            }
            free(parsedTokens);
            return NULL;
        }
        strcpy(tkn.data, tokens[i]);
        parsedTokens[i] = tkn;
    }
    // check unset labels that have not been declared before use
    // this alows jumping to labels that are on the end of file 
    for (int i = 0; i < *tokenCount; i++) {
        if(parsedTokens[i].type != TOKEN_LABEL_NOT_DEFINE){
            continue;
        }
        for (size_t j = 0; j < labelCount; j++) {
            if (strcmp(labels[j].data, parsedTokens[i].data) == 0) {
                parsedTokens[i].type = TOKEN_NUMBER;
                parsedTokens[i].keywordType = NOP;
                char* toStr = (char*)malloc(MAX_LABEL_SIZE);
                itoa(labels[j].ptr, toStr, 10);
                free(parsedTokens[i].data);
                parsedTokens[i].data = toStr;
                break;
            }
        }
    }
    free(labels);
    return parsedTokens;
}

// checks the program before execution
// so interpreter dosent have to check next tokens
// and have assers
void typeCheck(Token* tokens, int* tokenCount){
    size_t stackPointer = 0;
    Token t;
    while(stackPointer < *tokenCount){
        t = tokens[stackPointer];
        if(t.keywordType == PUSH){
            assert((tokens[stackPointer+1].type == TOKEN_NUMBER || tokens[stackPointer+1].type == TOKEN_STRING)&& "Programming error: next token isnt a number!");
        }
        if(t.keywordType == JNZ){
            if(strcmp(tokens[stackPointer+1].data, "ret") == 0){
            }else{
                assert(tokens[stackPointer+1].type == TOKEN_NUMBER && "Programming error: next token isnt a number!");
            }
        }
        if(t.keywordType == JMP){
            if(strcmp(tokens[stackPointer+1].data, "ret") == 0){
            }else{
                assert(tokens[stackPointer+1].type == TOKEN_NUMBER && "Programming error: next token isnt a number!");
            }
        }
        if(t.keywordType == CALL){
            assert(tokens[stackPointer+1].type == TOKEN_NUMBER && "Programming error: next token isnt a number!");
        }
        stackPointer++;
    }
}