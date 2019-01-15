#ifndef NetworkData_h
#define NetworkData_h

#include "msgpack.hpp"

class NetworkData : public cocos2d::Ref
{
public:
    NetworkData(msgpack::object * data): _data(data)
    {

    }
    ~NetworkData()
    {
        delete _data;
    }
    msgpack::object * _data;
};

#endif /* NetworkData_h */
