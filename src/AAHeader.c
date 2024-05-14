//
//  AAHeader.c
//  libAppleArchive
//

#include "AppleArchive.h"
#ifndef INT_MAX
#define INT_MAX 0x7fffffff
#endif

struct AAHeader_impl {
    uint32_t fieldCount;
    uint32_t fieldsSize; /* 0x4 */
    void *keys; /* 0x8, pointer to an array of keys */
    size_t encodedSize; /* 0x10 */
    uint64_t sizeAgainIGuess; /* idk what 0x18 is */
    unsigned char *encodedData; /* 0x20 */
    uint64_t payloadSize; /* 0x28 */
};

/* also reffered to as aaBlobReserve */
uint64_t realloc_blob(AAHeader header, int size) {
    if (size > 0xFFFF) { /* blob must be USHRT_MAX or smaller */
        return -1;
    }
    uint64_t oldSize = header->sizeAgainIGuess;
    if (oldSize >= size) {
        return 0;
    }
    long long payloadSize = size;
    while (payloadSize < size) {
        if (payloadSize == 0) {
            payloadSize = 64;
        } else {
            payloadSize = ((payloadSize >> 1) + payloadSize);
        }
    }
    if (size >= payloadSize) {
        return 0;
    }
    unsigned char *blobPtr;
    if (payloadSize <= INT_MAX) {
        unsigned char *encodedData = header->encodedData;
        blobPtr = realloc(encodedData, payloadSize);
        if (blobPtr) {
            header->encodedData = blobPtr;
            header->sizeAgainIGuess = payloadSize;
            return 0;
        } else {
            free(blobPtr);
        }
    }
    header->encodedSize = 0;
    header->sizeAgainIGuess = 0;
    header->encodedData = 0;
    header->fieldCount = 0;
    return -1;
}

int init_blob_with_magic(AAHeader header) {
    if (realloc_blob(header, 6) < 0) {
        /* error */
        return -1;
    }
    strncpy((char *)header->encodedData, "AA01", 4); /* builtin */
    header->encodedSize = 6;
    /* below line is definitely not right */
    /* header->encodedData[4] = 6; */
    /* the size is USHRT_MAX in the encoded data, start is 6 */
    uint16_t encodedDataSizePtr = (uint16_t *)(header->encodedData + 4);
    *encodedDataSizePtr = 6;
    return 0;
}

void AAHeaderDestroy(AAHeader header) {
    if (header) {
        free(header->keys);
        free(header->encodedData);
        header->encodedSize = 0;
        header->sizeAgainIGuess = 0;
        header->encodedData = 0;
        free(header);
    }
}

AAHeader AAHeaderCreate(void) {
    AAHeader header = malloc(sizeof(struct AAHeader_impl));
    if (!header) {
        ParallelCompressionLogError("malloc");
        return 0;
    }
    memset(header, 0, sizeof(struct AAHeader_impl));
    if (init_blob_with_magic(header) < 0) {
        ParallelCompressionLogError("init_blob_with_magic");
        AAHeaderDestroy(header);
        return 0;
    }
    return header;
}

int realloc_fields(AAHeader header, int size) {
    if (size < 0) {
        return -1;
    }
    int fieldsSizeOrig = header->fieldsSize;
    if (fieldsSizeOrig >= size) {
        return 0;
    }
    int fieldsSize = fieldsSizeOrig;
    while (fieldsSize < size) {
        if (fieldsSize == 0) {
            fieldsSize = 16;
        } else {
            fieldsSize = ((fieldsSize >> 1) + fieldsSize);
        }
    }
    if (fieldsSizeOrig >= fieldsSize) {
        return 0;
    }
    uint64_t actualBlockSize = fieldsSize * 48;
    if (actualBlockSize <= INT_MAX) {
        void *keys = header->keys;
        void *keysNewPtr = realloc(keys, actualBlockSize);
        if (keysNewPtr) {
            header->keys = keysNewPtr;
            header->fieldsSize = fieldsSize;
        } else {
            free(keys);
        }
    }
    header->fieldCount = 0;
    header->fieldsSize = 0;
    header->keys = 0;
    header->encodedSize = 0;
    return -1;
}

