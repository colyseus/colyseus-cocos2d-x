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
    this->endpoint = endpoint;
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

Room* Client::join(const std::string roomName, std::map<std::string, std::string> options)
{
    int requestIdInt = ++this->requestId;
    std::string requestId = std::to_string(requestIdInt);
    options.insert(std::make_pair("requestId", requestId));

    auto room = new Room(roomName, options);
    this->_connectingRooms.insert(std::make_pair(requestIdInt, room));
    this->connection->send((int)Protocol::JOIN_REQUEST, roomName, options);

    return room;
}

void Client::_onOpen()
{
    if (!id.empty() && this->onOpen) {
        this->onOpen(this);
    }
}

void Client::_onClose()
{
    if (this->onClose) {
        this->onClose(this);
    }
}

void Client::_onError(const WebSocket::ErrorCode& error)
{
    std::string message = "";
    
    switch (error) {
        case WebSocket::ErrorCode::CONNECTION_FAILURE:
            message = "CONNECTION_FAILURE";
            break;
        case WebSocket::ErrorCode::TIME_OUT:
            message = "TIME_OUT";
            break;
        case WebSocket::ErrorCode::UNKNOWN:
            message = "UNKNOWN";
            break;
    }
    
    if (this->onError) {
        this->onError(this, message);
    }
}

void Client::_onMessage(const WebSocket::Data& data)
{
    size_t len = data.len;
    const char *bytes = data.bytes;

    if (this->previousCode == 0) {
        unsigned char code = bytes[0];

        switch ((Protocol) code)
        {
            case Protocol::USER_ID:
            {

#ifdef COLYSEUS_DEBUG
                log("Protocol::USER_ID");
#endif
                
                id = colyseus_readstr(bytes, 1);
                

                std::cout << std::endl;
                std::cout << "client->id = '" << id << "'" << std::endl;
                std::cout << std::endl;


                if (this->onOpen) {
                    this->onOpen(this);
                }
                break;
            }
            case Protocol::JOIN_REQUEST:
            {

#ifdef COLYSEUS_DEBUG
                log("Protocol::JOIN_REQUEST");
#endif
                int requestId = bytes[1];
                Room* room = this->_connectingRooms.at(requestId);
                room->id = colyseus_readstr(bytes, 2);

                std::cout << std::endl;
                std::cout << "room->id = '" << room->id << "'" << std::endl;
                std::cout << std::endl;

                room->connect(this->createConnection(room->id, room->options));
                break;
            }
            case Protocol::JOIN_ERROR:
            {
#ifdef COLYSEUS_DEBUG
                log("Protocol::JOIN_ERROR");
#endif
                std::string message = colyseus_readstr(bytes, 1);
                std::cout << "Colyseus: Error Joining Room: " << message << std::endl;
                
                if (this->onError) {
                    this->onError(this, message);
                }
                
                break;
            }
            default:
            {
                this->previousCode = code;
                break;
            }
        }

    } else {
        if (this->previousCode == (int) Protocol::ROOM_LIST) {
            std::cout << len << std::endl;

            // TODO: ROOM_LIST
            /*
             msgpack::object_handle oh = msgpack::unpack(bytes, len);
             msgpack::object obj = oh.get();
             
             Protocol protocol = (Protocol) obj.via.array.ptr[0].via.i64;
             msgpack::object_array message(obj.via.array);
             */
        }
        this->previousCode = 0;
    }
}

Connection* Client::createConnection(std::string& path, std::map<std::string, std::string> options)
{
    std::vector<std::string> params;

    // append colyseusid to connection string.
    params.push_back("colyseusid=" + this->id);

    for (const auto& kv : options)
    {
        params.push_back(kv.first + "=" + kv.second);
    }

    std::string queryString;

    // concat "params" with "&" separator to form query string
    for (std::vector<std::string>::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        queryString += *p;
        if (p != params.end() - 1)
        {
            queryString += "&";
        }
    }

    return new Connection( this->endpoint + "/" + path + "?" + queryString );
}

void Client::leaveRoomHandle(msgpack::object_array data)
{
    std::string roomID = data.ptr[1].convert(id);

    std::map<const std::string,Room*>::iterator it = this->_rooms.begin();
    while (it != _rooms.end())
    {
        if(it->second->id == roomID)
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

void Client::close()
{
    this->connection->close();
}
