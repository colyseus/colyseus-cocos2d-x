#ifndef Connection_hpp
#define Connection_hpp

#include <stdio.h>
#include <functional>
#include "msgpack.hpp"

#include "cocos2d.h"
#include "network/WebSocket.h"

using namespace cocos2d::network;

typedef std::function<void(cocos2d::Ref*,cocos2d::Ref*)> RoomEventHandle;

class Connection : public WebSocket::Delegate
{
protected:
    WebSocket* _ws;

    virtual void onOpen(WebSocket* ws) override;
    virtual void onMessage(WebSocket* ws, const WebSocket::Data& data) override;
    virtual void onClose(WebSocket* ws) override;
    virtual void onError(WebSocket* ws, const WebSocket::ErrorCode& error) override;

public:
    Connection(const std::string& _endpoint);
    virtual ~Connection();

    // Methods
    void open();
    void close();
    WebSocket::State getReadyState();

    // Callbacks
    std::function<void()> _onOpen;
    std::function<void()> _onClose;
    std::function<void(const WebSocket::Data&)> _onMessage;
    std::function<void(const WebSocket::ErrorCode&)> _onError;

    // Properties
    std::string endpoint;

    template <typename... Args>
    inline void send(Args... args)
    {
        msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> pk(&buffer);
        msgpack::type::make_define_array(args...).msgpack_pack(pk);
        _ws->send((unsigned char *)buffer.data(), buffer.size());
    }

    inline void send(unsigned char *buffer,unsigned int size)
    {
        _ws->send(buffer, size);
    }

};
#endif /* Connection_hpp */
