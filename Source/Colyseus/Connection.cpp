#include <sstream>
#include <iostream>

#include "msgpack.hpp"
#include "Protocol.hpp"
#include "Connection.hpp"

using namespace cocos2d;
using namespace cocos2d::network;

Connection::Connection(const std::string& _endpoint)
{
    endpoint = _endpoint;
}

Connection::~Connection()
{
    CC_SAFE_DELETE(_ws);
}

void Connection::open()
{
    _ws = new WebSocket();
    if(!_ws->init(*this, endpoint))
    {
        // TODO: call _onError too
        // this->_onError(this, new WebSocket::ErrorCode);

        this->onClose(_ws);
    }
}

WebSocket::State Connection::getReadyState()
{
    return this->_ws->getReadyState();
}

void Connection::close()
{
    this->_ws->closeAsync();
}

void Connection::onOpen(WebSocket* ws)
{
    log("Websocket (%p) opened", ws);
    if (this->_onOpen) {
        this->_onOpen();
    }
}

void Connection::onMessage(WebSocket* ws, const WebSocket::Data& data)
{
    if (this->_onMessage) {
        this->_onMessage(data);
    }
}

void Connection::onClose(WebSocket* ws)
{
    log("websocket instance (%p) closed.", ws);

    CC_SAFE_DELETE(ws);

    if (this->_onClose) {
        this->_onClose();
    }
}

void Connection::onError(WebSocket* ws, const WebSocket::ErrorCode& error)
{
    log("Error was fired, error code: %d", error);
    if (this->_onError) {
        this->_onError(error);
    }
}
