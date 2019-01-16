#ifndef Room_hpp
#define Room_hpp

#include <stdio.h>
#include "Protocol.hpp"
#include "Connection.hpp"
#include "DeltaContainer.hpp"

class Room : public DeltaContainer
{
public:
    Room (const std::string&, cocos2d::Ref*);
    virtual ~Room();
    
    void connect(std::string& endpoint);

    // Methods
    void setState(msgpack::object state, int remoteCurrentTime, int remoteElapsedTime);
    void leave(bool requestLeave);
    void applyPatch (const char* bytes, int len);
    // void emitError (MessageEventArgs *args);
    
    // Callbacks
    std::function<void()> onJoin;
    std::function<void()> onLeave;
    std::function<void(cocos2d::Ref*)> onError;
    std::function<void(cocos2d::Ref*, msgpack::object*)> onMessage;
    std::function<void(cocos2d::Ref*)> onStateChange;
    
    // Properties
    Connection* connection;

    std::string id;
    std::string name;
    std::string sessionId;

private:
    cocos2d::Ref* options;
    
    void _onOpen();
    void _onClose();
    void _onError(const WebSocket::ErrorCode&);
    void _onMessage(const WebSocket::Data&);

    CC_SYNTHESIZE(int, _id, ID);
    CC_SYNTHESIZE(char *, _previousState, PreviousState);
    CC_SYNTHESIZE(int, _previousStateSize, PreviousStateSize);
};
#endif /* Room_hpp */
