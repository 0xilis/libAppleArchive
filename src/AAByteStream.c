//
//  AAByteStream.c
//  libAppleArchive
//

#include "AppleArchive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>

struct AAByteStreamFileDesc_impl {
    int fd;
    int automatic_close; /* 0x4 */
    int reserved; /* 0x8 */
};

struct AAByteStream_impl {
    void *fileDesc;
    AAByteStreamCloseProc closeProc; /* 0x8 */
    AAByteStreamReadProc readProc; /* 0x10 */
    AAByteStreamWriteProc writeProc; /* 0x18 */
    AAByteStreamPReadProc preadProc; /* 0x20 */
    AAByteStreamPWriteProc pwriteProc; /* 0x28 */
    AAByteStreamSeekProc seekProc; /* 0x30 */
    AAByteStreamCancelProc cancelProc; /* 0x38 */
    int (*truncate)(struct AAByteStreamFileDesc_impl *, off_t); /* 0x40 */
};

typedef struct AAByteStreamFileDesc_impl* AAByteStreamFileDesc;

int aaFileStreamClose(AAByteStreamFileDesc fileDesc) {
    if (!fileDesc) {
        return 0;
    }
    int fd = fileDesc->fd;
    if (fileDesc->automatic_close) {
        if (fd >= 0) {
            close(fd);
        }
    }
    free(fileDesc);
    return 0;
}

ssize_t aaFileStreamRead(AAByteStreamFileDesc fileDesc, void * buf, size_t nbyte) {
    if (fileDesc->reserved) {
        return -1;
    }
    return read(fileDesc->fd, buf, nbyte);
}

ssize_t aaFileStreamWrite(AAByteStreamFileDesc fileDesc, void * buf, size_t nbyte) {
    if (fileDesc->reserved == 0) {
        return -1;
    }
    return write(fileDesc->fd, buf, nbyte);
}

ssize_t aaFileStreamPRead(AAByteStreamFileDesc fileDesc, void * buf, size_t nbyte, off_t offset) {
    if (fileDesc->reserved == 0) {
        return -1;
    }
    return pread(fileDesc->fd, buf, nbyte, offset);
}

ssize_t aaFileStreamPWrite(AAByteStreamFileDesc fileDesc, void * buf, size_t nbyte, off_t offset) {
    if (fileDesc->reserved) {
        return -1;
    }
    return pwrite(fileDesc->fd, buf, nbyte, offset);
}

void aaFileStreamAbort(AAByteStreamFileDesc fileDesc) {
    fileDesc->reserved = 1;
}

off_t aaFileStreamSeek(AAByteStreamFileDesc fileDesc, off_t offset, int whence) {
    if (fileDesc->reserved) {
        return -1;
    }
    return lseek(fileDesc->fd, offset, whence);
}

int aaFileStreamTruncate(AAByteStreamFileDesc fileDesc, off_t len) {
    if (fileDesc->reserved) {
        return -1;
    }
    return ftruncate(fileDesc->fd, len);
}

AAByteStream AAFileStreamOpenWithFD(int fd, int automatic_close) {
    AAByteStream byteStream = calloc(1, 80);
    AAByteStreamFileDesc descStream = malloc(sizeof(struct AAByteStreamFileDesc_impl));
    if (!byteStream || !descStream) {
        ParallelCompressionLogError("malloc");
        free(byteStream);
        free(descStream);
        return 0;
    }
    descStream->fd = fd;
    descStream->automatic_close = automatic_close;
    descStream->reserved = 0;
        
    byteStream->fileDesc = descStream;
    byteStream->closeProc = (AAByteStreamCloseProc)aaFileStreamClose;
    byteStream->readProc = (AAByteStreamReadProc)aaFileStreamRead;
    byteStream->writeProc = (AAByteStreamWriteProc)aaFileStreamWrite;
    byteStream->preadProc = (AAByteStreamPReadProc)aaFileStreamPRead;
    byteStream->pwriteProc = (AAByteStreamPWriteProc)aaFileStreamPWrite;
    byteStream->seekProc = (AAByteStreamSeekProc)aaFileStreamSeek;
    byteStream->cancelProc = (AAByteStreamCancelProc)aaFileStreamAbort;
    byteStream->truncate = aaFileStreamTruncate;
    return byteStream;
}

AAByteStream AAFileStreamOpenWithPath(const char *path, int open_flags, mode_t open_mode) {
    int fd = open(path,open_flags,open_mode);
    if (fd < 0) {
        ParallelCompressionLogError("open: %s");
        return 0;
    }
    AAByteStream byteStream = AAFileStreamOpenWithFD(fd, 1);
    if (!byteStream) {
        close(fd);
        return 0;
    }
    return byteStream;
}

void AAByteStreamCancel(AAByteStream s) {
    AAByteStreamCancelProc cancelProc = s->cancelProc;
    if (!cancelProc) {
        return;
    }
    cancelProc(s->fileDesc);
}

int AAByteStreamClose(AAByteStream s) {
    if (!s) {
        return 0;
    }
    AAByteStreamFileDesc fileDesc = s->fileDesc;
    if (!fileDesc) {
        free(s);
        return 0;
    }
    AAByteStreamCloseProc closeProc = s->closeProc;
    int result = closeProc(s);
    free(s);
    return result;
}

ssize_t AAByteStreamRead(AAByteStream s, void *buf, size_t nbyte) {
    AAByteStreamReadProc readProc = s->readProc;
    if (!readProc) {
        return -1;
    }
    return readProc(s->fileDesc, buf, nbyte);
}

ssize_t AAByteStreamPRead(AAByteStream s, void *buf, size_t nbyte, off_t offset) {
    AAByteStreamPReadProc preadProc = s->preadProc;
    if (!preadProc) {
        return -1;
    }
    return preadProc(s, buf, nbyte, offset);
}

ssize_t AAByteStreamWrite(AAByteStream s, const void *buf, size_t nbyte) {
    AAByteStreamWriteProc writeProc = s->writeProc;
    if (!writeProc) {
        return -1;
    }
    return writeProc(s, buf, nbyte);
}

ssize_t AAByteStreamPWrite(AAByteStream s, const void *buf, size_t nbyte, off_t offset) {
    AAByteStreamPWriteProc pwriteProc = s->pwriteProc;
    if (!pwriteProc) {
        return -1;
    }
    return pwriteProc(s, buf, nbyte, offset);
}

off_t AAByteStreamSeek(AAByteStream s, off_t offset, int whence) {
    AAByteStreamSeekProc seekProc = s->seekProc;
    if (!seekProc) {
        return -1;
    }
    return seekProc(s, offset, whence);
}
