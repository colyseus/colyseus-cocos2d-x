#ifndef Protocol_hpp
#define Protocol_hpp

enum class Protocol {
    // User-related (0~10)
    USER_ID = 1,

    // Room-related (10~20)
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
