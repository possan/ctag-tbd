#include "MacroDeviceDefinition.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;

MacroDeviceParameter::MacroDeviceParameter() {
    index = 0;
    name = "";
    defaultValue = 0;
    minValue = 0;
    maxValue = 0;
    resolution = 0;
    uiType = "";
}

MacroDeviceParameter::~MacroDeviceParameter() {}

bool MacroDeviceParameter:: DeserializeJSON(const rapidjson::Value &jsonelement) {
    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceParameter", "Not a JSON object");
        return false;
    }

    if (jsonelement.HasMember("idx") && jsonelement["idx"].IsUint()) {
        index = jsonelement["idx"].GetUint();
    } else {
        ESP_LOGE("MacroDeviceParameter", "Missing or invalid 'idx' field");
        return false;
    }

    if (jsonelement.HasMember("name") && jsonelement["name"].IsString()) {
        name = jsonelement["name"].GetString();
    } else {
        ESP_LOGE("MacroDeviceParameter", "Missing or invalid 'name' field");
        return false;
    }

    if (jsonelement.HasMember("def") && jsonelement["def"].IsInt()) {
        defaultValue = jsonelement["def"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceParameter", "Missing or invalid 'def' field"); 
        return false;
    }

    if (jsonelement.HasMember("min") && jsonelement["min"].IsInt()) {
        minValue = jsonelement["min"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceParameter", "Missing or invalid 'min' field");
        return false;
    }

    if (jsonelement.HasMember("max") && jsonelement["max"].IsInt()) {
        maxValue = jsonelement["max"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceParameter", "Missing or invalid 'max' field");
        return false;
    }

    if (jsonelement.HasMember("res") && jsonelement["res"].IsInt()) {
        resolution = jsonelement["res"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceParameter", "Missing or invalid 'res' field");
        return false;
    }

    if (jsonelement.HasMember("ui") && jsonelement["ui"].IsString()) {
        uiType = jsonelement["ui"].GetString();
    } else {
        ESP_LOGE("MacroDeviceParameter", "Missing or invalid 'ui' field");
        return false;
    }

    return true;
}

bool MacroDeviceParameter::SerializeJSONInto(rapidjson::Document &doc) {
    Value defval(kNumberType);
    defval.SetInt(defaultValue);

    Value minval(kNumberType);
    minval.SetInt(minValue);

    Value maxval(kNumberType);
    maxval.SetInt(maxValue);

    Value resval(kNumberType);
    resval.SetInt(resolution);

    doc.SetObject();

    doc.AddMember("idx", index, doc.GetAllocator());
    doc.AddMember("name", Value(name.c_str(), doc.GetAllocator()), doc.GetAllocator());
    doc.AddMember("def", defval, doc.GetAllocator());
    // doc.AddMember("def", defaultValue, doc.GetAllocator());
    doc.AddMember("min", minval, doc.GetAllocator());
    doc.AddMember("max", maxval, doc.GetAllocator());
    doc.AddMember("res", resval, doc.GetAllocator());
    doc.AddMember("ui", Value(uiType.c_str(), doc.GetAllocator()), doc.GetAllocator());

    return true;
}

MacroDeviceParameterGroup::MacroDeviceParameterGroup() {
    name = "";
    parameters.clear();
}

MacroDeviceParameterGroup::~MacroDeviceParameterGroup() {}

bool MacroDeviceParameterGroup::DeserializeJSON(const rapidjson::Value &jsonelement) {
    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceParameterGroup", "Not a JSON object");
        return false;
    }

    if (jsonelement.HasMember("name") && jsonelement["name"].IsString()) {
        name = jsonelement["name"].GetString();
    } else {
        ESP_LOGE("MacroDeviceParameterGroup", "Missing or invalid 'name' field");
        return false;
    }

    if (jsonelement.HasMember("parameters") && jsonelement["parameters"].IsArray()) {
        for (auto &v : jsonelement["parameters"].GetArray()) {
            MacroDeviceParameter *p = new MacroDeviceParameter();
            if (p->DeserializeJSON(v)) {
                parameters.push_back(p);
            } else {
                ESP_LOGE("MacroDeviceParameterGroup", "Failed to deserialize a parameter in 'parameters' array");
                delete p;
            }
        }
    } else {
        ESP_LOGE("MacroDeviceParameterGroup", "Missing or invalid 'parameters' field");
        return false;
    }

    return true;
}

bool MacroDeviceParameterGroup::SerializeJSONInto(rapidjson::Document &doc) {
    doc.SetObject();
    doc.AddMember("name", Value(name.c_str(), doc.GetAllocator()), doc.GetAllocator());

    Value params(kArrayType);
    doc.AddMember("parameters", params, doc.GetAllocator());
    for (auto &p : parameters) {
        Document d2;
        if (p->SerializeJSONInto(d2)) {
            Value copyOfd2(d2, doc.GetAllocator());
            doc["parameters"].PushBack(copyOfd2.Move(), doc.GetAllocator());
        } else {
            ESP_LOGE("MacroDeviceParameterGroup", "Failed to serialize a parameter in 'params' array");
        }
    }

    return true;
}


MacroDeviceOutputMappingSource::MacroDeviceOutputMappingSource() {
    parameterIndex = 0;
    amount = 0;
}

MacroDeviceOutputMappingSource::~MacroDeviceOutputMappingSource() {}

bool MacroDeviceOutputMappingSource:: DeserializeJSON(const rapidjson::Value &jsonelement) {

    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Not a JSON object");
        return false;
    }

    if (jsonelement.HasMember("src") && jsonelement["src"].IsUint()) {
        parameterIndex = jsonelement["src"].GetUint();
    } else {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Missing or invalid 'src' field");
        return false;
    }

    if (jsonelement.HasMember("amt") && jsonelement["amt"].IsInt()) {
        amount = jsonelement["amt"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Missing or invalid 'amt' field");
        return false;
    }

    return true;
}

bool MacroDeviceOutputMappingSource:: SerializeJSONInto(rapidjson::Document &doc) {
    doc.SetObject();

    Value srcval(kNumberType);
    srcval.SetInt(parameterIndex);

    Value amtval(kNumberType);
    amtval.SetInt(amount);

    doc.AddMember("src", srcval, doc.GetAllocator());
    doc.AddMember("amt", amtval, doc.GetAllocator());

    return true;
}


MacroDeviceOutputMapping::MacroDeviceOutputMapping() {
    synthParameterId = "";
    startValue = 0;
    sources.clear();
}

MacroDeviceOutputMapping::~MacroDeviceOutputMapping() {}

bool MacroDeviceOutputMapping::DeserializeJSON(const rapidjson::Value &jsonelement) {

    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceOutputMapping", "Not a JSON object");
        return false;
    }

    if (jsonelement.HasMember("tgt") && jsonelement["tgt"].IsString()) {
        synthParameterId = jsonelement["tgt"].GetString();
    } else {
        ESP_LOGE("MacroDeviceOutputMapping", "Missing or invalid 'tgt' field");
        return false;
    }

    if (jsonelement.HasMember("start") && jsonelement["start"].IsInt()) {
        startValue = jsonelement["start"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMapping", "Missing or invalid 'start' field");
        return false;
    }

    if (jsonelement.HasMember("add") && jsonelement["add"].IsArray()) {
        for (auto &v : jsonelement["add"].GetArray()) {
            MacroDeviceOutputMappingSource *s = new MacroDeviceOutputMappingSource();
            if (s->DeserializeJSON(v)) {
                sources.push_back(s);
            } else {
                ESP_LOGE("MacroDeviceOutputMapping", "Failed to deserialize a source in 'add' array");
                delete s;
            }
        }
    }

    return true;
}

bool MacroDeviceOutputMapping::SerializeJSONInto(rapidjson::Document &doc) {
    doc.SetObject();

    doc.AddMember("tgt", Value(synthParameterId.c_str(), doc.GetAllocator()), doc.GetAllocator());

    Value startval(kNumberType);
    startval.SetInt(this->startValue);
    doc.AddMember("start", startval, doc.GetAllocator());

    Value add(kArrayType);
    doc.AddMember("add", add, doc.GetAllocator());
    for (auto &s : sources) {
        Document d2;
        if (s->SerializeJSONInto(d2)) {
            Value copyOfd2(d2, doc.GetAllocator());
            doc["add"].PushBack(copyOfd2.Move(), doc.GetAllocator());
        } else {
            ESP_LOGE("MacroDeviceOutputMapping", "Failed to serialize a source in 'add' array");
        }
    }

    return true;
}


MacroDeviceDefinition::MacroDeviceDefinition() {
    id = "";
    name = "";
    synthId = "";
    parameterGroups.clear();
    outputMappings.clear();
}

MacroDeviceDefinition::~MacroDeviceDefinition() {
}

bool MacroDeviceDefinition::DeserializeJSON(const rapidjson::Value &jsonelement) {
    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceDefinition", "Not a JSON object");
        return false;
    }

     if (jsonelement.HasMember("id") && jsonelement["id"].IsString()) {
        id = jsonelement["id"].GetString();
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'id' field");
        return false;
    }

    if (jsonelement.HasMember("name") && jsonelement["name"].IsString()) {
        name = jsonelement["name"].GetString();
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'name' field");
        return false;
    }

    if (jsonelement.HasMember("machine") && jsonelement["machine"].IsString()) {
        synthId = jsonelement["machine"].GetString();
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'machine' field");
        return false;
    }

    if (jsonelement.HasMember("groups") && jsonelement["groups"].IsArray()) {
        for (auto &v : jsonelement["groups"].GetArray()) {
            MacroDeviceParameterGroup *g = new MacroDeviceParameterGroup();
            if (g->DeserializeJSON(v)) {
                parameterGroups.push_back(g);
            } else {
                ESP_LOGE("MacroDeviceDefinition", "Failed to deserialize a parameter group in 'groups' array");
                delete g;
            }
        }
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'groups' field");
        return false;
    }

    if (jsonelement.HasMember("mapping") && jsonelement["mapping"].IsArray()) {
        for (auto &v : jsonelement["mapping"].GetArray()) {
            MacroDeviceOutputMapping *m = new MacroDeviceOutputMapping();
            if (m->DeserializeJSON(v)) {
                outputMappings.push_back(m);
            } else {
                ESP_LOGE("MacroDeviceDefinition", "Failed to deserialize an output mapping in 'mapping' array");
                delete m;
            }
        }
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'mapping' field");
        return false;
    }

    return true;
}

bool MacroDeviceDefinition::SerializeJSONInto(rapidjson::Document &doc) {
    doc.SetObject();

    Value s_id(kObjectType);
    s_id.SetString(id, doc.GetAllocator());

    doc.AddMember("id", s_id, doc.GetAllocator());
    doc.AddMember("name", Value(name.c_str(), doc.GetAllocator()), doc.GetAllocator());
    doc.AddMember("machine", Value(synthId.c_str(), doc.GetAllocator()), doc.GetAllocator());

    Value groups(kArrayType);
    doc.AddMember("groups", groups, doc.GetAllocator());
    for (auto g : parameterGroups) {
        Document d2;
        if (g->SerializeJSONInto(d2)) {
            Value copyOfd2(d2, doc.GetAllocator());
            doc["groups"].PushBack(copyOfd2.Move(), doc.GetAllocator());
        }
    }

    Value mapping(kArrayType);
    doc.AddMember("mapping", mapping, doc.GetAllocator());
    for (auto m : outputMappings) {
        Document d2;
        if (m->SerializeJSONInto(d2)) {
            Value copyOfd2(d2, doc.GetAllocator());
            doc["mapping"].PushBack(copyOfd2.Move(), doc.GetAllocator());
        }
    }

    return true;
}
