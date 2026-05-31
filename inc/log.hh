#ifndef INC_LOG
#define INC_LOG

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

#pragma region function
void fs_login(uint16_t uid, char *passwd);
void fs_logout(uint16_t uid);
#pragma endregion function


#endif /* INC_LOG */
