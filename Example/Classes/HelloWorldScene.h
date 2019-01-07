//
//  HelloWorldScene.hpp
//  CocosEngine
//
//  Created by Hoang Hung on 4/7/17.
//
//

#ifndef HelloWorldScene_hpp
#define HelloWorldScene_hpp

#include <stdio.h>

#include "Header.h"
#include "./GamePlay/Player.hpp"


class Planet;
class Satellite;
class Room;

class HelloWorldScene : public cocos2d::Scene
{
    enum Command {
        DEPART = 1,
        ATTACK,
        SEND_ATTACK_TO_ONOTHER,
    };

public:
    bool init();
    void onConnectToServer(int response);
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnd(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMove(cocos2d::Touch* touch, cocos2d::Event* event);

public:
    CREATE_FUNC(HelloWorldScene);
    virtual ~HelloWorldScene();
    CC_SYNTHESIZE(std::vector<Planet*>, _planets, Planets);
    CC_SYNTHESIZE(std::vector<Planet*>, _planetsSelected, PlanetsSelected);

    Planet* getPlanetByID(std::string id);
    void addPlayer(Player* player);
    void removePlayer(std::string playerID);
    void moveSatellites(Planet* src, Planet *des, bool isCollistion = true);


    PlayerType getPlayerWin();

private: //recieve message
    void regirsterServerListener();
    void onGameRoomInit(cocos2d::Ref* sender,cocos2d::Ref* args);
    void onAddPlayer(std::vector<std::string>,msgpack::object);
    void onPlayerRemoved(std::vector<std::string>,msgpack::object);
    void onUpdatePlanet(std::vector<std::string> matches ,msgpack::object value);
    void onRoomDataUpdate(cocos2d::Ref* room,cocos2d::Ref* data);
    void onRoomError(cocos2d::Ref* room,cocos2d::Ref* data);
    void onSatelliteUpdate(std::vector<std::string> matches ,msgpack::object value);
    void onPlanetChangeState(std::vector<std::string> matches ,msgpack::object value);


    Room* _gameRoom;
    std::map<std::string,Player*> _players;

private: //send command
    void depart(Planet* src, Planet *des);
    void attack(Planet* src, Planet *des,int nSatellite);

private:
    static HelloWorldScene * _instance;
    static HelloWorldScene* getInstance();
    friend class Planet;
    friend class Satellite;


};
#endif /* HelloWorldScene_hpp */
