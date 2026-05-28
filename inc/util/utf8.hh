#ifndef INC_UTIL_UTF8
#define INC_UTIL_UTF8

#pragma region include::standard
#pragma endregion include::standard

bool utf8_encode(char *buf, unsigned int cp, int *out_len);

bool utf8_decode(const char *buf, int start, unsigned int *cp, int *blen);

#endif /* INC_UTIL_UTF8 */
