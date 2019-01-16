//
//  Client.hpp
//  CocosEngine
//
//  Created by Hung Hoang Manh on 3/22/17.
//
//

#ifndef Client_hpp
#define Client_hpp

#include <stdio.h>

#include "network/WebSocket.h"
#include "msgpack.hpp"
#include "Connection.hpp"
#include "Room.hpp"

using namespace cocos2d::network;

class Client : public cocos2d::Ref
{
public:
    Client(const std::string& endpoint);
    virtual ~Client();
    
    // Methods
    void close();
    void connect();

    Room* join(const std::string& roomName, cocos2d::Ref* options = nullptr);
    Room* rejoin(const std::string& roomName, std::string& sessionId);

    // Properties
    Connection* connection;
    std::string id;
    
    // Callbacks
    std::function<void(cocos2d::Ref*)> onOpen;
    std::function<void(cocos2d::Ref*)> onClose;
    std::function<void(cocos2d::Ref*, const WebSocket::ErrorCode&)> onError;
    
    Room* getRoomByName(const std::string& name);
    Room* getRoomByID(int ID);

private:
    void _onOpen();
    void _onClose();
    void _onError(const WebSocket::ErrorCode&);
    void _onMessage(const WebSocket::Data&);
    
    void joinRoomHandle(msgpack::object_array data);
    void joinRoomErrorDRoomHandle(msgpack::object_array data);
    void leaveRoomHandle(msgpack::object_array data);
    
    std::map<const std::string,Room*> _rooms;
};

#endif /* Client_hpp */
