#ifndef DeltaContainer_h
#define DeltaContainer_h

#include <iostream>
#include <regex>
#include <string>

#include "Compare.hpp"

typedef std::function<void(std::vector<std::string>,msgpack::object)> PatchAction;
typedef std::function<void(std::vector<std::string>,std::string,msgpack::object)> FallbackAction;

template <typename T>
class Listener
{
public:
    T callback;
    std::string operation;
    std::vector<std::regex> rules;
};

class DeltaContainer
{
public:
    static std::vector<std::string> splitStr(const std::string &sourceStr, char delim);

public:
    DeltaContainer(msgpack::object_handle *data = nullptr);
    virtual ~DeltaContainer();
    
    msgpack::object_handle *data;
    std::map<std::string, std::vector<Listener<PatchAction>>> listeners;
    std::vector<Listener<FallbackAction>> fallbackListeners;
    std::map<std::string,std::regex> matcherPlaceholders;

    std::vector<PatchObject> set(msgpack::object newData);
    void registerPlaceholder(std::string placeholder, std::regex matcher);

    Listener<FallbackAction> listen(FallbackAction callback);
    Listener<PatchAction> listen(std::string segments, std::string operation, PatchAction callback);
    void removeListener(Listener<PatchAction> listener);
    void removeAllListeners();
    
protected:
    std::vector<std::regex> parseRegexRules (std::vector<std::string> rules);
    void checkPatches(std::vector<PatchObject> patches);
    std::vector<std::string> checkPatch(PatchObject patch, Listener<PatchAction> listener);
    void reset();

};

#endif /* DeltaContainer_h */
