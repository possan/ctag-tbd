#include "SynthDefinitionDataModel.hpp"
#include "SynthDefinition.hpp"
#include "TrackDefinition.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <dirent.h>
#include "esp_log.h"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


SynthDefinitionDataModel::SynthDefinitionDataModel() {
    synths.clear();
    tracks.clear();
}

SynthDefinitionDataModel::~SynthDefinitionDataModel() {
}

// #define MB_BUF_SZ 1024

void SynthDefinitionDataModel::ReloadSynthDefinitions() {
    ESP_LOGI("SynthDefinitionDataModel", "Trying to read synth defintition file");

    #ifndef TBD_SIM
        const std::string MODELJSONFN = "/sdcard/data/synthdefinitions.json";
    #else
        const std::string MODELJSONFN = "../../sdcard_image/data/synthdefinitions.json";
    #endif

    Document d;
    loadJSON(d, MODELJSONFN);
    // char *buffer = (char *) heap_caps_malloc(MB_BUF_SZ, MALLOC_CAP_SPIRAM);
    // d.GetAllocator().Clear();
    // ESP_LOGI("SynthDefinitionDataModel", "read buffer");
    // //FILE*
    // FILE * fp = fopen(MODELJSONFN.c_str(), "r");
    // if (fp == NULL) {
    //     ESP_LOGE("SynthDefinitionDataModel", "could not open file %s", MODELJSONFN.c_str());
    //     return;
    // }
    // //char readBuffer[512];
    // ESP_LOGI("SynthDefinitionDataModel", "read stream");
    // FileReadStream is(fp, buffer, MB_BUF_SZ);
    // ESP_LOGI("SynthDefinitionDataModel", "trying to parse");
    // d.ParseStream(is);

    if (d.HasParseError()) {
        //     ESP_LOGE("SynthDefinitionDataModel", "JSON parse error: %d", d.GetParseError());
        return;
        // } else {
        //     ESP_LOGI("SynthDefinitionDataModel", "JSON parsed successfully");
    }

    DeserializeJSON(d);

    // synths.clear();
    // tracks.clear();

    // if (d.HasMember("synths")) {
    //     for (auto &v : d["synths"].GetArray()) {
    //         SynthDefinition *s = new SynthDefinition();
    //         if (s->DeserializeJSON(v)) {
    //             synths.push_back(s);
    //         } else {
    //             delete s;
    //         }
    //     }
    // }

    // if (d.HasMember("tracks")) {
    //     for (auto &v : d["tracks"].GetArray()) {
    //         TrackDefinition *t = new TrackDefinition();
    //         if (t->DeserializeJSON(v)) {
    //             tracks.push_back(t);
    //         } else {
    //             delete t;
    //         }
    //     }
    // }
}

int SynthDefinitionDataModel::GetNumberOfSynthDefinitions() {
    return synths.size();
}

void SynthDefinitionDataModel::GetSynthDeviceDefinitionId(int index, std::string *idOutput) {
}

SynthDefinition *SynthDefinitionDataModel::GetSynthDefinition(const std::string id) {
    return nullptr;
}

bool SynthDefinitionDataModel::DeserializeJSON(const rapidjson::Value &jsonelement) {
    synths.clear();
    tracks.clear();

    if (jsonelement.HasMember("machines")) {
        for (auto &v : jsonelement["machines"].GetArray()) {
            SynthDefinition *s = new SynthDefinition();
            if (s->DeserializeJSON(v)) {
                ESP_LOGI("SynthDefinitionDataModel", "Deserialized synth definition: #%s \"%s\"", s->id.c_str(), s->name.c_str());
                synths.push_back(s);
            } else {
                delete s;
            }
        }
    }

    if (jsonelement.HasMember("tracks")) {
        for (auto &v : jsonelement["tracks"].GetArray()) {
            TrackDefinition *t = new TrackDefinition();
            if (t->DeserializeJSON(v)) {
                ESP_LOGI("SynthDefinitionDataModel", "Deserialized track definition: #%d \"%s\"", t->index, t->name.c_str());
                tracks.push_back(t);
            } else {
                delete t;
            }
        }
    }

    return true;
}

bool SynthDefinitionDataModel::DeserializeJSON(const std::string *jsonString) {
    Document document;
    if (document.Parse(jsonString->c_str()).HasParseError()) {
        return false;
    }
    // Implement deserialization logic here
    return true;
}

