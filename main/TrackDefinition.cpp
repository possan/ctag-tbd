#include "TrackDefinition.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


TrackDefinition::TrackDefinition() {
    index = 0;
    name = "";
    midiChannel = 0;
    drumNote = 0;
    baseCC = 0;
    macroMachineIds.clear();
}

TrackDefinition::~TrackDefinition() {
}

void TrackDefinition::GetDefinitionJson(std::string *target) {
    Document d;
    target->assign("{}");
}

bool TrackDefinition::DeserializeJSON(const Value &jsonelement) {
    if (!jsonelement.HasMember("index")) return false;
    if (!jsonelement.HasMember("name")) return false;
    if (!jsonelement.HasMember("midichannel")) return false;
    if (!jsonelement.HasMember("drumnote")) return false;
    if (!jsonelement.HasMember("basecc")) return false;

    index = jsonelement["index"].GetInt();
    name = jsonelement["name"].GetString();
    midiChannel = jsonelement["midichannel"].GetInt();
    drumNote = jsonelement["drumnote"].GetInt();
    baseCC = jsonelement["basecc"].GetInt();

    macroMachineIds.clear();
    if (jsonelement.HasMember("machines") && jsonelement["machines"].IsArray()) {
        for (auto &v : jsonelement["machines"].GetArray()) {
            macroMachineIds.push_back(v.GetString());
        }
    }

    return true;
}

bool TrackDefinition::SerializeJSONInto(const rapidjson::Value &jsonelement, rapidjson::Document::AllocatorType &allocator) {
    // Implementation goes here
    // parentjsonelement
    return true;
}

