#ifndef INC_IALLFRE
#define INC_IALLFRE

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

#pragma region function
struct inode *ialloc(void);
void ifree(uint32_t dinodeid);
#pragma endregion function


#endif /* INC_IALLFRE */