int aaHeaderInitWithEncodedData(AAHeader header, size_t headerSize, char *encodedData) {
    init_blob_with_magic(header);
    header->fieldCount = 0;
    header->payloadSize = 0;
    if (headerSize <= 5) {
        /*
         * headerSize should always be 6 or greater.
         * This is as it needs to represent magic (4bytes)
         * and, 2 bytes to store headerSize.
         */
        ParallelCompressionLogError("invalid header size: %llu");
        header->fieldCount = 0;
        header->encodedSize = 0;
        header->payloadSize = 0;
        return -1;
    }
    uint32_t *dumbHack = *(uint32_t **)&encodedData;
    uint32_t encodedMagic = dumbHack[0];
    /* Here, we check that the first 4 bytes of encodedData are YAA1 or AA01 */
    if (encodedMagic != 0x31414159 && encodedMagic != 0x31304141) {
        ParallelCompressionLogError("invalid header magic");
        header->fieldCount = 0;
        header->encodedSize = 0;
        header->payloadSize = 0;
        return -1;
    }
    uint16_t *headerSizePointer = encodedData + 4;
    uint16_t headerSizeStored = *headerSizePointer;
    if (headerSizeStored != headerSize) {
        ParallelCompressionLogError("header size mismatch: stored %u, got %llu");
        header->fieldCount = 0;
        header->encodedSize = 0;
        header->payloadSize = 0;
        return -1;
    }
    if (realloc_blob(header, headerSize) < 0) {
        ParallelCompressionLogError("realloc_blob");
        header->fieldCount = 0;
        header->encodedSize = 0;
        header->payloadSize = 0;
        return -1;
    }
    memcpy(header->encodedData, encodedData, headerSize);
    /* Even if we had YAA1, overwrite it with AA01 */
    strncpy(header->encodedData, "AA01", 4);
    /* For some reason, ->encodedSize is still 6? */
    header->encodedSize = 6;
    if (headerSize < 7) {
        /* We're only (magic) 06 00 so return */
        return;
    }
    size_t stringSizi = 0;
    while (true) {
        if (realloc_fields(header, header->fieldsSize) < 0) {
            ParallelCompressionLogError("realloc_fields");
            header->fieldCount = 0;
            header->encodedSize = 0;
            header->payloadSize = 0;
            return -1;
        }
        void *keys = header->keys;
        uint32_t fieldCount = header->fieldCount; /* The original! */
        header->fieldCount = fieldCount + 1;
        uint64_t encodedSize = header->encodedSize;
        /* Doing +4 to account for 3 bytes of field key name and 1 byte of subtype */
        if (encodedSize + 4 > headerSize) {
            ParallelCompressionLogError("truncated header");
            header->fieldCount = 0;
            header->encodedSize = 0;
            header->payloadSize = 0;
            return -1;
        }
        /* realloc_fields has keys be fieldsSize*48 */
        uint64_t totalKeysAllocation = fieldCount * 48;
        uint32_t *newKey = keys + totalKeysAllocation;
        *newKey = encodedData + encodedSize;
        /* subtype for field key should be 4th character */
        uint32_t newKeySubtype = (uint32_t *)(*((uint8_t *)(newKey + 3)));
        /*
         * Ok, so this seems to be having pos 8 of newKey
         * to be set to the subtype in the encoded data.
         * We aren't setting the newKey in the encodedData
         * btw, it's header->keys.
         */
        *((uint32_t *)(keys + totalKeysAllocation + 8)) = newKeySubtype;
        /*
         * Now, we set subtype in encoded data to 0?
         */
        *((uint8_t *)(newKey + 3)) = 0;
        /* Switch case to find fieldSize? */
        AAFieldType fieldType;
        size_t fieldSize;
        switch (newKeySubtype) {
            case '*':
                fieldSize = 0;
                fieldType = AA_FIELD_TYPE_FLAG;
                break;
            case '1':
                fieldSize = 1;
                fieldType = AA_FIELD_TYPE_UINT;
                break;
            case '2':
                fieldSize = 2;
                fieldType = AA_FIELD_TYPE_UINT;
                break;
            case '4':
                fieldSize = 4;
                fieldType = AA_FIELD_TYPE_UINT;
                break;
            case '8':
                fieldSize = 8;
                fieldType = AA_FIELD_TYPE_UINT;
                break;
            case 'A':
                fieldSize = 2;
                fieldType = AA_FIELD_TYPE_BLOB;
                break;
            case 'B':
                fieldSize = 4;
                fieldType = AA_FIELD_TYPE_BLOB;
                break;
            case 'C':
                fieldSize = 8;
                fieldType = AA_FIELD_TYPE_BLOB;
                break;
            case 'F':
                fieldSize = 4;
                fieldType = AA_FIELD_TYPE_HASH;
                break;
            case 'G':
                fieldSize = 20;
                fieldType = AA_FIELD_TYPE_HASH;
                break;
            case 'H':
                fieldSize = 32;
                fieldType = AA_FIELD_TYPE_HASH;
                break;
            case 'I':
                fieldSize = 48;
                fieldType = AA_FIELD_TYPE_HASH;
                break;
            case 'J':
                fieldSize = 64;
                fieldType = AA_FIELD_TYPE_HASH;
                break;
            case 'S':
                fieldSize = 8;
                fieldType = AA_FIELD_TYPE_TIMESPEC;
                break;
            case 'T':
                fieldSize = 12;
                fieldType = AA_FIELD_TYPE_TIMESPEC;
                break;
            case 'P':
                /* Field is a string, different from other types */
                uint64_t encodedHeaderSize = header->encodedSize;
                /* Doing + 6 to account for 3 bytes of field key name and 1 byte of subtype, AND 2 bytes for string size */
                if (encodedHeaderSize + 6 > headerSize) {
                    ParallelCompressionLogError("truncated header");
                    header->fieldCount = 0;
                    header->encodedSize = 0;
                    header->payloadSize = 0;
                    return -1;
                }
                uint16_t *stringSizePtr = (uint16_t *)(((uint8_t *)(encodedData + encodedHeaderSize)) + 4);
                size_t stringSize = *stringSizePtr;
                stringSizi = stringSize;
                fieldSize = stringSize + 2;
                fieldType = AA_FIELD_TYPE_STRING;
                break;
            default:
                ParallelCompressionLogError("invalid field subtype: %d");
                header->fieldCount = 0;
                header->encodedSize = 0;
                header->payloadSize = 0;
                return -1;
        }
        /* Out of the switch case */
        void *keysBop = keys + totalKeysAllocation;
        *((uint32_t *)(keysBop + 4)) = fieldType;
        uint64_t encodedHeaderSize = header->encodedSize;
        if (encodedHeaderSize + fieldSize + 4 > headerSize) {
            ParallelCompressionLogError("truncated header");
            header->fieldCount = 0;
            header->encodedSize = 0;
            header->payloadSize = 0;
            return -1;
        }
        /* Probably not accurate... */
        *(uint32_t *)(keysBop + 12) = encodedHeaderSize;
        *(uint32_t *)(keysBop + 16) = (int32_t)fieldSize + 4;
        *(uint64_t *)(keysBop + 24) = 0;
        uint64_t blobSizePtr = (uint64_t *)(keysBop + 32);
        *blobSizePtr = 0;
        uint64_t *dunno = (uint64_t *)(keysBop + 40);
        *dunno = 0;
        switch (fieldType) {
            case AA_FIELD_TYPE_UINT:
                /* Assuming size is fieldSize because why wouldn't it be */
                memcpy(dunno, encodedData + encodedHeaderSize + 4, fieldSize);
                break;
            case AA_FIELD_TYPE_STRING:
                *dunno = stringSizi;
                break;
            case AA_FIELD_TYPE_HASH:
                *dunno = fieldSize;
                break;
            case AA_FIELD_TYPE_TIMESPEC:
                /* ? */
                break;
            case AA_FIELD_TYPE_BLOB:
                memcpy(blobSizePtr, encodedData + encodedHeaderSize + 4, 8);
                *(uint64_t *)(keysBop + 24) = header->payloadSize;
                break;
        }
        header->payloadSize = header->payloadSize + *blobSizePtr;
        uint64_t iDontKnowWhatThisIs = *(uint32_t *)(keysBop + 16);
        uint64_t newEncodedSize = iDontKnowWhatThisIs + header->encodedSize;
        header->encodedSize = newEncodedSize;
        if (newEncodedSize >= headerSize) {
            /* Header parsing was a success!! */
            return 0;
        }
    }
}

AAHeader AAHeaderCreateWithEncodedData(size_t headerSize, uint8_t *encodedData) {
    AAHeader header = AAHeaderCreate();
    if (!header) {
        return 0;
    }
    if (aaHeaderInitWithEncodedData(header, headerSize, encodedData) < 0) {
        AAHeaderDestroy(header);
        return 0;
    }
    return header;
}
