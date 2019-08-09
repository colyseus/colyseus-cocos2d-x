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
// #include "Auth.hpp"

using namespace cocos2d;
using namespace cocos2d::network;

typedef std::map<std::string, std::string> JoinOptions;

class Client : public cocos2d::Ref
{
public:
    Client(const std::string &endpoint)
    {
        this->endpoint = endpoint;
        this->connection = new Connection(endpoint);
        // this->auth = new Auth(endpoint);
    }

    ~Client()
    {
        delete this->connection;
    }

    // Methods
    void close()
    {
        this->connection->close();
    }

    void connect()
    {
        this->connection->_onOpen = CC_CALLBACK_0(Client::_onOpen, this);
        this->connection->_onClose = CC_CALLBACK_0(Client::_onClose, this);
        this->connection->_onError = CC_CALLBACK_1(Client::_onError, this);
        this->connection->_onMessage = CC_CALLBACK_1(Client::_onMessage, this);
        this->connection->open();
    }

    template <typename S>
    Room<S> *join(const std::string roomName, JoinOptions options = JoinOptions())
    {
        int requestIdInt = ++this->requestId;
        std::string requestId = std::to_string(requestIdInt);
        options.insert(std::make_pair("requestId", requestId));

        std::cout << "let's create room instance..." << std::endl;
        char* room = (char*)new Room<S>(roomName, options);
        std::cout << "let's cast to IRoom*" << std::endl;
        IRoom* ref = (IRoom*) room;
        std::cout << "okay..." << std::endl;

        this->_connectingRooms.insert(std::make_pair(requestIdInt, ref));
        this->connection->send((int)Protocol::JOIN_REQUEST, roomName, options);

        std::cout << "let's return the Room<S> instance..." << std::endl;
        return (Room<S> *)room;
    }

    // template <typename S>
    // Room<S>* rejoin(const std::string roomName, std::string& sessionId);

    // Properties
    Connection* connection;
    std::string id;
    // Auth* auth;

    // Callbacks
    std::function<void()> onOpen;
    std::function<void()> onClose;
    std::function<void(std::string)> onError;

protected:
    void _onOpen()
    {
        if (!id.empty() && this->onOpen)
        {
            this->onOpen();
        }
    }

    void _onClose()
    {
        if (this->onClose)
        {
            this->onClose();
        }
    }

    void _onError(const WebSocket::ErrorCode &error)
    {
        std::string message = "";

        switch (error)
        {
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

        if (this->onError)
        {
            this->onError(message);
        }
    }

    void _onMessage(const WebSocket::Data &data)
    {
        size_t len = data.len;
        const char *bytes = data.bytes;

        if (this->previousCode == 0)
        {
            unsigned char code = bytes[0];

            switch ((Protocol)code)
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

                if (this->onOpen)
                {
                    this->onOpen();
                }
                break;
            }
            case Protocol::JOIN_REQUEST:
            {

#ifdef COLYSEUS_DEBUG
                log("Protocol::JOIN_REQUEST");
#endif

                int requestId = bytes[1];
                IRoom *room = this->_connectingRooms.at(requestId);
                room->id = colyseus_readstr(bytes, 2);

                std::string processPath = "";
                int nextIndex = 3 + room->id.length();
                if (len > nextIndex) {
                    processPath = std::string(colyseus_readstr(bytes, nextIndex)) + "/";
                }

                std::cout << std::endl;
                std::cout << "room->id = '" << room->id << "'" << std::endl;
                std::cout << std::endl;

                room->connect(this->createConnection(processPath + room->id, room->options));
                break;
            }
            case Protocol::JOIN_ERROR:
            {
#ifdef COLYSEUS_DEBUG
                log("Protocol::JOIN_ERROR");
#endif
                std::string message = colyseus_readstr(bytes, 1);
                std::cout << "Colyseus: Error Joining Room: " << message << std::endl;

                if (this->onError)
                {
                    this->onError(message);
                }

                break;
            }
            default:
            {
                this->previousCode = code;
                break;
            }
            }
        }
        else
        {
            if (this->previousCode == (int)Protocol::ROOM_LIST)
            {
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

    Connection *createConnection(std::string path, JoinOptions options = JoinOptions())
    {
        std::vector<std::string> params;

        // append colyseusid to connection string.
        params.push_back("colyseusid=" + this->id);

        for (const auto &kv : options)
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

        return new Connection(this->endpoint + "/" + path + "?" + queryString);
    }

    std::string endpoint;
    std::map<int, IRoom*> _connectingRooms;
    int requestId = 0;
    unsigned char previousCode = 0;
};

#endif /* Client_hpp */
