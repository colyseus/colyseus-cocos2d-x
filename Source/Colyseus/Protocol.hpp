#ifndef Protocol_hpp
#define Protocol_hpp

char* colyseus_readstr(const char* bytes, int offset);

enum class Protocol : int {
    // User-related (0~8)
    USER_ID = 1,

    // Room-related (9~19)
    JOIN_REQUEST = 9,
    JOIN_ROOM = 10,
    JOIN_ERROR = 11,
    LEAVE_ROOM = 12,
    ROOM_DATA = 13,
    ROOM_STATE = 14,
    ROOM_STATE_PATCH = 15,

    // Match-making related (20~29)
    ROOM_LIST = 20,

    // Generic messages (50~60)
    BAD_REQUEST = 50,
};

#endif /* Protocol_hpp */
