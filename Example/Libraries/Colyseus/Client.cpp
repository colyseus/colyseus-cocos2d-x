#include <sstream>
#include <iostream>

#include "msgpack.hpp"

#include "Client.hpp"
#include "Room.hpp"
#include "Protocol.hpp"

using namespace cocos2d;
using namespace cocos2d::network;

Client::Client(const std::string& endpoint)
{
    this->connection = new Connection(endpoint);
}

Client::~Client()
{
    delete this->connection;
}

void Client::connect()
{
    this->connection->_onOpen = CC_CALLBACK_0(Client::_onOpen, this);
    this->connection->_onClose = CC_CALLBACK_0(Client::_onClose, this);
    this->connection->_onError = CC_CALLBACK_1(Client::_onError, this);
    this->connection->_onMessage = CC_CALLBACK_1(Client::_onMessage, this);
    this->connection->open();
}

Room* Client::join(const std::string& roomName, cocos2d::Ref* options)
{
    if (this->_rooms.find(roomName) == _rooms.end())
    {
        std::pair<const std::string, Room*> roomPair(roomName, new Room(roomName, options));
        this->_rooms.insert(roomPair);
    }
    this->connection->send((int)Protocol::JOIN_ROOM, roomName);
    return this->_rooms.find(roomName)->second;
}

void Client::_onOpen()
{
    if (!id.empty()) {
        this->onOpen(this);
    }
}

void Client::_onClose()
{
    this->onClose(this);
}

void Client::_onError(const WebSocket::ErrorCode& error)
{
    this->onError(this, error);
}

void Client::_onMessage(const WebSocket::Data& data)
{
    size_t len = data.len;
    const char *bytes = data.bytes;

    msgpack::object_handle oh = msgpack::unpack(bytes, len);
    msgpack::object obj = oh.get();

    std::cout << "-----------------------CLIENT-RAW----------------------------" << std::endl;
    std::cout << obj << std::endl;
    std::cout << "-------------------------------------------------------------" << std::endl;

    Protocol protocol = (Protocol) obj.via.array.ptr[0].via.i64;
    msgpack::object_array message(obj.via.array);

    switch (protocol) {
        case Protocol::USER_ID:
            log("Protocol::USER_ID");
            id = message.ptr[1].convert(id);
            this->onOpen(this);
            break;

        case Protocol::JOIN_ROOM:
            log("Protocol::JOIN_ROOM");
            joinRoomHandle(message);
            break;

        case Protocol::JOIN_ERROR:
            log("Protocol::JOIN_ERROR");
            joinRoomErrorDRoomHandle(message);
            break;

        default:
            break;
    }
}

void Client::joinRoomHandle(msgpack::object_array data)
{
    int64_t roomID = data.ptr[1].via.i64;
    std::string roomName;
    data.ptr[2].convert(roomName);
    std::map<const std::string,Room*>::iterator it = this->_rooms.find(roomName);
    if(it != _rooms.end())
    {
        log("JOIN ROOM SUCCESSFUL roomid = %i , roomName = %s",(int)roomID,roomName.c_str());
        (*it).second->setID(roomID);
    }
	else
    {
        log("PLEASE CALL Client::join(), maybe error belong to Server");
    }
}

void Client::joinRoomErrorDRoomHandle(msgpack::object_array data)
{
    std::string roomName;
    data.ptr[2].convert(roomName);
    std::map<const std::string,Room*>::iterator it = this->_rooms.find(roomName);
    if(it != _rooms.end())
    {
        // (*it).second->emitError(new MessageEventArgs((*it).second,nullptr));
        _rooms.erase(it);
    }
}

void Client::leaveRoomHandle(msgpack::object_array data)
{
    int64_t roomID = data.ptr[1].via.i64;
    std::map<const std::string,Room*>::iterator it = this->_rooms.begin();
    while (it != _rooms.end())
    {
        if(it->second->getID() == roomID)
            return;
    }
}

Room* Client::getRoomByName(const std::string& name)
{
    std::map<const std::string,Room*>::iterator it = this->_rooms.find(name);
    if (it != _rooms.end())
    {
        return it->second;
    }
    return nullptr;
}

Room* Client::getRoomByID(int ID)
{
    std::map<const std::string,Room*>::iterator it = this->_rooms.begin();
    while (it != _rooms.end())
    {
        if(it->second->getID() == ID)
            return it->second;
        it++;
    }
    return nullptr;
}

void Client::close()
{
    this->connection->close();
}
