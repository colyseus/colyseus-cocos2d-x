//
//  Room.hpp
//  CocosEngine
//
//  Created by Hung Hoang Manh on 3/22/17.
//
//

#ifndef Room_hpp
#define Room_hpp

#include <stdio.h>
#include "MessageEventArgs.hpp"
#include "Protocol.hpp"
#include "Connection.hpp"
#include "DeltaContainer.hpp"

typedef std::function<void(cocos2d::Ref*,cocos2d::Ref*)> RoomEventHandle;

class Room : public cocos2d::Ref
{
public:
    Room (const std::string& name, cocos2d::Ref* options);
    virtual ~Room();

    // Methods
    void setState(msgpack::object state, int remoteCurrentTime, int remoteElapsedTime);
    void leave(bool requestLeave);
    void receiveData (NetworkData* data);
    void applyPatch (const char* bytes, int len);
    void emitError (MessageEventArgs *args);
    
    // Callbacks
    RoomEventHandle _onJoinRoom;
    RoomEventHandle _onError;
    RoomEventHandle _onLeave;
    RoomEventHandle _onPatch;
    RoomEventHandle _onData;
    RoomEventHandle _onSetRoomState;
    
    // Properties
    Connection* connection;
    std::string name;

private:
    CC_SYNTHESIZE(DeltaContainer *, _state, State);
    CC_SYNTHESIZE(int, _id, ID);
    CC_SYNTHESIZE(char *, _previousState, PreviousState);
    CC_SYNTHESIZE(int, _previousStateSize, PreviousStateSize);
};
#endif /* Room_hpp */
