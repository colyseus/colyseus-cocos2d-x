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
#include "network/HttpClient.h"

#include "thirdparty/nlohmann/json.hpp"

#include "msgpack.hpp"
#include "Connection.hpp"
#include "Room.hpp"

// #include "Auth.hpp"

using namespace cocos2d;
using namespace cocos2d::network;

typedef nlohmann::json JoinOptions;

class Client : public cocos2d::Ref
{
public:
    std::string endpoint;
    // Auth* auth;

    Client(const std::string &endpoint)
    {
        this->endpoint = endpoint;
        // this->auth = new Auth(endpoint);
    }

    ~Client() {}

    template <typename S>
    inline void joinOrCreate(const std::string roomName, JoinOptions options, std::function<void(const std::string &, Room<S> *)> callback)
    {
        this->createMatchMakeRequest<S>("joinOrCreate", roomName, options, callback);
    }

    template <typename S>
    inline void join(const std::string roomName, JoinOptions options, std::function<void(const std::string &, Room<S> *)> callback)
    {
        this->createMatchMakeRequest<S>("join", roomName, options, callback);
    }

    template <typename S>
    inline void create(const std::string roomName, JoinOptions options, std::function<void(const std::string &, Room<S> *)> callback)
    {
        this->createMatchMakeRequest<S>("create", roomName, options, callback);
    }

    template <typename S>
    inline void joinById(const std::string roomId, JoinOptions options, std::function<void(const std::string &, Room<S> *)> callback)
    {
        this->createMatchMakeRequest<S>("joinById", roomId, options, callback);
    }

    template <typename S>
    inline void reconnect(const std::string roomId, const std::string sessionId, std::function<void(const std::string &, Room<S> *)> &callback)
    {
        this->createMatchMakeRequest<S>("joinById", roomId, {{"sessionId", sessionId}}, callback);
    }

protected:
    template <typename S>
    inline void createMatchMakeRequest(
        std::string method,
        std::string roomName,
        JoinOptions options,
        std::function<void(std::string, Room<S> *)> callback
    )
    {
        HttpRequest *req = new (std ::nothrow) HttpRequest();
        req->setUrl(this->endpoint.replace(0, 2, "http") + "/matchmake/" + method + "/" + roomName);
        req->setRequestType(HttpRequest::Type::POST);

        std::vector<std::string> headers;
        headers.push_back("Accept:application/json");
        headers.push_back("Content-Type:application/json");
        req->setHeaders(headers);

        std::string data = options.dump();
        if (data == "null") { data = "{}"; }

        req->setRequestData(data.c_str(), data.length());

        req->setResponseCallback( [this, roomName, callback] (network::HttpClient* client, network::HttpResponse* response) {
            if (!response)
            {
                callback("cocos2d::network::HttpClient => no response", nullptr);
                return;
            }

            if (response && response->getResponseCode() == 200) {
                std::vector<char> *data = response->getResponseData();
                std::string json_string(data->begin(), data->end());
                auto json = nlohmann::json::parse(json_string);

                Room<S> *room = new Room<S>(roomName);
                room->id = json["room"]["roomId"].get<std::string>();
                room->sessionId = json["sessionId"].get<std::string>();

                std::string processId = json["room"]["processId"].get<std::string>();

                room->onError = [callback](std::string message) {
                    callback(message, nullptr);
                };

                room->onJoin = [room, callback]() {
                    room->onError = nullptr;
                    callback("", room);
                };

                room->connect(this->createConnection(processId + "/" + room->id + "?sessionId=" + room->sessionId));
            }
            else {
                callback(("Error " + std::to_string(response->getResponseCode()) + " in request"), nullptr);
            }
        });

        cocos2d::network::HttpClient::getInstance()->send(req);
        req->release();
    }

    Connection *createConnection(std::string path, JoinOptions options = JoinOptions())
    {
        return new Connection(this->endpoint + "/" + path);
    }

};

#endif /* Client_hpp */
