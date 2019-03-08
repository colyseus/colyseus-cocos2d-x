#ifndef FossilDeltaSerializer_hpp
#define FossilDeltaSerializer_hpp

#include "Serializer.hpp"

class FossilDeltaSerializer : public Serializer 
{
public:
    FossilDeltaSerializer();
    virtual ~FossilDeltaSerializer();

    StateContainer state;

    virtual msgpack::object_handle* getState();
    virtual void setState(const char* bytes, int length);
    virtual void patch(const char* bytes, int length);
    virtual void teardown();
    virtual void handshake(const char* bytes, int offset);

protected:
    const char* _previousState;
    int _previousStateSize;
};

#endif /* FossilDeltaSerializer_hpp */
