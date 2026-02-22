#include "SynthDefinition.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


SynthParameter::SynthParameter() {
    id = "";
    name = "";
    type = SynthParameterType_None;
    defaultValue = 0;
    cc = 0;
}

SynthParameter::~SynthParameter() {
}






SynthDefinition::SynthDefinition() {
    id = "";
    name = "";
    type = SynthType_None;
    parameters.clear();
}

SynthDefinition::~SynthDefinition() {
}

void SynthDefinition::GetDefinitionJson(std::string *target) {
    Document d;
    target->assign("{}");
}

bool SynthDefinition::DeserializeJSON(const Value &jsonelement) {
    if (!jsonelement.HasMember("id")) return false;
    if (!jsonelement.HasMember("name")) return false;
    if (!jsonelement.HasMember("type")) return false;

    id = jsonelement["id"].GetString();
    name = jsonelement["name"].GetString();
    type = static_cast<SynthType>(jsonelement["type"].GetInt());

    parameters.clear();
    if (jsonelement.HasMember("parameters") && jsonelement["parameters"].IsArray()) {
        for (auto &v : jsonelement["parameters"].GetArray()) {
            SynthParameter *p = new SynthParameter();

            if (!v.HasMember("id")) return false;
            if (!v.HasMember("name")) return false;
            if (!v.HasMember("type")) return false;
            if (!v.HasMember("default")) return false;
            if (!v.HasMember("cc")) return false;

            p->id = v["id"].GetString();
            p->name = v["name"].GetString();
            if (v["type"].GetString() == std::string("cc")) {
                p->type = SynthParameterType_CC;
            } else if (v["type"].GetString() == std::string("nrpm")) {
                p->type = SynthParameterType_NRPM;
            } else {
                p->type = SynthParameterType_None;
            }
            p->defaultValue = v["default"].GetUint();
            p->cc = v["cc"].GetUint();

            parameters.push_back(p);
        }
    }

    return true;
}

// bool SynthDefinition::SerializeJSONInto(const rapidjson::Value &jsonelement, rapidjson::Document::AllocatorType &allocator) {
//     // Implementation goes here

//     // Value obj(kObjectType);
//     // Value id(jsonelement["id"].GetString(), allocator);
//     // Value name(jsonelement["name"].GetString(), allocator);
//     // Value hint(kStringType);

//     // jsonelement.AddMember("id", id.Move(), allocator);
//     // jsonelement.AddMember("name", name.Move(), allocator);
//     // // jsonelement.PushBack(obj.Move(), allocator);

//     return true;
// }
