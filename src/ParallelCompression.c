//
//  ParallelCompression.c
//  libAppleArchive
//

#include "ParallelCompression.h"

#include <stdio.h>
#include <sys/sysctl.h>
#include <string.h>
#include <stdlib.h>

uint64_t getDefaultNThreads(void) {
    int32_t cores = 0;
    size_t size = 4;
    if (!sysctlbyname("hw.physicalcpu", &cores, &size, 0, 0)) {
        ParallelCompressionLogError("sysctlbyname");
        return 1;
    }
    return cores;
}

/*
 * pathIsValid
 *
 * Reimplementation of libAppleArchive's pathIsValid
 * Returns 0 if invalid, returns 1 if valid
 * Requirements:
 * -Path must be shorter than 1024 characters.
 * -Path must not contain a NULL before size.
 * -Path must not start with /
 * -Path *cannot* have "./" after a "/" (beginning is fine!)
 * -Path *cannot* have "../" after a "/"
 * -Path *cannot* begin with "../"
*/
int pathIsValid(const char *path, int size) {
    if (!size) {
        /* if 0 size, valid path */
        return 1;
    }
    if (size > 1023) {
        /* path must be 1023 or smaller */
        return 0;
    }
    if (memchr(path, 0, size)) {
        /* path must not be null terminated before size */
        return 0;
    }
    if (path[0] == '/') {
        /* path must not start with / */
        return 0;
    }
    uint64_t pos = 0;
    const char *shortPath;
loop_through_path:
    shortPath = path + pos;
    void *slashPtr = memchr(shortPath, '/', (uint64_t)(path - pos));
    if (!slashPtr) {
        return pos != size;
    }
    long slashIndex = (slashPtr - (void *)path);
    int slashIndexInShortPath = ((char *)slashIndex - pos);
    if (slashIndex == pos) {
        /* no / right after / */
        return 0;
    }
    if (pos && slashIndexInShortPath == 1 && shortPath[0] == '.') {
        /* no ./ when not at beginning at path */
        return 0;
    }
    if ((!pos || (pos && slashIndexInShortPath != 1)) && slashIndexInShortPath == 2 && shortPath[0] == '.' && shortPath[1] == '.') {
        /* no ../ */
        return 0;
    }
    pos = slashIndex + 1;
    if (pos < size) { goto loop_through_path;};
    return pos != size;
}

size_t SharedBufferReadFromBufferProc(uint8_t **buffer, uint8_t *dest, size_t n) {
    memcpy(dest, *buffer, n);
    *buffer = *buffer + n;
    return n;
}

size_t SharedBufferWriteToBufferProc(uint8_t **buffer, uint8_t *src, size_t n) {
    memcpy(*buffer, src, n);
    /* *buffer + n means that *buffer will be set to the end of *buffer */
    *buffer = *buffer + n;
    return n;
}
