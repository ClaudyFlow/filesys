#ifndef INC_IGETPUT
#define INC_IGETPUT

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

#pragma region function
struct inode *iget(uint32_t dinodeid);
void iput(struct inode *pinode);
#pragma endregion function

#endif /* INC_IGETPUT */
