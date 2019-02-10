/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

#define COLYSEUS_DEBUG 1
#include "Colyseus/Client.hpp"

USING_NS_CC;

Client* colyseus = new Client("ws://colyseus-examples.herokuapp.com");
Room* room;

Map<std::string, Sprite*> players;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    colyseus->onOpen = CC_CALLBACK_0(HelloWorld::onConnectToServer, this);
    colyseus->connect();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);
    return true;
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void HelloWorld::onConnectToServer()
{
    log("Colyseus: CONNECTED TO SERVER!");
    room = colyseus->join("state_handler", std::map<std::string, std::string>());
    room->onMessage = CC_CALLBACK_2(HelloWorld::onRoomMessage, this);
    room->onStateChange = CC_CALLBACK_1(HelloWorld::onRoomStateChange, this);

    room->listen("players/:id", [this](std::map<std::string, std::string> path, PatchObject patch) -> void {
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "CALLBACK FOR 'players/:id' >>" << std::endl;
        std::cout << "OPERATION: " << patch.op << std::endl;
        std::cout << "PLAYER ID:" << path.at(":id") << std::endl;
        std::cout << "VALUE: " << patch.value << std::endl;

        std::string sessionId = path.at(":id");

        if (patch.op == "add") {
            // convert patch.value to specified type
            std::map<std::string, float> value;
            patch.value.convert(value);

            // add player sprite
            auto sprite = Sprite::create("HelloWorld.png");
            sprite->setPosition(Vec2(value.at("x"), value.at("y")));
            players.insert(sessionId, sprite);
            this->addChild(sprite, 0);

        } else if (patch.op == "remove") {
            auto sprite = players.at(sessionId);
            this->removeChild(sprite);
            players.erase(sessionId);
        }
    });

    // listen for player position changes
    room->listen("players/:id/:axis", [this](std::map<std::string, std::string> path, PatchObject patch) -> void {
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "CALLBACK FOR 'players/:id/:axis' >>" << std::endl;
        std::cout << "OPERATION: " << patch.op << std::endl;

        std::string convrt = "";
        std::string result = "";
        for (auto it = path.cbegin(); it != path.cend(); it++)
        {
            convrt = it->second;
            result += (it->first) + " = " + (convrt) + ", ";
        }
        std::cout << "PATH =>" << result << std::endl;

        std::cout << "VALUE: " << patch.value << std::endl;

        if (patch.op == "replace") {
            std::string sessionId = path.at(":id");

            auto sprite = players.at(sessionId);
            if (path.at(":axis") == "x") {
                sprite->setPositionX(patch.value.via.f64);
            } else {
                sprite->setPositionY(patch.value.via.f64);
            }
        }
    });
}

void HelloWorld::onRoomMessage(Room* sender, msgpack::object message)
{
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "HelloWorld::onRoomMessage" << std::endl;
    std::cout << message << std::endl;
}

void HelloWorld::onRoomStateChange(Room* sender)
{
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "HelloWorld::onRoomStateChange" << std::endl;
    std::cout << sender->state->get() << std::endl;

    // send command to move x
    auto data = std::map<std::string, float>();
    data.insert(std::make_pair("x", 0.1));
    room->send(data);
}
