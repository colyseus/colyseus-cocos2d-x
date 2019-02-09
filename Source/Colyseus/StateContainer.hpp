#ifndef StateContainer_h
#define StateContainer_h

#include <iostream>
#include <regex>
#include <string>

#include "Compare.hpp"

typedef std::function<void(std::map<std::string, std::string>, PatchObject)> PatchAction;
typedef std::function<void(PatchObject)> FallbackAction;

template <typename T>
class Listener
{
public:
    T callback;
    std::vector<std::regex> rules;
    std::vector<std::string> segments;
//    std::string operation;
    int id;
    bool operator== (Listener<T> const& other) { return this->id == other.id; }
};

class StateContainer
{
public:
    static std::vector<std::string> splitStr(const std::string &sourceStr, char delim);

public:
    StateContainer(msgpack::object_handle *data = nullptr);
    virtual ~StateContainer();
    
    msgpack::object_handle *data;
    std::vector<Listener<PatchAction>> listeners;
    std::vector<Listener<FallbackAction>> fallbackListeners;
    std::map<std::string,std::regex> matcherPlaceholders;

    std::vector<PatchObject> set(msgpack::object newData);
    void registerPlaceholder(std::string placeholder, std::regex matcher);

    Listener<FallbackAction> listen(FallbackAction callback);
    Listener<PatchAction> listen(std::string segments, PatchAction callback, bool immediate=false);
    void removeListener(Listener<PatchAction> &listener);
    void removeAllListeners();
    
protected:
    std::vector<std::regex> parseRegexRules (std::vector<std::string> rules);

    void checkPatches(std::vector<PatchObject> patches);
    void checkPatches(std::vector<PatchObject> patches, std::vector<Listener<PatchAction>> &);
    std::map<std::string, std::string> checkPatch(PatchObject patch, Listener<PatchAction> listener);

    void reset();

};

#endif /* StateContainer_h */
