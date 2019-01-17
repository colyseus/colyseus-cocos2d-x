#ifndef Room_hpp
#define Room_hpp

#include <stdio.h>
#include "Protocol.hpp"
#include "Connection.hpp"
#include "DeltaContainer.hpp"

class Room : public DeltaContainer
{
public:
    Room (const std::string, std::map<std::string, std::string>);
    virtual ~Room();

    // Methods
    void connect(Connection* connection);
    void leave(bool requestLeave);

    void setState(msgpack::object_bin, int, int);
    void applyPatch(const char*, int);
    // void emitError (MessageEventArgs *args);
    
    // Callbacks
    std::function<void()> onJoin;
    std::function<void()> onLeave;
    std::function<void(Room*, msgpack::object)> onMessage;
    std::function<void(Room*)> onStateChange;
    std::function<void(Room*, const WebSocket::ErrorCode&)> onError;
    
    // Properties
    Connection* connection;
    std::string roomId;
    std::map<std::string, std::string> options;

    std::string name;
    std::string sessionId;

private:
    void _onClose();
    void _onError(const WebSocket::ErrorCode&);
    void _onMessage(const WebSocket::Data&);

    const char* _previousState;
    int _previousStateSize;
};
#endif /* Room_hpp */
