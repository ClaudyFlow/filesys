#ifndef INC_ACCESS
#define INC_ACCESS

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

#pragma region function
uint32_t file_access(uint32_t user_id, struct inode *inode, uint16_t mode);
#pragma endregion function


#endif /* INC_ACCESS */
