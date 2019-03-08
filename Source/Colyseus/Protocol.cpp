#include "Protocol.hpp"

char* colyseus_readstr(const char* bytes, int offset)
{
    int str_size = (int) bytes[offset] + 1;
    char* str = new char[str_size];
    memcpy(str, bytes + offset + 1, str_size);
    str[str_size-1] = '\0'; // endl
    return str;
}
