//
//  AAArchiveStream.c
//  libAppleArchive
//

#include "AppleArchive.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct AAEncoderStream_impl {
    AAFlagSet flags;
    uint64_t nthreads; /* 0x8 */
    AAByteStream byteStream; /* 0x10 */
    uint64_t filler_0; /* 0x18 */
    uint64_t filler_1; /* 0x20 */
    uint64_t something; /* 0x28 */
    char filler_2[0x5e]; /* 0x30 */
    
};

typedef struct AAEncoderStream_impl * AAEncoderStream;

AAArchiveStream AAEncodeArchiveOutputStreamOpen(AAByteStream stream, void * _Nullable msg_data, AAEntryMessageProc _Nullable msg_proc, AAFlagSet flags, int n_threads) {
    AAArchiveStream archiveStream = malloc(0x38);
    AAEncoderStream encoderStream = malloc(0x478);
    if (!archiveStream || !encoderStream) {
        /*  encoderStreamClose() */
        /* for now, just free it */
        free(encoderStream);
        free(archiveStream);
        return 0;
    }
    memset(archiveStream, 0, 0x38);
    bzero(encoderStream + 8, 0x470);
    encoderStream->flags = flags;
    uint64_t nthreads = n_threads;
    if (n_threads) {
        nthreads = getDefaultNThreads();
    }
    encoderStream->nthreads = nthreads;
    encoderStream->byteStream = stream;
    
    /* FINISH LATER */
    
    return archiveStream;
}
