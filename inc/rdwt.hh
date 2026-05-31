#ifndef INC_RDWT
#define INC_RDWT

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

#pragma region function
uint32_t fs_read(uint16_t cfd, uint32_t user_id, char *buf, uint32_t len);
uint32_t fs_write(uint16_t cfd, uint32_t user_id, char *buf, uint32_t len);
#pragma endregion function


#endif /* INC_RDWT */
