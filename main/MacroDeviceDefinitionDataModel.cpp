#include "MacroDeviceDefinitionDataModel.hpp"
#include "MacroDeviceDefinition.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <dirent.h>
#include "esp_log.h"
#include "ctagResources.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


MacroDeviceDefinitionDataModel::MacroDeviceDefinitionDataModel() {
}

MacroDeviceDefinitionDataModel::~MacroDeviceDefinitionDataModel() {
}

void MacroDeviceDefinitionDataModel::ReloadMachineDefinitions() {
    ESP_LOGI("MacroDeviceDefinitionDataModel", "Trying to read macro device definition file");

    Document d;

    definitions.clear();

    DIR *dir;
    struct dirent *ent;
    Value sparray(kArrayType);
    // m.AddMember("availableProcessors", sparray, m.GetAllocator());
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrodefinitions"));
    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fn(ent->d_name);
            // if (fn.find("mui-") != std::string::npos) {
            ESP_LOGI("MacroDeviceDefinitionDataModel", "Filename: %s", fn.c_str());

            Document d;
            loadJSON(d, path + "/" + fn);
            if(!d.HasParseError()) {
                MacroDeviceDefinition *def = new MacroDeviceDefinition();
                if(def->DeserializeJSON(d)) {
                    ESP_LOGI("MacroDeviceDefinitionDataModel", "Deserialized macro device definition: #%s \"%s\"", def->id.c_str(), def->name.c_str());
                    definitions.push_back(def);
                } else {
                    ESP_LOGE("MacroDeviceDefinitionDataModel", "Failed to deserialize macro device definition from file %s", fn.c_str());
                    delete def;
                }
            }
        }
        closedir(dir);
    } else {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "Could not open directory %s", path.c_str());
    }

}

MacroDeviceDefinition *MacroDeviceDefinitionDataModel::GetMacroDeviceDefinition(
    std::string id) {
    Document d;
    return new MacroDeviceDefinition();
}


void MacroDeviceDefinitionDataModel::SerializeListJSON(std::string *output) {
    Document d;
    d.SetObject();

    Value machinesarray(kArrayType);
    d.AddMember("machines", machinesarray, d.GetAllocator());
    for(MacroDeviceDefinition *s : definitions) {
        d["machines"].PushBack(Value(s->id.c_str(), d.GetAllocator()), d.GetAllocator());
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGW("MacroDeviceDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

bool MacroDeviceDefinitionDataModel::UpdateDefinition(const std::string &jsonString) {
    Document d;

    if (d.Parse(jsonString.c_str()).HasParseError()) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "Failed to parse JSON string: %s", jsonString.c_str());
        return false;
    }

    if (!d.HasMember("id")) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "JSON string does not contain 'id' field: %s", jsonString.c_str());
        return false;
    }

    std::string id = d["id"].GetString();

    // just save the file now when we know the id

    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrodefinitions"));
    std::string filename = path + "/" + id + ".json";

    fp = fopen(filename.c_str(), "w");
    if (fp == NULL) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "could not open file %s", filename.c_str());
        return false;
    }
    fwrite(jsonString.c_str(), 1, jsonString.size(), fp);
    fclose(fp);

    ReloadMachineDefinitions();

    return true;
}


void MacroDeviceDefinitionDataModel::SerializeItemJSON(const std::string &id, std::string *output) {
    Document d;
    d.SetObject();

    for(MacroDeviceDefinition *s : definitions) {
        if (s->id == id) {
            // Document d2;
            if (s->SerializeJSONInto(d)) {
                // d = d2.Move();
            } else {
                ESP_LOGE("MacroDeviceDefinitionDataModel", "Failed to serialize macro device definition #%s \"%s\" into JSON", s->id.c_str(), s->name.c_str());
            }
            break;
        }
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGW("MacroDeviceDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}