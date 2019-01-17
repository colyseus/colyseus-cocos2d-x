#include "Room.hpp"

#define FOSSIL_ENABLE_DELTA_CKSUM_TEST 1 // enable checksum on fossil-delta
#include "fossil/delta.c"

#include <string.h>
#include <sstream>
#include <iostream>

using namespace cocos2d;

Room::Room (const std::string _name, std::map<std::string, std::string> _options)
{
    name = _name;
    options = _options;

    _previousState = NULL;
    _previousStateSize = 0;
}

Room::~Room()
{
    if (_previousState)
    {
        delete _previousState;
    }
}

void Room::connect(Connection* connection)
{
    this->connection = connection;
    this->connection->_onClose = CC_CALLBACK_0(Room::_onClose, this);
    this->connection->_onError = CC_CALLBACK_1(Room::_onError, this);
    this->connection->_onMessage = CC_CALLBACK_1(Room::_onMessage, this);
    this->connection->open();
}

void Room::_onClose()
{
    std::cout << "Room::_onClose started" << std::endl;
    if (this->onLeave) {
        this->onLeave();
    }
    std::cout << "Room::_onClose finished" << std::endl;
}

void Room::_onError(const WebSocket::ErrorCode& error)
{
    if (this->onError) {
        this->onError(this, error);
    }
}

void Room::_onMessage(const WebSocket::Data& data)
{
    std::cout << "Room:_onMessage" << std::endl;

    size_t len = data.len;
    const char *bytes = data.bytes;
    
    msgpack::object_handle oh = msgpack::unpack(bytes, len);
    msgpack::object obj = oh.get();
    
    std::cout << "------------------------ROOM-RAW-----------------------------" << std::endl;
    std::cout << obj << std::endl;
    std::cout << "-------------------------------------------------------------" << std::endl;
    
    Protocol protocol = (Protocol) obj.via.array.ptr[0].via.i64;
    msgpack::object_array message(obj.via.array);
    
    switch (protocol)
    {
        case Protocol::JOIN_ROOM:
        {
            roomId = message.ptr[1].convert(roomId);
            
            if (this->onJoin) {
                this->onJoin();
            }
            break;
        }
        case Protocol::JOIN_ERROR:
        {
            log("Colyseus.Room: join error");
            //this->onError(this);
            break;
        }
        case Protocol::LEAVE_ROOM:
        {
            log("Colyseus.Room: LEAVE_ROOM");
            break;
        }
        case Protocol::ROOM_DATA:
        {
            log("Colyseus.Room: ROOM_DATA");
            if (this->onMessage) {
                msgpack::object data;
                data = message.ptr[1].convert(data);
                this->onMessage(this, data);
            }
            break;
        }
        case Protocol::ROOM_STATE:
        {
            log("Colyseus.Room: ROOM_STATE");
            
            int64_t remoteCurrentTime = message.ptr[2].via.i64;
            int64_t remoteElapsedTime = message.ptr[3].via.i64;
            
            this->setState(message.ptr[1].via.bin, (int)remoteCurrentTime, (int)remoteElapsedTime);
            break;
        }
        case Protocol::ROOM_STATE_PATCH:
        {
            log("Colyseus.Room: ROOM_STATE_PATCH");
            
            msgpack::object_array patchBytes = message.ptr[1].via.array;
            
            char * patches = new char[patchBytes.size];
            for(int idx = 0; idx < patchBytes.size ; idx++)
            {
                patches[idx] = patchBytes.ptr[idx].via.i64;
            }
            
            this->applyPatch(patches, patchBytes.size);
            delete [] patches;
            
            break;
        }
        default:
        {
            break;
        }
    }
}

void Room::setState(msgpack::object_bin encodedState, int remoteCurrentTime, int remoteElapsedTime)
{
    
    msgpack::object_handle oh = msgpack::unpack(encodedState.ptr, encodedState.size);
    msgpack::object state = oh.get();
    this->set(state);

    if (_previousState) {
        delete _previousState;
    }

    this->_previousState = encodedState.ptr;
    this->_previousStateSize = encodedState.size;

    if (onStateChange) {
        this->onStateChange(this);
    }
}

void Room::leave(bool requestLeave)
{
    if (requestLeave && !this->roomId.empty()) {
        this->connection->send ((int)Protocol::LEAVE_ROOM);
    } else {
        log("MAY BE WAITING FOR JOIN RESPONSE");

        if (onLeave) {
            this->onLeave();
        }
    }
}

void Room::applyPatch (const char* delta, int len)
{
    int newStateSize = delta_output_size(delta, len);
    char* temp = new char[newStateSize];
    
    _previousStateSize = delta_apply(_previousState, _previousStateSize, delta, len, temp);

    if (_previousState) {
        // TODO: free _previousState from memory.
        // delete [] _previousState;
    }
    _previousState = temp;

    msgpack::object_handle oh = msgpack::unpack(_previousState, _previousStateSize);
    
    this->set(oh.get());
    
    if (onStateChange) {
        this->onStateChange(this);
    }
}
