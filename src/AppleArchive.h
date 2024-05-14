//
//  libAppleArchive.h
//  libAppleArchive
//

#ifndef __APPLE_ARCHIVE_H
#define __APPLE_ARCHIVE_H

#include <stdio.h>
#include <stdlib.h>

// No other headers are included in the AA API headers
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/acl.h>

// All the API headers will use these macros

// Availability
#if __has_include( <Availability.h> ) && !__BUILDING_APPLE_ARCHIVE
#include <Availability.h>
#define APPLE_ARCHIVE_AVAILABLE __API_AVAILABLE
#else
#define APPLE_ARCHIVE_AVAILABLE(...)
#endif

// Swift availability
#if __has_attribute(swift_private)
#define APPLE_ARCHIVE_SWIFT_PRIVATE __attribute__((swift_private))
#else
#define APPLE_ARCHIVE_SWIFT_PRIVATE
#endif

// API functions
#define APPLE_ARCHIVE_API __attribute__((visibility("default"))) APPLE_ARCHIVE_SWIFT_PRIVATE
#define APPLE_ARCHIVE_INLINE static inline __attribute__((always_inline,overloadable)) APPLE_ARCHIVE_SWIFT_PRIVATE

// Fixed size enums
#if __has_extension(cxx_fixed_enum)
#define APPLE_ARCHIVE_ENUM(name,type) enum name : type
#else
#define APPLE_ARCHIVE_ENUM(name,type) enum name
#endif

// API version
#define APPLE_ARCHIVE_API_VERSION 1

#include "ParallelCompression.h"
#include "AADefs.h"
#include "AACustomByteStream.h"
#include "AAByteStream.h"
#include "AAFieldKeys.h"
#include "AAEntryMessage.h"
#include "AAFlagSet.h"
#include "AAArchiveStream.h"

#endif /* libAppleArchive_h */
