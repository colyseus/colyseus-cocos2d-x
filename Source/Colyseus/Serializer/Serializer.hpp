#ifndef Serializer_hpp
#define Serializer_hpp

#include "StateContainer.hpp"

class Serializer
{
public:
//    Serializer();
//    virtual ~Serializer();

    virtual msgpack::object_handle* getState() = 0;
    virtual void setState(const char* bytes, int length) = 0;
    virtual void patch(const char* bytes, int length) = 0;
    virtual void teardown() = 0;
    virtual void handshake(const char* bytes, int offset) = 0;
};

#endif /* Serializer_hpp */
