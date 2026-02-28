#include "MacroSoundPreset.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "ctagDataModelBase.hpp"

using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


MacroSoundPreset::MacroSoundPreset() {
    id = "";
    displayName = "";
    groupName = "";
    parameterValues.clear();
}

MacroSoundPreset::~MacroSoundPreset() {
}

bool MacroSoundPreset::DeserializeJSON(const rapidjson::Value &jsonelement) {
    // Implement deserialization logic here

    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroSoundPreset", "Not a JSON object");
        return false;
    }

     if (jsonelement.HasMember("id") && jsonelement["id"].IsString()) {
        id = jsonelement["id"].GetString();
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'id' field");
        return false;
    }

    if (jsonelement.HasMember("name") && jsonelement["name"].IsString()) {
        displayName = jsonelement["name"].GetString();
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'name' field");
        return false;
    }

    if (jsonelement.HasMember("group") && jsonelement["group"].IsString()) {
        groupName = jsonelement["group"].GetString();
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'group' field");
        return false;
    }

    if (jsonelement.HasMember("macro") && jsonelement["macro"].IsString()) {
        macroDeviceId = jsonelement["macro"].GetString();
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'macro' field");
        return false;
    }

    if (jsonelement.HasMember("values") && jsonelement["values"].IsArray()) {
        parameterValues.clear();
        for (auto &v : jsonelement["values"].GetArray()) {
            if (v.IsInt()) {
                parameterValues.push_back(v.GetInt());
            } else {
                ESP_LOGE("MacroSoundPreset", "Invalid 'values' array element");
                return false;
            }
        }
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'values' field");
        return false;
    }

    return true;
}

bool MacroSoundPreset::SerializeJSONInto(rapidjson::Document &doc) {
    // TODO: Just read from disk?

    doc.SetObject();

    Value s_id(kObjectType);
    s_id.SetString(id, doc.GetAllocator());

    doc.AddMember("id", s_id, doc.GetAllocator());
    doc.AddMember("name", Value(displayName.c_str(), doc.GetAllocator()), doc.GetAllocator());
    doc.AddMember("group", Value(groupName.c_str(), doc.GetAllocator()), doc.GetAllocator());
    doc.AddMember("macro", Value(macroDeviceId.c_str(), doc.GetAllocator()), doc.GetAllocator());

    Value paramValues(kArrayType);
    doc.AddMember("values", paramValues, doc.GetAllocator());
    for (auto &v : parameterValues) {
        Value val(kNumberType);
        val.SetInt(v);
        doc["values"].PushBack(val, doc.GetAllocator());
    }

    return true;
}
