#include "FossilDeltaSerializer.hpp"

// #define FOSSIL_ENABLE_DELTA_CKSUM_TEST 0 // enable checksum on patches
#include "../fossil/delta.c"

#include <string.h>
#include <sstream>
#include <iostream>

FossilDeltaSerializer::FossilDeltaSerializer () : Serializer()
{
    _previousState = NULL;
    _previousStateSize = 0;
}

FossilDeltaSerializer::~FossilDeltaSerializer()
{
    if (_previousState)
    {
        delete _previousState;
    }
}

void FossilDeltaSerializer::setState(const char* bytes, int length)
{
    msgpack::object_handle *state = new msgpack::object_handle();
    msgpack::unpack(*state, bytes, length);
    this->state.set(state);

    if (_previousState) {
        delete _previousState;
    }

    this->_previousState = bytes;
    this->_previousStateSize = length;
}

msgpack::object_handle* FossilDeltaSerializer::getState()
{
    return state.state;
}

void FossilDeltaSerializer::patch(const char* bytes, int length)
{
    int newStateSize = delta_output_size(bytes, length);
    char* temp = new char[newStateSize];

    _previousStateSize = delta_apply(_previousState, _previousStateSize, bytes, length, temp);

    if (_previousStateSize == -1) {
        std::cout << "FATAL ERROR: fossil/delta had an error!" << std::endl;
    }

    if (_previousState) {
        // TODO: free _previousState from memory.
        // delete [] _previousState;
    }
    _previousState = temp;

    msgpack::object_handle *newState = new msgpack::object_handle();
    msgpack::unpack(*newState, _previousState, _previousStateSize);

    this->state.set(newState);
}

void FossilDeltaSerializer::teardown()
{
}

void FossilDeltaSerializer::handshake(const char* bytes, int offset)
{
}
