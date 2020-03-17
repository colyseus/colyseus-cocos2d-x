#include <iostream>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#include "Auth.hpp"
#include "network/HttpRequest.h"

using namespace cocos2d;
using namespace cocos2d::network;

Auth::Auth(std::string endpoint)
{
    this->endpoint = endpoint.replace(0, 2, "http");
    this->token = this->readToken();
}

Auth::~Auth()
{
}

void Auth::login()
{
    std::vector<std::string> query;
    return login(query);
}

void Auth::login(std::string email, std::string password)
{
    std::vector<std::string> query;
    query.push_back("email=" + this->urlencode(email));
    query.push_back("password=" + this->urlencode(password));
    return login(query);
}

void Auth::login(std::string fbAccessToken)
{
    std::vector<std::string> query;
    query.push_back("accessToken=" + this->urlencode(fbAccessToken));
    return login(query);
}

void Auth::login(std::vector<std::string> query)
{
    request(HttpRequest::Type::POST, "/auth", query, "");
}

void Auth::save()
{
    // TODO: save
    std::vector<std::string> query;
    // request("PUT", "/auth", query, )
}

void Auth::request(HttpRequest::Type method, std::string segments, std::vector<std::string> query)
{
    request(method, segments, query, "");
}

void Auth::request(HttpRequest::Type method, std::string segments, std::vector<std::string> query, std::string upload)
{
    std::string deviceId = this->getDeviceId();
    if (deviceId != "")
    {
        query.push_back("deviceId=" + this->urlencode(this->getDeviceId()));
        query.push_back("platform=" + this->urlencode(this->getPlatform()));
    }

    HttpRequest *req = new (std ::nothrow) HttpRequest();
    req->setUrl(this->endpoint + segments + "?" + this->string_join(query, "&"));
    req->setRequestType(method);

    if (upload != "") {
        req->setRequestData(upload.c_str(), upload.length());
    }

    req->setResponseCallback(CC_CALLBACK_2(Auth::onHttpRequestCompleted, this));

    HttpClient::getInstance()->send(req);

    req->release();
}

void Auth::onHttpRequestCompleted(HttpClient * client, HttpResponse * response)
{
    // TODO: implementation needed
}

std::string Auth::getPlatform()
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    return "mac";
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    return "ios";
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    return "android";
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
    return "windows";
#elif CC_TARGET_PLATFORM == CC_PLATFORM_LINUX
    return "linux";
#endif
}

std::string Auth::getDeviceId()
{
    // TODO: get unique device id.

    // TODO: for Android

    // TODO: for iOS

    return "";
}

std::string Auth::readToken()
{
    // TODO: read auth token from file
    return "";
}

void Auth::writeToken(std::string token)
{
    // TODO: write auth token to file
    // https://github.com/cocos2d/cocos2d-x/issues/18549
}

std::string Auth::urlencode(const std::string &s)
{
    //RFC 3986 section 2.3 Unreserved Characters (January 2005)
    const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

    std::string escaped = "";
    for (size_t i = 0; i < s.length(); i++)
    {
        if (unreserved.find_first_of(s[i]) != std::string::npos)
        {
            escaped.push_back(s[i]);
        }
        else
        {
            escaped.append("%");
            char buf[3];
            sprintf(buf, "%.2X", s[i]);
            escaped.append(buf);
        }
    }
    return escaped;
}

std::string Auth::string_join(const std::vector<std::string> &elements, const std::string &separator)
{
    if (!elements.empty())
    {
        std::stringstream ss;
        auto it = elements.cbegin();
        while (true)
        {
            ss << *it++;
            if (it != elements.cend())
                ss << separator;
            else
                return ss.str();
        }
    }
    return "";
}

