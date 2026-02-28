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

int compareGroups(const void *a, const void *b) {
    MacroSoundPresetGroup *ga = *(MacroSoundPresetGroup**)a;
    MacroSoundPresetGroup *gb = *(MacroSoundPresetGroup**)b;
    return ga->displayName.compare(gb->displayName);
}

void MacroSoundPresetDataModel::ReloadSoundPresets() {
    // return;

    ESP_LOGI("MacroSoundPresetDataModel", "Trying to read macro sound preset file");

    Document d;

    presets.clear();
    groups.clear();

    DIR *dir;
    struct dirent *ent;
    Value sparray(kArrayType);
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fn(ent->d_name);
            ESP_LOGI("MacroSoundPresetDataModel", "Filename: %s", fn.c_str());


    ESP_LOGI("MacroSoundPresetDataModel", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

             Document d;
            loadJSON(d, path + "/" + fn);
            if(!d.HasParseError()) {
                MacroSoundPreset *preset = new MacroSoundPreset();
                if(preset->DeserializeJSON(d)) {
                    ESP_LOGI("MacroSoundPresetDataModel", "Deserialized macro sound preset: #%s \"%s\"", preset->id.c_str(), preset->displayName.c_str());
                    presets.push_back(preset);

                    MacroSoundPresetGroup *groupfound = nullptr;
                    for(MacroSoundPresetGroup *g : groups) {
                        if (g->id == preset->groupName) {
                            groupfound = g;
                            break;
                        }
                    }

                    if (groupfound != nullptr) {
                        groupfound->fileIds.push_back(preset->id);
                    } else {
                        MacroSoundPresetGroup *newGroup = new MacroSoundPresetGroup();
                        newGroup->id = preset->groupName;
                        newGroup->displayName = preset->groupName;
                        newGroup->fileIds.push_back(preset->id);
                        groups.push_back(newGroup);
                    }

                } else {
                    ESP_LOGE("MacroSoundPresetDataModel", "Failed to deserialize macro sound preset from file %s", fn.c_str());
                    delete preset;
                }
            }
        }
        closedir(dir);

        // TODO: Sort groups
        qsort(groups.data(), groups.size(), sizeof(MacroSoundPresetGroup*), compareGroups);

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

void MacroSoundPresetDataModel::GetPresetIndexJson(std::string *output) {
    Document doc;

    doc.SetObject();

    Value groupsarray(kArrayType);
    doc.AddMember("groups", groupsarray, doc.GetAllocator());
    for(MacroSoundPresetGroup *g : groups) {

        Value groupobj(kObjectType);
        groupobj.AddMember("name", Value(g->displayName.c_str(), doc.GetAllocator()), doc.GetAllocator());

        Value presetsarray(kArrayType);
        groupobj.AddMember("presets", presetsarray, doc.GetAllocator());
        for(std::string fid : g->fileIds) {
            groupobj["presets"].PushBack(Value(fid.c_str(), doc.GetAllocator()), doc.GetAllocator());
        }

        doc["groups"].PushBack(groupobj, doc.GetAllocator());
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    ESP_LOGW("MacroSoundPresetDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
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

    // ReloadSoundPresets();

    return true;
}


void MacroSoundPresetDataModel::SerializeItemJSON(const std::string &id, std::string *output) {
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    std::string filename = path + "/" + id + ".json";

    output->assign("");

    FILE *fp = fopen(filename.c_str(), "r");
    if (fp == NULL) {
        ESP_LOGE("MacroSoundPresetDataModel", "could not open file %s", filename.c_str());
        return;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *content = (char *) heap_caps_malloc(filesize + 32, MALLOC_CAP_SPIRAM);
    if (content == NULL) {
        ESP_LOGE("MacroSoundPresetDataModel", "could allocate memory");
        // output->assign("");
        fclose(fp);
        return;
    }

    fread(content, 1, filesize, fp);
    fclose(fp);

    content[filesize] = '\0';
    output->assign(content);

    heap_caps_free(content);
}

void MacroSoundPresetDataModel::DeleteItem(const std::string &id) {
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    std::string filename = path + "/" + id + ".json";
    ESP_LOGI("MacroSoundPresetDataModel", "Deleting file: %s", filename.c_str());
    unlink(filename.c_str());
}
