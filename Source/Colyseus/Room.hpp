#ifndef Room_hpp
#define Room_hpp

#include <stdio.h>
#include "Protocol.hpp"
#include "Connection.hpp"
#include "StateContainer.hpp"
#include "Serializer/Serializer.hpp"

class Room 
{
public:
    Room (const std::string, std::map<std::string, std::string>);
    virtual ~Room();

    // Methods
    void connect(Connection* connection);
    void leave(bool consented=true);
    
    template <typename T>
    inline void send (T data)
    {
        this->connection->send((int)Protocol::ROOM_DATA, this->id, data);
    }

    msgpack::object_handle* getState();
    Listener<FallbackAction> listen(FallbackAction callback);
    Listener<PatchAction> listen(std::string segments, PatchAction callback, bool immediate=false);
    void removeListener(Listener<PatchAction> &listener);
    
    // Callbacks
    std::function<void()> onJoin;
    std::function<void()> onLeave;
    std::function<void(Room*, msgpack::object)> onMessage;
    std::function<void(Room*)> onStateChange;
    std::function<void(Room*, std::string)> onError;
    
    // Properties
    Connection* connection;
    std::string id;
    std::map<std::string, std::string> options;

    std::string name;
    std::string sessionId;
    std::string serializerId;

protected:
    void _onClose();
    void _onError(const WebSocket::ErrorCode&);
    void _onMessage(const WebSocket::Data&);
    
    void setState(const char*, int);
    void applyPatch(const char*, int);

    Serializer* serializer;
    unsigned char previousCode = 0;
};
#endif /* Room_hpp */
