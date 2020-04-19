#ifndef Protocol_hpp
#define Protocol_hpp

enum class Protocol : int {
    // Room-related (10~19)
    JOIN_ROOM = 10,
    JOIN_ERROR = 11,
    LEAVE_ROOM = 12,
    ROOM_DATA = 13,
    ROOM_STATE = 14,
    ROOM_STATE_PATCH = 15,

    // Generic messages (50~60)
    BAD_REQUEST = 50,
};

#endif /* Protocol_hpp */
