#if !defined(_ENDIAN_H) && !defined(_ENDIAN_H_)
#define _ENDIAN_H_
#ifdef __INTEL_COMPILER
#define __builtin_bswap16(v) ((uint16_t)_bswap16((__int16)(v)))
#define __builtin_bswap32(v) ((uint32_t)_bswap((int)(v)))
#define __builtin_bswap64(v) ((uint64_t)_bswap64((__int64)(v)))
#elif _MSC_VER >= 1400
#define __builtin_bswap16(v) ((uint16_t)_byteswap_ushort((unsigned short)(v)))
#define __builtin_bswap32(v) ((uint32_t)_byteswap_ulong((unsigned long)(v)))
#define __builtin_bswap64(v) ((uint64_t)_byteswap_uint64((unsigned __int64)(v)))
#endif

#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ ==  __ORDER_BIG_ENDIAN__) || \
	(defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) || \
	(defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN)
#define htobe16
#define htole16 __builtin_bswap16
#define be16toh
#define le16toh __builtin_bswap16
#define htobe32
#define htole32 __builtin_bswap32
#define be32toh
#define le32toh __builtin_bswap32
#define htobe64
#define htole64 __builtin_bswap64
#define be64toh
#define le64toh __builtin_bswap64
#else
#define htobe16 __builtin_bswap16
#define htole16
#define be16toh __builtin_bswap16
#define le16toh
#define htobe32 __builtin_bswap32
#define htole32
#define be32toh __builtin_bswap32
#define le32toh
#define htobe64 __builtin_bswap64
#define htole64
#define be64toh __builtin_bswap64
#define le64toh
#endif
#endif
