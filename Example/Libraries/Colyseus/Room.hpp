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
    
    void connect(Connection* connection);

    // Methods
    void setState(msgpack::object state, int remoteCurrentTime, int remoteElapsedTime);
    void leave(bool requestLeave);
    void applyPatch (const char* bytes, int len);
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
    
    void _onOpen();
    void _onClose();
    void _onError(const WebSocket::ErrorCode&);
    void _onMessage(const WebSocket::Data&);

    CC_SYNTHESIZE(char *, _previousState, PreviousState);
    CC_SYNTHESIZE(int, _previousStateSize, PreviousStateSize);
};
#endif /* Room_hpp */
