#ifndef INC_BALLFRE
#define INC_BALLFRE

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

#pragma region function
uint32_t balloc(void);
uint32_t bfree(uint32_t block_num);
#pragma endregion function


#endif /* INC_BALLFRE */
