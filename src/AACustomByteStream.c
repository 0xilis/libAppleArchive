//
//  AACustomByteStream.c
//  libAppleArchive
//

#include "AppleArchive.h"

struct AAByteStream_impl {
    void *data;
    AAByteStreamCloseProc closeProc; /* 0x8 */
    AAByteStreamReadProc readProc; /* 0x10 */
    AAByteStreamWriteProc writeProc; /* 0x18 */
    AAByteStreamPReadProc preadProc; /* 0x20 */
    AAByteStreamPWriteProc pwriteProc; /* 0x28 */
    AAByteStreamSeekProc seekProc; /* 0x30 */
    AAByteStreamCancelProc cancelProc; /* 0x38 */
    int (*truncate)(struct AAByteStreamFileDesc_impl *, off_t); /* 0x40 */
    uint64_t padding; /* 0x48 */
};

AAByteStream AACustomByteStreamOpen(void) {
    AAByteStream stream = calloc(1, sizeof(struct AAByteStream_impl));
    if (!stream) {
        ParallelCompressionLogError("malloc");
    }
    return stream;
}

void AACustomByteStreamSetData(AAByteStream s, void *data) {
    s->data = data;
}

void AACustomByteStreamSetCloseProc(AAByteStream s, AAByteStreamCloseProc proc) {
    s->closeProc = proc;
}

void AACustomByteStreamSetReadProc(AAByteStream s, AAByteStreamReadProc proc) {
    s->readProc = proc;
}

void AACustomByteStreamSetPReadProc(AAByteStream s, AAByteStreamPReadProc proc) {
    s->preadProc = proc;
}

void AACustomByteStreamSetWriteProc(AAByteStream s, AAByteStreamWriteProc proc) {
    s->writeProc = proc;
}

void AACustomByteStreamSetPWriteProc(AAByteStream s, AAByteStreamPWriteProc proc) {
    s->pwriteProc = proc;
}

void AACustomByteStreamSetSeekProc(AAByteStream s, AAByteStreamSeekProc proc) {
    s->seekProc = proc;
}

void AACustomByteStreamSetCancelProc(AAByteStream s, AAByteStreamCancelProc proc) {
    s->cancelProc = proc;
}
