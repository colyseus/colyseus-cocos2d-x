#ifndef Room_hpp
#define Room_hpp

#include <stdio.h>
#include "Protocol.hpp"
#include "Connection.hpp"

#include "schema.h"
#include "Serializer/Serializer.hpp"
#include "Serializer/SchemaSerializer.hpp"

/*
class IRoom {
public:
    virtual ~IRoom() = default;

    std::string id;
    std::map<std::string, std::string> options;
    virtual void connect(Connection *connection) = 0;
};
*/

template <typename S>
class Room
// : IRoom
{
public:
    Room(const std::string _name)
    {
        name = _name;
        serializer = new SchemaSerializer<S>();
    }
    ~Room() {}

    // Methods
    void connect(Connection *connection)
    {
        this->connection = connection;
        this->connection->_onClose = CC_CALLBACK_0(Room::_onClose, this);
        this->connection->_onError = CC_CALLBACK_1(Room::_onError, this);
        this->connection->_onMessage = CC_CALLBACK_1(Room::_onMessage, this);
        this->connection->open();
    }

    void leave(bool consented = true)
    {
        if (!this->id.empty())
        {
            if (consented)
            {
                this->connection->send((int)Protocol::LEAVE_ROOM);
            }
            else
            {
                this->connection->close();
            }
        }
        else
        {
            if (onLeave)
            {
                this->onLeave();
            }
        }
    }
    template <typename T>
    inline void send (T data)
    {
        this->connection->send((int)Protocol::ROOM_DATA, data);
    }

    S *getState()
    {
        return this->serializer->getState();
    }

    // Callbacks
    std::function<void()> onJoin;
    std::function<void()> onLeave;
    std::function<void(const msgpack::object &)> onMessage;
    std::function<void(S*)> onStateChange;
    std::function<void(const std::string &)> onError;

    // Properties
    Connection* connection;

    std::string id;
    std::string name;
    std::string sessionId;
    std::string serializerId;

protected:
    void _onClose()
    {
        if (this->onLeave)
        {
            this->onLeave();
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
        size_t offset = 0;
        const char *bytes = data.bytes;
#ifdef COLYSEUS_DEBUG
        std::cout << "onMessage bytes =>" << bytes << std::endl;
#endif

        unsigned char code = bytes[offset++];

        switch ((Protocol)code)
        {
            case Protocol::JOIN_ROOM:
            {
#ifdef COLYSEUS_DEBUG
                std::cout << "Colyseus.Room: join error" << std::endl;
#endif

                serializerId = colyseus_readstr(bytes, offset);
                offset += serializerId.length() + 1;

                // TODO: instantiate serializer by id
                if (len > offset)
                {
                    serializer->handshake(bytes, offset);
                }

                if (this->onJoin)
                {
                    this->onJoin();
                }

                this->connection->send((int)Protocol::JOIN_ROOM);
                break;
            }
            case Protocol::JOIN_ERROR:
            {
#ifdef COLYSEUS_DEBUG
                std::cout << "Colyseus.Room: join error" << std::endl;
#endif
                std::string message = colyseus_readstr(bytes, 1);

                if (this->onError)
                {
                    this->onError(message);
                }
                break;
            }
            case Protocol::LEAVE_ROOM:
            {
#ifdef COLYSEUS_DEBUG
                std::cout << "Colyseus.Room: LEAVE_ROOM" << std::endl;
#endif
                this->leave();

                break;
            }
            case Protocol::ROOM_DATA:
            {
#ifdef COLYSEUS_DEBUG
                std::cout << "Colyseus.Room: ROOM_DATA" << std::endl;
#endif
                if (this->onMessage)
                {

                    msgpack::object_handle oh = msgpack::unpack(bytes, len, offset);
                    msgpack::object data = oh.get();

#ifdef COLYSEUS_DEBUG
                    std::cout << "-------------------Colyseus:onMessage------------------------" << std::endl;
                    std::cout << data << std::endl;
                    std::cout << "-------------------------------------------------------------" << std::endl;
#endif

                    this->onMessage(data);
                }
                break;
            }
            case Protocol::ROOM_STATE:
            {
#ifdef COLYSEUS_DEBUG
                std::cout << "Colyseus.Room: ROOM_STATE" << std::endl;
#endif
                this->setState(bytes, offset, len);
                break;
            }
            case Protocol::ROOM_STATE_PATCH:
            {
#ifdef COLYSEUS_DEBUG
                std::cout << "Colyseus.Room: ROOM_STATE_PATCH" << std::endl;
#endif
                this->applyPatch(bytes, offset, len);

                break;
            }
            default:
            {
                break;
            }
        }
    }

    void setState(const char *bytes, int offset, int length)
    {
        this->serializer->setState(bytes, offset, length);

        if (onStateChange)
        {
            this->onStateChange(this->getState());
        }
    }

    void applyPatch(const char *bytes, int offset, int length)
    {
        this->serializer->patch(bytes, offset, length);

        if (onStateChange)
        {
            this->onStateChange(this->getState());
        }
    }

    Serializer<S>* serializer;
};
#endif /* Room_hpp */
