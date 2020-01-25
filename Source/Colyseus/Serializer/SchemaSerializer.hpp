#ifndef SchemaSerializer_hpp
#define SchemaSerializer_hpp

#include "schema.h"
#include "Serializer.hpp"

template <typename S>
class SchemaSerializer : public Serializer<S>
{
public:
    SchemaSerializer() { state = new S(); }
    ~SchemaSerializer() {
        delete state;
        delete it;
    }

    colyseus::schema::Iterator *it = new colyseus::schema::Iterator();
    S* state;
    S* getState() { return state; };

    void setState(const char* bytes, int offset, int length) {
        it->offset = offset;
        ((colyseus::schema::Schema*)state)->decode(reinterpret_cast<unsigned const char *>(bytes), length, it);
    }

    void patch(const char* bytes, int offset, int length) {
        it->offset = offset;
        ((colyseus::schema::Schema*)state)->decode(reinterpret_cast<unsigned const char *>(bytes), length, it);
    }

    void handshake(const char* bytes, int offset) {
        // TODO: validate incoming schema with Reflection.
    }

    void teardown() {
    }
};

#endif /* SchemaSerializer_hpp */
