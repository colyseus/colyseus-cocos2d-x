#ifndef Room_hpp
#define Room_hpp

#include <stdio.h>
#include "Protocol.hpp"
#include "Connection.hpp"

#include "schema.h"
#include "Serializer/Serializer.hpp"
#include "Serializer/SchemaSerializer.hpp"

class IRoom {
public:
    virtual ~IRoom() = default;

    std::string id;
    std::map<std::string, std::string> options;
    virtual void connect(Connection *connection) = 0;
};

template <typename S>
class Room : IRoom
{
public:
    Room(const std::string _name, std::map<std::string, std::string> _options)
    {
        name = _name;
        options = _options;
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
        this->connection->send((int)Protocol::ROOM_DATA, this->id, data);
    }

    S *getState()
    {
        return this->serializer->getState();
    }

    // Callbacks
    std::function<void()> onJoin;
    std::function<void()> onLeave;
    std::function<void(msgpack::object)> onMessage;
    std::function<void(S*)> onStateChange;
    std::function<void(std::string)> onError;

    // Properties
    Connection* connection;

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
        const char *bytes = data.bytes;

        if (this->previousCode == 0)
        {
            unsigned char code = bytes[0];

            switch ((Protocol)code)
            {
            case Protocol::JOIN_ROOM:
            {
                int offset = 1;

                sessionId = colyseus_readstr(bytes, offset);
                offset += sessionId.length() + 1;

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
            default:
            {
                this->previousCode = code;
                break;
            }
            }
        }
        else
        {
            switch ((Protocol)this->previousCode)
            {
            case Protocol::ROOM_DATA:
            {
#ifdef COLYSEUS_DEBUG
                std::cout << "Colyseus.Room: ROOM_DATA" << std::endl;
#endif
                if (this->onMessage)
                {

                    msgpack::object_handle oh = msgpack::unpack(bytes, len);
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
                this->setState(bytes, len);
                break;
            }
            case Protocol::ROOM_STATE_PATCH:
            {
#ifdef COLYSEUS_DEBUG
                std::cout << "Colyseus.Room: ROOM_STATE_PATCH" << std::endl;
#endif
                this->applyPatch(bytes, len);

                break;
            }
            default:
                break;
            }

            this->previousCode = 0;
        }
    }

    void setState(const char *bytes, int length)
    {
        this->serializer->setState(bytes, length);

        if (onStateChange)
        {
            this->onStateChange(this->getState());
        }
    }

    void applyPatch(const char *bytes, int length)
    {
        this->serializer->patch(bytes, length);

        if (onStateChange)
        {
            this->onStateChange(this->getState());
        }
    }

    Serializer<S>* serializer;
    unsigned char previousCode = 0;
};
#endif /* Room_hpp */
