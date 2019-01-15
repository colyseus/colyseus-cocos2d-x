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

class Room;
using namespace cocos2d::network;

class Client
{
public:
    Client(const std::string& endpoint);
    virtual ~Client();
    
    // Methods
    void close();
    void open();

    Room* join(const std::string& roomName, cocos2d::Ref* options);
    Room* rejoin(const std::string& roomName, std::string& sessionId);
    
    void recvUserHandle(msgpack::object_array data);
    void joinRoomHandle(msgpack::object_array data);
    void joinRoomErrorDRoomHandle(msgpack::object_array data);
    void leaveRoomHandle(msgpack::object_array data);
    void roomPatchStateHandle(msgpack::object_array data);
    void roomStateHandle(msgpack::object_array data);
    void roomDataHandle(msgpack::object_array data);
    void badRequestHandle(msgpack::object_array data);

    // Properties
    Connection* connection;
    std::string id;
    
    // Callbacks
    std::function<void>* onOpen;
    std::function<void>* onClose;
    std::function<void(std::string)>* onError;
    
    Room* getRoomByName(const std::string& name);
    Room* getRoomByID(int ID);
    
//    template <typename... Args>
//    inline void send(Args... args)
//    {
//        msgpack::sbuffer buffer;
//        msgpack::packer<msgpack::sbuffer> pk(&buffer);
//        msgpack::type::make_define_array(args...).msgpack_pack(pk);
//        _ws->send((unsigned char *)buffer.data(),buffer.size());
//    }
private:
    bool parseMsg(const char *data, int len);
    std::map<const std::string,Room*> _rooms;
};

#endif /* Client_hpp */
