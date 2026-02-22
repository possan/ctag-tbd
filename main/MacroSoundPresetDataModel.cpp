#include "MacroSoundPresetDataModel.hpp"
#include "MacroSoundPresetGroup.hpp"
#include "MacroSoundPreset.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <dirent.h>
#include "esp_log.h"
#include "ctagResources.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


MacroSoundPresetDataModel::MacroSoundPresetDataModel() {
    presets.clear();
    groups.clear();
}

MacroSoundPresetDataModel::~MacroSoundPresetDataModel() {
}

void MacroSoundPresetDataModel::ReloadSoundPresets() {
    ESP_LOGI("MacroSoundPresetDataModel", "Trying to read macro sound preset file");

    Document d;

    presets.clear();

    DIR *dir;
    struct dirent *ent;
    Value sparray(kArrayType);
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fn(ent->d_name);
            ESP_LOGI("MacroSoundPresetDataModel", "Filename: %s", fn.c_str());

            Document d;
            loadJSON(d, path + "/" + fn);
            if(!d.HasParseError()) {
                MacroSoundPreset *preset = new MacroSoundPreset();
                if(preset->DeserializeJSON(d)) {
                    ESP_LOGI("MacroSoundPresetDataModel", "Deserialized macro sound preset: #%s \"%s\"", preset->id.c_str(), preset->displayName.c_str());
                    presets.push_back(preset);
                } else {
                    ESP_LOGE("MacroSoundPresetDataModel", "Failed to deserialize macro sound preset from file %s", fn.c_str());
                    delete preset;
                }
            }
        }
        closedir(dir);
    } else {
        ESP_LOGE("MacroSoundPresetDataModel", "Could not open directory %s", path.c_str());
    }
}

MacroSoundPreset *MacroSoundPresetDataModel::GetMacroSoundPreset(
    std::string id) {
    return new MacroSoundPreset();
}

int MacroSoundPresetDataModel::GetNumberOfSoundPresetGroups() {
    return 0;
}

void MacroSoundPresetDataModel::GetMacroSoundPresetGroupId(int index, std::string *idOutput) {
    idOutput->assign("xyz");
}

int MacroSoundPresetDataModel::GetNumberOfSoundPresets() {
    return 0;
}

void MacroSoundPresetDataModel::GetMacroSoundPresetId(int index, std::string *idOutput) {
    idOutput->assign("xyz");
}

void MacroSoundPresetDataModel::GetPresetIndexJson(std::string *target) {
    Document d;
    target->assign("{}");
}

void MacroSoundPresetDataModel::SerializeListJSON(std::string *output) {
    Document doc;

    doc.SetObject();

    Value presetsarray(kArrayType);
    doc.AddMember("presets", presetsarray, doc.GetAllocator());
    for(MacroSoundPreset *s : presets) {
        doc["presets"].PushBack(Value(s->id.c_str(), doc.GetAllocator()), doc.GetAllocator());
        // Document d2;
        // if (s->SerializeJSONInto(d2)) {
        //     Value copyOfd2(d2, doc.GetAllocator());
        //     doc["presets"].PushBack(copyOfd2.Move(), doc.GetAllocator());
        // } else {
        //     ESP_LOGE("MacroSoundPresetDataModel", "Failed to serialize macro sound preset #%s \"%s\"", s->id.c_str(), s->displayName.c_str());
        // }
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    ESP_LOGW("MacroSoundPresetDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

bool MacroSoundPresetDataModel::UpdatePreset(const std::string &jsonString) {
    Document d;
    if (d.Parse(jsonString.c_str()).HasParseError()) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to parse JSON string: %s", jsonString.c_str());
        return false;
    }

    if (!d.HasMember("id")) {
        ESP_LOGE("MacroSoundPresetDataModel", "JSON string does not contain 'id' field: %s", jsonString.c_str());
        return false;
    }

    std::string id = d["id"].GetString();

    // just save the file now when we know the id

    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    std::string filename = path + "/" + id + ".json";

    fp = fopen(filename.c_str(), "w");
    if (fp == NULL) {
        ESP_LOGE("MacroSoundPresetDataModel", "could not open file %s", filename.c_str());
        return false;
    }
    fwrite(jsonString.c_str(), 1, jsonString.size(), fp);
    fclose(fp);

    ReloadSoundPresets();

    return true;
}


void MacroSoundPresetDataModel::SerializeItemJSON(const std::string &id, std::string *output) {
    Document d;
    d.SetObject();

    for (MacroSoundPreset *s : presets) {
        if (s->id == id) {
            // Document d2;
            if (s->SerializeJSONInto(d)) {
                // d.Set(d2.Move());
                // Value copyOfd2(d2, d.GetAllocator());
                // d.AddMember("preset", copyOfd2.Move(), d.GetAllocator());
            } else {
                ESP_LOGE("MacroSoundPresetDataModel", "Failed to serialize macro sound preset #%s \"%s\"", s->id.c_str(), s->displayName.c_str());
            }
            break;
        }
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGW("MacroSoundPresetDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());

}
