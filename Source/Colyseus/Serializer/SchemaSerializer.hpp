#ifndef SchemaSerializer_hpp
#define SchemaSerializer_hpp

#include "schema.hpp"
#include "Serializer.hpp"

template <typename S>
class SchemaSerializer : public Serializer<S>
{
public:
    SchemaSerializer() { state = new S(); }
    ~SchemaSerializer() { delete state; }

    S* state;
    S* getState() { return state; };

    void setState(const char* bytes, int length) {
        ((colyseus::schema::Schema*)state)->decode(bytes, length);
    }

    void patch(const char* bytes, int length) {
        ((colyseus::schema::Schema*)state)->decode(bytes, length);
    }

    void handshake(const char* bytes, int offset) {
        // TODO: validate incoming schema with Reflection.
    }

    void teardown() {
    }

protected:
};

#endif /* SchemaSerializer_hpp */
