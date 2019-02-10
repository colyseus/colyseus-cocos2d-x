#include <iostream>
#include <regex>
#include <string>
#include <algorithm>

#include "StateContainer.hpp"
#include "Compare.hpp"

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
    if (state) {
        delete state;
    }
}

StateContainer::StateContainer(msgpack::object_handle *state)
{
    matcherPlaceholders.insert(std::make_pair(":id", std::regex("^([a-zA-Z0-9\\-_]+)$")));
    matcherPlaceholders.insert(std::make_pair(":number", std::regex("^([0-9]+)$")));
    matcherPlaceholders.insert(std::make_pair(":string",std::regex("^(\\w+)$")));
    matcherPlaceholders.insert(std::make_pair(":axis",std::regex("^([xyz])$")));
    matcherPlaceholders.insert(std::make_pair(":*",std::regex("(.*)")));

    if (Compare::emptyState == nullptr)
    {
        // generate msgpack empty state only once
        Compare::emptyState = new msgpack::object_handle();

        std::stringstream emptyStateStream;
        msgpack::pack(emptyStateStream, std::map<std::string, std::string>());
        msgpack::unpack(*Compare::emptyState, emptyStateStream.str().data(), emptyStateStream.str().size());
    }

    this->state = Compare::emptyState;
    this->reset();
}

    
void StateContainer::set(msgpack::object_handle *newState)
{
    std::vector<PatchObject> patches = Compare::getPatchList(this->state->get(), newState->get());
    this->checkPatches(patches);

    if (this->state != Compare::emptyState) {
        delete this->state;
    }
    this->state = newState;
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
        std::vector<Listener<PatchAction>> listeners;
        listeners.push_back(listener);
        checkPatches(Compare::getPatchList(Compare::emptyState->get(), this->state->get()), listeners);
        listeners.clear();
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
    
void StateContainer::checkPatches(std::vector<PatchObject> patches, std::vector<Listener<PatchAction>> &_listeners)
{
    std::cout << "CHECK PATCHES!, listeners.size() => " << _listeners.size() << std::endl;

#ifdef COLYSEUS_DEBUG
    std::cout << "--------StateContainer::set (patches)---------" << std::endl;
    for(int i = 0 ; i < patches.size();i++)
    {
        std::cout<< patches[i].op << std::endl;
        for(int j = 0 ;  j < patches[i].path.size() ; j++)
            std::cout << patches[i].path[j]  << "/";
        std::cout << std::endl;
    }
    std::cout << "----------------------------------------------" << std::endl;
#endif

    for (int i = (int)patches.size() - 1; i >= 0; i--)
    {
        bool matched = false;
        for (int j = 0; j < _listeners.size(); j++)
        {
            Listener<PatchAction> listener = _listeners[j];
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

void StateContainer::checkPatches(std::vector<PatchObject> patches)
{
    this->checkPatches(patches, this->listeners);
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

        } else if (matches.size() >= 2) {
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
