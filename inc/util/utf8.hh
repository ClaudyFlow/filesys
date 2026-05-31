#ifndef INC_UTIL_UTF8
#define INC_UTIL_UTF8

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

#pragma region function
bool utf8_encode(char *buf, uint32_t cp, int32_t *out_len);
bool utf8_decode(const char *buf, int32_t start, uint32_t *cp, int32_t *blen);
#pragma endregion function


#endif /* INC_UTIL_UTF8 */