void SynthDefinitionDataModel:: SerializeTrackJSON(int index, std::string *output){
    Document d;

    d.SetObject();

    Value tracksarray(kArrayType);
    d.AddMember("tracks", tracksarray, d.GetAllocator());
    for(TrackDefinition *t : tracks) {
        if (t->index == index) {
            d.AddMember("index", t->index, d.GetAllocator());
            d.AddMember("name", Value(t->name.c_str(), d.GetAllocator()), d.GetAllocator());

            Value machinearray(kArrayType);
            for(auto k : t->macroMachineIds) {
                machinearray.PushBack(Value(k.c_str(), d.GetAllocator()), d.GetAllocator());
            }
            d.AddMember("machines", machinearray, d.GetAllocator());
            // d["tracks"].PushBack(d, d.GetAllocator());
        }
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGW("SynthDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

void SynthDefinitionDataModel::SerializeSynthJSON(const std::string id, std::string *output){
 // Implement serialization logic here
    Document d;

    d.SetObject();

    // Value machinesarray(kArrayType);
    // d.AddMember("machines", machinesarray, d.GetAllocator());
    for(SynthDefinition *s : synths) {
        if (s->id == id ) {

            // Value machinejson(kObjectType);
            d.AddMember("id", Value(s->id.c_str(), d.GetAllocator()), d.GetAllocator());
            d.AddMember("name", Value(s->name.c_str(), d.GetAllocator()), d.GetAllocator());
            // machinejson.AddMember("type", s->type, d.GetAllocator());
            
            Value paramarray(kArrayType);
            d.AddMember("parameters", paramarray, d.GetAllocator());
            
            for(auto p : s->parameters) {
                Value param(kObjectType);

                param.AddMember("id", Value(p->id.c_str(), d.GetAllocator()), d.GetAllocator());
                param.AddMember("name", Value(p->name.c_str(), d.GetAllocator()), d.GetAllocator());
                if (p->type == SynthParameterType_CC) {
                    param.AddMember("type", "CC", d.GetAllocator());
                } else if (p->type == SynthParameterType_NRPM) {
                    param.AddMember("type", "NRPM", d.GetAllocator());
                } else {
                    param.AddMember("type", "None", d.GetAllocator());
                }
                param.AddMember("cc", p->cc, d.GetAllocator());
                param.AddMember("default", p->defaultValue, d.GetAllocator());

                d["parameters"].PushBack(param, d.GetAllocator());
            }
            
            // d["machines"].PushBack(machinejson, d.GetAllocator());


        // if (s->SerializeJSONInto(machinejson, d.GetAllocator())) {
        // }
    }
        }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGW("SynthDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

void SynthDefinitionDataModel::SerializeListJSON(std::string *output) {
    // Implement serialization logic here
    Document d;

    d.SetObject();

    Value machinesarray(kArrayType);
    d.AddMember("machines", machinesarray, d.GetAllocator());
    for(SynthDefinition *s : synths) {
        Value machineid = Value(s->id.c_str(), d.GetAllocator());
        d["machines"].PushBack(machineid, d.GetAllocator());
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGW("SynthDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}


void SynthDefinitionDataModel::SerializeStateJSON(std::string *output) {
    // Implement serialization logic here
    Document d;

    d.SetObject();

    // Value machinesarray(kArrayType);
    // d.AddMember("machines", machinesarray, d.GetAllocator());
    // for(SynthDefinition *s : synths) {
    //     Value machinejson(kObjectType);
    //     machinejson.AddMember("id", Value(s->id.c_str(), d.GetAllocator()), d.GetAllocator());
    //     d["machines"].PushBack(machinejson, d.GetAllocator());
    // }

    Value tracksarray(kArrayType);
    d.AddMember("tracks", tracksarray, d.GetAllocator());
    for(TrackDefinition *t : tracks) {
        Value trackjson(kObjectType);
        trackjson.AddMember("index", t->index, d.GetAllocator());
        trackjson.AddMember("activeMachineId", Value(t->activeMachineId.c_str(), d.GetAllocator()), d.GetAllocator());
        d["tracks"].PushBack(trackjson, d.GetAllocator());
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGW("SynthDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}
