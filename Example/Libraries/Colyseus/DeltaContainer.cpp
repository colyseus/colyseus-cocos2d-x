#include <iostream>
#include <regex>
#include <string>

#include "DeltaContainer.hpp"

std::vector<std::string> DeltaContainer::splitStr(const std::string &sourceStr, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(sourceStr);
    std::string unit;
    while (std::getline(ss, unit, delim))
    {
        elems.push_back(unit);
    }
    return elems;
}

DeltaContainer::~DeltaContainer()
{
    if(data)
        delete data;
}

DeltaContainer::DeltaContainer(msgpack::object_handle *data)
{
    matcherPlaceholders.insert(std::make_pair(":id", std::regex("^([a-zA-Z0-9\\-_]+)$")));
    matcherPlaceholders.insert(std::make_pair(":number", std::regex("^([0-9]+)$")));
    matcherPlaceholders.insert(std::make_pair(":string",std::regex("^(\\w+)$")));
    matcherPlaceholders.insert(std::make_pair(":axis",std::regex("^([xyz])$")));
    matcherPlaceholders.insert(std::make_pair(":*",std::regex("(.*)")));
    
    this->data = data;
    this->reset();
}

    
std::vector<PatchObject> DeltaContainer::set(msgpack::object newData)
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
    std::cout << "------------patches.size(----------------------" << std::endl;
    for(int i = 0 ; i < patches.size();i++)
    {
        std::cout<< patches[i].op << std::endl;
        for(int j = 0 ;  j < patches[i].path.size() ; j++)
            std::cout << patches[i].path[j]  << "/";
        std::cout << std::endl;
    }
    std::cout << "----------------------------------------------" << std::endl;
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
    
void DeltaContainer::registerPlaceholder(std::string placeholder, std::regex matcher)
{
    std::pair<std::string,std::regex> pair(placeholder,matcher);
    this->matcherPlaceholders.insert(pair);
}
    
Listener<FallbackAction> DeltaContainer::listen(FallbackAction callback)
{
    Listener<FallbackAction> listener;
    listener.callback = callback;
    listener.operation = "";
    listener.rules = std::vector<std::regex>();
    this->fallbackListeners.push_back(listener);
    return listener;
}

Listener<PatchAction> DeltaContainer::listen(std::string segments, std::string operation, PatchAction callback)
{
    std::vector<std::string> words = splitStr(segments, '/');
    std::vector<std::regex> regexpRules = parseRegexRules(words);
    Listener<PatchAction> listener;
    listener.callback = callback;
    listener.operation = operation;
    listener.rules = regexpRules;
    
    auto item = listeners.find(operation);
    if(item == listeners.end()) {
        std::vector<Listener<PatchAction>> list;
        list.push_back(listener);
        std::pair<std::string, std::vector<Listener<PatchAction>>> pair(operation,list);
        listeners.insert(pair);
    } else {
        item->second.push_back(listener);
    }
    return listener;
}

void DeltaContainer::removeListener(Listener<PatchAction> listener)
{
    auto it = listeners.find(listener.operation);
    if (it != listeners.end())
    {
        auto patchActionList = it->second;
        for (auto it2 = patchActionList.begin(); it2 != patchActionList.end() ;it2++)
        {
            if (it2->operation == listener.operation) {
                it2 = patchActionList.erase(it2);
            }
        }
    }
    
}
    
void DeltaContainer::removeAllListeners()
{
    reset();
}
    

std::vector<std::regex> DeltaContainer::parseRegexRules (std::vector<std::string> rules)
{
    std::vector<std::regex>regexpRules;
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
    

void DeltaContainer::checkPatches(std::vector<PatchObject> patches)
{
    for (int i = (int)patches.size() - 1; i >= 0; i--)
    {
        bool matched = false;
        auto op = patches[i].op;
        auto it = listeners.find(op);
        if (it != listeners.end()) {
            std::vector<Listener<PatchAction>> list = it->second;
            for (int j = 0; j < list.size(); j++)
            {
                Listener<PatchAction> listener = list[j];
                auto matches = checkPatch(patches[i], listener);
                if (matches.size() > 0) {
                    listener.callback(matches, patches[i].value);
                    matched = true;
                }
            }
            
            // check for fallback listener
            int fallbackListenersCount = (int) fallbackListeners.size();
            if (!matched && fallbackListenersCount > 0) {
                for (int j = 0; j < fallbackListenersCount; j++)
                {
                    fallbackListeners [j].callback(patches[i].path, patches [i].op, patches [i].value);
                }
            }
        }
    }
    
}

std::vector<std::string> DeltaContainer::checkPatch(PatchObject patch, Listener<PatchAction> listener)
{
    // skip if rules count differ from patch
    if (patch.path.size() != listener.rules.size()) {
        return std::vector<std::string>();
    }
    std::vector<std::string> pathVars;
    
    for (int i = 0; i < listener.rules.size(); i++)
    {
        std::cmatch matches;    // same as std::match_results<const char*> cm;
        std::regex_match(patch.path[i].c_str(),matches,listener.rules[i]);
        if (matches.size() == 0 || matches.size() > 2)
        {
            return std::vector<std::string>();
        } else if ( matches[0].length() > 1 )
        {
            pathVars.push_back(matches[0].str());
        }
    }
    
    return pathVars;
}

void DeltaContainer::reset()
{
    listeners.clear();
    fallbackListeners.clear();
}
