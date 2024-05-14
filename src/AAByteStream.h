// AppleArchive byte streams

#pragma once

#ifndef __APPLE_ARCHIVE_H
#error Include AppleArchive.h instead of this file
#endif

#if __has_feature(assume_nonnull)
_Pragma("clang assume_nonnull begin")
#endif

#pragma mark - Stream functions

#ifndef AAByteStream_h
#define AAByteStream_h

/*!
  @abstract Sequential write

  @param s ByteStream
  @param buf provides the bytes to write
  @param nbyte number of bytes to write

  @return number of bytes written on success, and a negative error code on failure or if \p write is not implemented
*/
APPLE_ARCHIVE_API ssize_t AAByteStreamWrite(
  AAByteStream s,
  const void * buf,
  size_t nbyte)
APPLE_ARCHIVE_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

/*!
  @abstract Random-access write

  @param s ByteStream
  @param buf provides the bytes to write
  @param nbyte number of bytes to write
  @param offset write location in stream

  @return number of bytes written on success, and a negative error code on failure or if \p pwrite is not implemented
*/
APPLE_ARCHIVE_API ssize_t AAByteStreamPWrite(
  AAByteStream s,
  const void * buf,
  size_t nbyte,
  off_t offset)
APPLE_ARCHIVE_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

/*!
  @abstract Sequential read

  @param s ByteStream
  @param buf receives the bytes to read
  @param nbyte number of bytes to read

  @return number of bytes read on success, and a negative error code on failure or if \p read is not implemented
*/
APPLE_ARCHIVE_API ssize_t AAByteStreamRead(
  AAByteStream s,
  void * buf,
  size_t nbyte)
APPLE_ARCHIVE_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

/*!
  @abstract Random-access read

  @param s ByteStream
  @param buf receives the bytes to read
  @param nbyte number of bytes to read
  @param offset read location in stream

  @return number of bytes read on success, and a negative error code on failure or if \p pread is not implemented
*/
APPLE_ARCHIVE_API ssize_t AAByteStreamPRead(
  AAByteStream s,
  void * buf,
  size_t nbyte,
  off_t offset)
APPLE_ARCHIVE_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

/*!
  @abstract Seek

  @discussion
  Set internal stream position to \p offset, relative to \p whence, one of SEEK_SET, SEEK_CUR, SEEK_END

  @param s ByteStream
  @param offset new location relative to origin
  @param whence origin

  @return the new stream position on success, relative to the beginning of the stream, and a negative value on failure
*/
APPLE_ARCHIVE_API off_t AAByteStreamSeek(
  AAByteStream s,
  off_t offset,
  int whence)
APPLE_ARCHIVE_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

/*!
  @abstract Cancel, the stream still needs to be closed

  @discussion Asynchronous cancellation. Subsequent calls to the stream are expected to fail.

  @param s ByteStream
*/
APPLE_ARCHIVE_API void AAByteStreamCancel(
  AAByteStream s)
APPLE_ARCHIVE_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

/*!
  @abstract Close stream

  @discussion Destroy the stream and release all resources.  The stream handle becomes invalid after this call.

  @param s ByteStream, ignored if NULL

  @return 0 on success, a negative value on failure
*/
APPLE_ARCHIVE_API int AAByteStreamClose(
  AAByteStream _Nullable s)
APPLE_ARCHIVE_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

#pragma mark - Stream objects

/*!
  @abstract Create file stream with an open file descriptor

  @discussion
  All calls are directly mapped to the read, write, etc. system calls.

  @param fd is the opened file descriptor
  @param automatic_close if not 0, we'll close(fd) when the stream is closed

  @return a new stream instance on success, and NULL on failure
*/
APPLE_ARCHIVE_API AAByteStream _Nullable AAFileStreamOpenWithFD(
  int fd,
  int automatic_close)
APPLE_ARCHIVE_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

/*!
  @abstract Open a new file descriptor and create file stream

  @discussion
  The file is opened with open(path, open_flags, open_mode).
  All calls are directly mapped to the read, write, etc. system calls.
  We will call close(fd) when the stream is destroyed.

  @param path is the file to open
  @param open_flags are the flags passed to open(2)
  @param open_mode is the creation mode passed to open(2)

  @return a new stream instance on success, and NULL on failure
*/
APPLE_ARCHIVE_API AAByteStream _Nullable AAFileStreamOpenWithPath(
  const char * path,
  int open_flags,
  mode_t open_mode)
APPLE_ARCHIVE_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

#endif /* AAByteStream_h */

#if __has_feature(assume_nonnull)
_Pragma("clang assume_nonnull end")
#endif
