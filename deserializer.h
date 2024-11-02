struct Deserializer{
    FILE* file;
    bool reachedEnd;
};

typedef struct Deserializer Deserializer;

Deserializer* makeDeserializer(char* filePath){
    Deserializer* deser = malloc(sizeof(Deserializer));
    deser->file = fopen(filePath, "rb");
    if(deser->file == NULL){
        free(deser);
        return NULL;
    }
    deser->reachedEnd = false;
}

uint8_t readByte(Deserializer* deser){
    assert(deser->reachedEnd == false && "Reached the end of file");
    uint8_t byte;
    byte = fgetc(deser->file);
    if(byte == EOF){
        deser->reachedEnd = true;
    }
    return byte;
}


uint8_t deserializeUint8_t(Deserializer* deser){
    return readByte(deser);
}

uint32_t deserializeUint32_t(Deserializer* deser) {
    uint32_t value = 0;

    for (int i = 0; i < sizeof(uint32_t); i++) {
        uint8_t byte = readByte(deser);
        value |= (uint32_t)byte << (i * 8);
    }
    return value;
}

size_t deserializeSize_t(Deserializer* deser) {
    size_t value = 0;

    for (int i = 0; i < sizeof(size_t); i++) {
        uint8_t byte = readByte(deser);
        value |= (size_t)byte << (i * 8);
    }
    return value;
}

char* deserializeStr(Deserializer* deser) {
    size_t bufferSize = 64;
    char* str = (char*)malloc(bufferSize);
    if (str == NULL) {
        return NULL;
    }

    size_t index = 0;

    while (true) {
        uint8_t byte = readByte(deser);
        if (byte == '\0') {
            str[index] = '\0';
            break;
        }
        if (index + 1 >= bufferSize) {
            bufferSize *= 2;
            char* temp = (char*)realloc(str, bufferSize);
            if (temp == NULL) {
                free(str);
                return NULL;
            }
            str = temp;
        }
        str[index++] = (char)byte;
    }

    return str;
}