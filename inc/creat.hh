#ifndef INC_CREAT
#define INC_CREAT

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

#pragma region function
void fs_creat(uint32_t user_id, char *filename, uint16_t mode);
#pragma endregion function


#endif /* INC_CREAT */
