#include <iostream>
#include <regex>
#include <string>
#include <algorithm>

#include "StateContainer.hpp"

int listenerId = 0;

std::vector<std::string> StateContainer::splitStr(const std::string &sourceStr, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(sourceStr);
    std::string unit;
    while (std::getline(ss, unit, delim))
    {
        elems.push_back(unit);
    }
    return elems;
}

StateContainer::~StateContainer()
{
    if (data) {
        delete data;
    }
}

StateContainer::StateContainer(msgpack::object_handle *data)
{
    matcherPlaceholders.insert(std::make_pair(":id", std::regex("^([a-zA-Z0-9\\-_]+)$")));
    matcherPlaceholders.insert(std::make_pair(":number", std::regex("^([0-9]+)$")));
    matcherPlaceholders.insert(std::make_pair(":string",std::regex("^(\\w+)$")));
    matcherPlaceholders.insert(std::make_pair(":axis",std::regex("^([xyz])$")));
    matcherPlaceholders.insert(std::make_pair(":*",std::regex("(.*)")));
    
    this->data = data;
    this->reset();
}

    
std::vector<PatchObject> StateContainer::set(msgpack::object newData)
{
    //        std::cout << "-------------------------set---------------------------------" << std::endl;
    std::vector<PatchObject> patches;
    if(this->data)
    {
        //            std::cout << "OldState = "  << this->data->get()  << std::endl;
        //            std::cout << "newState = "  << newData  << std::endl;
        patches = Compare::getPatchList(this->data->get(), newData);
        this->checkPatches(patches);
    }

#ifdef COLYSEUS_DEBUG
    std::cout << "------------patches.size(----------------------" << std::endl;
    for(int i = 0 ; i < patches.size();i++)
    {
        std::cout<< patches[i].op << std::endl;
        for(int j = 0 ;  j < patches[i].path.size() ; j++)
            std::cout << patches[i].path[j]  << "/";
        std::cout << std::endl;
    }
    std::cout << "----------------------------------------------" << std::endl;
#endif

    msgpack::sbuffer buffer;
    msgpack::packer<msgpack::sbuffer> pk(&buffer);
    pk.pack(newData);
    msgpack::object_handle *oh = new msgpack::object_handle();
    msgpack::unpack(*oh,buffer.data(), buffer.size());
    
    if(data)
        delete data;
    
    this->data = oh;;
    buffer.release();
    //        std::cout << "-----------------------------------------------------------" << std::endl;
    
    return patches;
}
    
void StateContainer::registerPlaceholder(std::string placeholder, std::regex matcher)
{
    std::pair<std::string,std::regex> pair(placeholder,matcher);
    this->matcherPlaceholders.insert(pair);
}
    
Listener<FallbackAction> StateContainer::listen(FallbackAction callback)
{
    Listener<FallbackAction> listener;
    listener.id = ++listenerId;
    listener.callback = callback;
    listener.rules = std::vector<std::regex>();
    this->fallbackListeners.push_back(listener);
    return listener;
}

Listener<PatchAction> StateContainer::listen(std::string segments, PatchAction callback, bool immediate)
{
    Listener<PatchAction> listener;
    
    listener.id = ++listenerId;
    listener.callback = callback;
    listener.segments = splitStr(segments, '/');
    listener.rules = parseRegexRules(listener.segments);
    listeners.push_back(listener);
    
    if (immediate) {
        checkPatches(Compare::getPatchList(msgpack::object(), this->data->get()));
    }

    return listener;
}

void StateContainer::removeListener(Listener<PatchAction> &listener)
{
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}
    
void StateContainer::removeAllListeners()
{
    reset();
}

std::vector<std::regex> StateContainer::parseRegexRules (std::vector<std::string> rules)
{
    std::vector<std::regex> regexpRules;
    for (int i = 0; i < rules.size(); i++)
    {
        auto segment = rules[i];
        if (segment.find(':') == 0)
        {
            auto it = matcherPlaceholders.find(segment);
            if (it != matcherPlaceholders.end()) {
                regexpRules.push_back(it->second);
            } else {
                auto it2 = matcherPlaceholders.find("*");
                if (it2 != matcherPlaceholders.end()) {
                    regexpRules.push_back(it2->second);
                }
            }
            
        } else {
            regexpRules.push_back(std::regex(segment));
        }
    }
    return regexpRules;
}
    

void StateContainer::checkPatches(std::vector<PatchObject> patches)
{
    for (int i = (int)patches.size() - 1; i >= 0; i--)
    {
        bool matched = false;
        for (int j = 0; j < listeners.size(); j++)
        {
            Listener<PatchAction> listener = listeners[j];
            auto matches = checkPatch(patches[i], listener);
            if (matches.size() > 0) {
                listener.callback(matches, patches[i]);
                matched = true;
            }
        }
        
        // check for fallback listener
        int fallbackListenersCount = (int) fallbackListeners.size();
        if (!matched && fallbackListenersCount > 0) {
            for (int j = 0; j < fallbackListenersCount; j++)
            {
                fallbackListeners[j].callback(patches[i]);
            }
        }
    }
    
}

std::map<std::string, std::string> StateContainer::checkPatch(PatchObject patch, Listener<PatchAction> listener)
{
    // skip if rules count differ from patch
    if (patch.path.size() != listener.rules.size()) {
        return std::map<std::string, std::string>();
    }
    std::map<std::string, std::string> pathVars;
    
    for (int i = 0; i < listener.rules.size(); i++)
    {
        std::cmatch matches;    // same as std::match_results<const char*> cm;
        std::regex_match(patch.path[i].c_str(), matches, listener.rules[i]);
        if (matches.size() == 0 || matches.size() > 2) {
            return std::map<std::string, std::string>();

        } else if (matches[0].length() > 1) {
            pathVars.insert(std::make_pair(listener.segments[i], matches[0].str()));
        }
    }
    
    return pathVars;
}

void StateContainer::reset()
{
    listeners.clear();
    fallbackListeners.clear();
}
