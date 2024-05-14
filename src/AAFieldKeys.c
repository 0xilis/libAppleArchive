//
//  AAFieldKeys.c
//  libAppleArchive
//

#include "AppleArchive.h"
#include <stdio.h>
#include <stdlib.h>

struct AAFieldKeySet_impl {
    uint64_t keyCount;
    AAFieldKey *keys; /* 0x8 */
};

AAFieldKeySet AAFieldKeySetCreate(void) {
    AAFieldKeySet fieldKeySet = malloc(0x10);
    if (!fieldKeySet) {
        ParallelCompressionLogError("malloc");
        return 0;
    }
    fieldKeySet->keyCount = 0;
    fieldKeySet->keys = 0;
    return fieldKeySet;
}

AAFieldKey AAFieldKeySetGetKey(AAFieldKeySet key_set, uint32_t i) {
    AAFieldKey *firstKeyPtr = key_set->keys;
    return *(firstKeyPtr + (i * 4));
}

void AAFieldKeySetDestroy(AAFieldKeySet key_set) {
    if (key_set) {
        free(key_set->keys);
        free(key_set);
    }
}

uint32_t AAFieldKeySetGetKeyCount(AAFieldKeySet key_set) {
    return (uint32_t)key_set->keyCount;
}

int AAFieldKeySetClear(AAFieldKeySet key_set) {
    /* this doesn't free or 0 keys... */
    key_set->keyCount = 0;
    return 0;
}
