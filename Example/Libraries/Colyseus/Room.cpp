#include "Room.hpp"

#define FOSSIL_ENABLE_DELTA_CKSUM_TEST 1 // enable checksum on fossil-delta
#include "Colyseus/fossil/delta.c"

#include <sstream>
#include <iostream>

using namespace cocos2d;

#define MAX_LEN_STATE_STR 4096

Room::Room (const std::string& _name, cocos2d::Ref* _options)
{
    name = _name;
    options = _options;
    
    connection = new Connection(nullptr);

    _id = -1;
    _previousState = NULL;
    _previousStateSize = 0;
}

Room::~Room()
{
    if (_previousState)
        delete _previousState;
}

void Room::connect(std::string& endpoint)
{
    connection->endpoint = endpoint;
    connection->_onClose = CC_CALLBACK_0(Room::_onClose, this);
    connection->_onError = CC_CALLBACK_1(Room::_onError, this);
    connection->_onMessage = CC_CALLBACK_1(Room::_onMessage, this);
    connection->open();
}

void Room::_onOpen()
{
}

void Room::_onClose()
{
    if (this->onLeave) {
        this->onLeave();
    }
}

void Room::_onError(const WebSocket::ErrorCode& error)
{
    if (this->onError) {
        this->onError(error);
    }
}

void Room::_onMessage(const WebSocket::Data& data)
{
    size_t len = data.len;
    const char *bytes = data.bytes;
    
    msgpack::object_handle oh = msgpack::unpack(bytes, len);
    msgpack::object obj = oh.get();
    
    std::cout << "------------------------ROOM-RAW-----------------------------" << std::endl;
    std::cout << obj << std::endl;
    std::cout << "-------------------------------------------------------------" << std::endl;
    
    Protocol protocol = (Protocol) obj.via.array.ptr[0].via.i64;
    msgpack::object_array message(obj.via.array);
    
    switch (protocol) {
        case Protocol::JOIN_ROOM:
            id = message.ptr[1].convert(id);
            if (onJoin) {
                this->onJoin();
            }
            break;
            
        case Protocol::JOIN_ERROR:
            log("Colyseus.Room: join error");
            //this->onError(this);
            break;
            
        case Protocol::LEAVE_ROOM:
            log("Colyseus.Room: LEAVE_ROOM");
            break;
            
        case Protocol::ROOM_DATA:
            log("Colyseus.Room: ROOM_DATA");
            if (this->onMessage) {
                this->onMessage(this, message);
            }
            break;

        case Protocol::ROOM_STATE:
            log("Colyseus.Room: ROOM_STATE");
            
            msgpack::object state;
            message.ptr[1].convert(state);

            int64_t remoteCurrentTime = message.ptr[2].via.i64;
            int64_t remoteElapsedTime = message.ptr[3].via.i64;
            
            this->setState(state, (int)remoteCurrentTime, (int)remoteElapsedTime);
            break;

        case Protocol::ROOM_STATE_PATCH:
            log("Colyseus.Room: ROOM_STATE_PATCH");
            
            msgpack::object_array patchBytes = message.ptr[1].via.array;
            
            char * patches = new char[patchBytes.size];
            for(int idx = 0; idx < patchBytes.size ; idx++)
            {
                patches[idx] = patchBytes.ptr[idx].via.i64;
            }
            
            this->applyPatch(patches,patchBytes.size);
            delete [] patches;

            break;
        default:
            break;
    }
}

void Room::setState(msgpack::object state, int remoteCurrentTime, int remoteElapsedTime)
{
    this->Set(state);

    msgpack::sbuffer temp_sbuffer;
    msgpack::packer<msgpack::sbuffer> packer(&temp_sbuffer);
    packer.pack(state);

    if (_previousState)
        delete _previousState;
    _previousState = NULL;

    _previousStateSize = (int) temp_sbuffer.size();
    _previousState = new char[temp_sbuffer.size()];
    memcpy(_previousState, temp_sbuffer.data(), temp_sbuffer.size());

    if (onStateChange) {
        this->onStateChange(this);
    }
}

void Room::leave(bool requestLeave)
{
    if (requestLeave && this->_id > 0) {
        this->connection->send ((int)Protocol::LEAVE_ROOM);
    } else {
        log("MAY BE WAITING FOR JOIN RESPONSE");
        if (onLeave) {
            this->onLeave();
        }
    }
}

void Room::applyPatch (const char* delta ,int len)
{
    char * temp = new char[MAX_LEN_STATE_STR];
    _previousStateSize = delta_apply(_previousState,_previousStateSize, delta, len, temp);

    CCASSERT(_previousStateSize < MAX_LEN_STATE_STR,"MAX_LEN_STATE_STR not enought!!");

    delete [] _previousState;
    _previousState = NULL;
    _previousState = new char[_previousStateSize];
    memcpy(_previousState, temp, _previousStateSize);
    delete [] temp;

    msgpack::object_handle oh = msgpack::unpack(_previousState, _previousStateSize);
    this->Set(oh.get());
    
    if (onStateChange) {
        this->onStateChange(this);
    }
}
