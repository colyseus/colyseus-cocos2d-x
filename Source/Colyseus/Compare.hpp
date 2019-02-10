#ifndef Compare_h
#define Compare_h

#include <sstream>
#include <iostream>
#include <stdio.h>
#include "msgpack.hpp"

class PatchObject
{
public:
    PatchObject(std::vector<std::string> path, std::string op, msgpack::object value);
    std::vector<std::string> path;
    std::string op; // "add" | "remove" | "replace"
    msgpack::object value;
    msgpack::object previousValue;
};

class Compare
{
public:
    static msgpack::object_handle *emptyState;

    static bool containsKey(msgpack::object_map map, msgpack::object_kv key);
    static std::vector<PatchObject> getPatchList(const msgpack::object tree1, const msgpack::object tree2);
    static void generate(
        const msgpack::object mirrorPacked,
        const msgpack::object objPacked, std::vector<PatchObject> *patches,
        std::vector<std::string> path
    );
};


#endif /* Compare_h */
