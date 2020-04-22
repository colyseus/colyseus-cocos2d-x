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
    std::function<void(const int32_t&, const std::string&)> _onError;

    // Properties
    std::string endpoint;

    inline void send(const unsigned char *buffer, unsigned int size)
    {
        _ws->send(buffer, size);
    }

};
#endif /* Connection_hpp */
