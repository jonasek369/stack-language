#define DEFAULT_SERIALIZER_BYTE_SIZE 256

struct Serializer{
	void* data; 
	size_t size;
	size_t capacity;
};

typedef struct Serializer Serializer;

Serializer* makeSerializer(){
	Serializer* ser = malloc(sizeof(Serializer));
	if (ser == NULL) {
        return NULL;
    }
	ser->data = NULL;
	ser->size = 0;
	ser->capacity = 0;
	return ser;
}

void allocSerializer(Serializer* ser) {
    if (ser->data == NULL) {
        ser->data = malloc(DEFAULT_SERIALIZER_BYTE_SIZE);
        if (ser->data == NULL) {
            perror("Failed to allocate memory for serializer");
            exit(EXIT_FAILURE);
        }
        ser->capacity = DEFAULT_SERIALIZER_BYTE_SIZE;
    }
}

void reallocSerialize(Serializer* ser) {
    void* newData = realloc(ser->data, ser->capacity * 2);
    if (newData == NULL) {
        perror("Failed to reallocate memory for serializer");
        exit(EXIT_FAILURE);
    }
    ser->data = newData;
    ser->capacity *= 2;
}

void freeSerializer(Serializer* ser) {
    if (ser) {
        free(ser->data);
        free(ser);
    }
}


void serializeStr(Serializer* ser, char* str){
	allocSerializer(ser);
	while(*str != '\0'){
		if(ser->size+1 >= ser->capacity){
			reallocSerialize(ser);
		}
		((char*)ser->data)[ser->size++] = *str++;
	}
    if (ser->size + 1 >= ser->capacity) {
        reallocSerialize(ser);
    }
    ((char*)ser->data)[ser->size++] = '\0';
}


void serializeUint32_t(Serializer* ser, uint32_t* value){
    if (ser == NULL || value == NULL) {
        return;
    }
    if (ser->data == NULL) {
        allocSerializer(ser);
    }
    if (ser->size + sizeof(uint32_t) > ser->capacity) {
        reallocSerialize(ser);
    }
    unsigned char* bytePtr = (unsigned char*)value;
    for (int i = 0; i < sizeof(uint32_t); i++) {
        ((char*)ser->data)[ser->size++] = bytePtr[i];
    }
}

void serializeSize_t(Serializer* ser, size_t* value){
    if (ser == NULL || value == NULL) {
        return;
    }
    if (ser->data == NULL) {
        allocSerializer(ser);
    }
    if (ser->size + sizeof(size_t) > ser->capacity) {
        reallocSerialize(ser);
    }
    unsigned char* bytePtr = (unsigned char*)value;
    for (int i = 0; i < sizeof(size_t); i++) {
        ((char*)ser->data)[ser->size++] = bytePtr[i];
    }
}

void serializeUint8_t(Serializer* ser, uint8_t* value){
    if (ser == NULL || value == NULL) {
        return;
    }
    if (ser->data == NULL) {
        allocSerializer(ser);
    }
    if (ser->size + sizeof(uint8_t) > ser->capacity) {
        reallocSerialize(ser);
    }
    unsigned char* bytePtr = (unsigned char*)value;
    for (int i = 0; i < sizeof(uint8_t); i++) {
        ((char*)ser->data)[ser->size++] = bytePtr[i];
    }
}

int saveToFile(const char* filename, struct Serializer* serializer) {
    if (serializer == NULL || serializer->data == NULL || filename == NULL) {
        return -1;
    }

    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        return -1;
    }
    size_t written = fwrite(serializer->data, 1, serializer->size, file);
    if (written != serializer->size) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

