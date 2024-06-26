/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/


#include "ctagSPDataModel.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "esp_log.h"
#include "ctagResources.hpp"

/*
#ifndef TBD_SIM
#define SPIFFS_PATH "/spiffs"
#else
#define SPIFFS_PATH "../../spiffs_image"
#endif
 */

using namespace CTAG::SP;

ctagSPDataModel::ctagSPDataModel(const string &id, const bool isStereo) {
    // acquire data from json files ui model and patch model
    muiFileName = string(CTAG::RESOURCES::spiffsRoot + "/data/sp/mui-") + id + string(".jsn");
    //std::cout << "Reading " << muiFileName << std::endl;
    loadJSON(mui, muiFileName);
    mpFileName = string(CTAG::RESOURCES::spiffsRoot + "/data/sp/mp-") + id + string(".jsn");
    //std::cout << "Reading " << mpFileName << std::endl;
    loadJSON(mp, mpFileName);
    // load last activated preset
    ESP_LOGD("Model", "Loading patch number %d", mp["activePatch"].GetInt());
    LoadPreset(mp["activePatch"].GetInt());

}

ctagSPDataModel::~ctagSPDataModel() {
}

const char *ctagSPDataModel::GetCStrJSONParams() {
    json.Clear();
    mergeModels();
    Writer<StringBuffer> writer(json);
    mui.Accept(writer);
    //printf("%s\n", json.GetString());
    return json.GetString();
}

void ctagSPDataModel::mergeModels() {
    // iterate preset model for all parameters
    if (!activePreset.HasMember("params")) return;
    if (!mui.HasMember("params")) return;
    Value &patchParams = activePreset["params"];
    for (auto &v : patchParams.GetArray()) {
        //printf("Searching for %s in ui model", v["id"].GetString());
        recursiveFindAndInsert(v, mui["params"]);
    }
}

void ctagSPDataModel::SetParamValue(const string &id, const string &key, const int val) {
    ESP_LOGD("Model", "Setting id %s, with %s to %d", id.c_str(), key.c_str(), val);
    if (!activePreset.HasMember("params")) return;
    Value &patchParams = activePreset["params"];
    if (!patchParams.IsArray()) return;
    for (auto &v : patchParams.GetArray()) {
        //printf("Searching for %s in ui model", v["id"].GetString());
        if (!v.HasMember("id")) return;
        if (!v["id"].IsString()) return;
        if (v["id"] == id) {
            ESP_LOGD("Model", "I found the value");
            if (!v.HasMember(key)) return;
            v[key] = val;
        }
    }
}

int ctagSPDataModel::GetParamValue(const string &id, const string &key) {
    if (!activePreset.HasMember("params")) return 0;
    Value &patchParams = activePreset["params"];
    if (!patchParams.IsArray()) return 0;
    for (auto &v : patchParams.GetArray()) {
        if (!v.HasMember("id")) return 0;
        if (!v["id"].IsString()) return 0;
        if (v["id"] == id) {
            if (!v.HasMember(key)) return 0;
            return v[key].GetInt();
        }
    }
    return 0;
}


const char *ctagSPDataModel::GetCStrJSONPresets() {
    if (!mp.HasMember("activePatch")) return nullptr;
    if (!mp["activePatch"].IsInt()) return nullptr;
    json.Clear();
    int num = 0;
    // data to be printed
    Value obj(kObjectType);
    Value a(kArrayType);
    Value n(kNumberType);
    // temporary root documents p is export d is patch file
    Document p, d;
    // data to be printed
    n.SetInt(mp["activePatch"].GetInt());
    obj.AddMember("activePresetNumber", n, p.GetAllocator());
    // load presets to document d
    loadJSON(d, mpFileName);
    if (!d.HasMember("patches")) return nullptr;
    if (!d["patches"].IsArray()) return nullptr;
    // iterate presets
    for (auto &v : d["patches"].GetArray()) {
        // construct items for export document
        Value o(kObjectType);
        Value name(kStringType);
        name.SetString(v["name"].GetString(), p.GetAllocator());
        o.AddMember("name", name.Move(), p.GetAllocator());
        o.AddMember("number", num++, p.GetAllocator());
        a.PushBack(o, p.GetAllocator());
    }
    obj.AddMember("presets", a, p.GetAllocator());
    Writer<StringBuffer> writer(json);
    obj.Accept(writer);
    //printf("%s\n", json.GetString());
    return json.GetString();
}

void ctagSPDataModel::LoadPreset(const int num) {
    loadJSON(mp, mpFileName);
    int patchNum = num;
    if (patchNum < 0) patchNum = 0;
    if (!mp.HasMember("patches")) return;
    if (!mp["patches"].IsArray()) return;
    if (patchNum >= mp["patches"].GetArray().Size()) {
        ESP_LOGD("Model", "Bounds check patch num %d, size %d", patchNum, mp["patches"].GetArray().Size());
        patchNum = mp["patches"].GetArray().Size() - 1;
    }
    activePreset.GetAllocator().Clear();
    activePreset.CopyFrom(mp["patches"].GetArray()[patchNum], activePreset.GetAllocator());
    ESP_LOGD("Model", "Preset Name is %s number %d", activePreset["name"].GetString(), patchNum);
    // save currently loaded preset to model
    if (!mp.HasMember("activePatch")) return;
    mp["activePatch"].SetInt(patchNum);
    storeJSON(mp, mpFileName);
    json.Clear();
    Writer<StringBuffer> writer(json);
    mp.Accept(writer);
}

void ctagSPDataModel::recursiveFindAndInsert(const Value &paramF, Value &paramI) {
    if (!paramI.IsArray()) return;
    for (auto &v : paramI.GetArray()) {
        //printf("Matching for %s in with %s", paramF["id"].GetString(), v["id"].GetString());
        if (!paramF.HasMember("id")) return;
        if (!v.HasMember("id")) return;
        if (paramF["id"] == v["id"]) {
            //printf("Found, inserting data...");
            Value::MemberIterator iter = v.FindMember("current");
            if (iter == v.MemberEnd())
                v.AddMember("current", paramF["current"].GetInt(), mui.GetAllocator());
            else
                iter->value = paramF["current"].GetInt();
            if (v["type"] == "bool" && paramF.HasMember("trig")) {
                Value::MemberIterator iter = v.FindMember("trig");
                if (iter == v.MemberEnd())
                    v.AddMember("trig", paramF["trig"].GetInt(), mui.GetAllocator());
                else
                    iter->value = paramF["trig"].GetInt();
            } else if (v["type"] == "int" && paramF.HasMember("cv")) {
                Value::MemberIterator iter = v.FindMember("cv");
                if (iter == v.MemberEnd())
                    v.AddMember("cv", paramF["cv"].GetInt(), mui.GetAllocator());
                else
                    iter->value = paramF["cv"].GetInt();
            }
            break;
        } else if (v["type"] == "group") {
            //printf("Going for a group recursion");
            recursiveFindAndInsert(paramF, v["params"]);
        }
    }
}


void ctagSPDataModel::SavePreset(const string &name, const int number) {
    //ESP_LOGE("Model", "Save preset %s %d", name.c_str(), number);
    //ESP_LOGE("MOdel", "Stored JSON before");
    //PrintSelf();
    loadJSON(mp, mpFileName);
    int patchNum = number;
    if (patchNum < 0) patchNum = 0;
    if (!mp.HasMember("patches")) return;
    if (!mp["patches"].IsArray()) return;
    if (patchNum > mp["patches"].GetArray().Size()) patchNum = mp["patches"].GetArray().Size();
    if (!activePreset.HasMember("name")) return;
    activePreset["name"].SetString(name, activePreset.GetAllocator());
    //ESP_LOGE("Model", "Adding new number %d, patchnum %d, patch array size %d", number, patchNum, mp["patches"].GetArray().Size());
    if (patchNum == mp["patches"].GetArray().Size()) {
        Value copyOfPreset(activePreset, mp.GetAllocator());
        //copyOfPreset["name"].SetString(name, mp.GetAllocator());
        mp["patches"].PushBack(copyOfPreset.Move(), mp.GetAllocator());
        /*
        ESP_LOGE("Model", "Adding new number %d, patchnum %d, patch array size %d", number, patchNum,
                 mp["patches"].GetArray().Size());
                 */
    } else {
        mp["patches"][patchNum].CopyFrom(activePreset, mp.GetAllocator()); // saved preset is current
        //ESP_LOGE("Model", "Replace");
    }
    if (!mp.HasMember("activePatch")) return;
    mp["activePatch"] = patchNum;
    storeJSON(mp, mpFileName);
    //ESP_LOGE("MOdel", "Stored JSON after");
    //PrintSelf();
}

void ctagSPDataModel::PrintSelf() {
    ESP_LOGD("Model", "mp:");
    printJSON(mp);
    ESP_LOGD("Model", "mui:");
    printJSON(mui);
    ESP_LOGD("Model", "activePreset:");
    printJSON(activePreset);
}

const char *ctagSPDataModel::GetCStrJSONAllPresetData() {
    Document d;
    json.Clear();
    loadJSON(d, mpFileName);
    Writer<StringBuffer> writer(json);
    d.Accept(writer);
    //printf("%s\n", json.GetString());
    return json.GetString();
}

bool ctagSPDataModel::IsParamTrig(const string &id) {
    if (!activePreset.HasMember("params")) return false;
    Value &patchParams = activePreset["params"];
    if (!patchParams.IsArray()) return false;
    for (auto &v : patchParams.GetArray()) {
        if (!v.HasMember("id")) return false;
        if (!v["id"].IsString()) return false;
        if (v["id"] == id) {
            if (v.HasMember("trig")) return true;
        }
    }
    return false;
}

bool ctagSPDataModel::IsParamCV(const string &id) {
    if (!activePreset.HasMember("params")) return false;
    Value &patchParams = activePreset["params"];
    if (!patchParams.IsArray()) return false;
    for (auto &v : patchParams.GetArray()) {
        if (!v.HasMember("id")) return false;
        if (!v["id"].IsString()) return false;
        if (v["id"] == id) {
            if (v.HasMember("cv")) return true;
        }
    }
    return false;
}

std::string ctagSPDataModel::GetActivePluginParameters() {
    rapidjson::StringBuffer parameters;
    rapidjson::Writer<rapidjson::StringBuffer> writer(parameters);
    activePreset.Accept(writer);
    return parameters.GetString();
}

void ctagSPDataModel::SetActivePluginParameters(const std::string &parameters) {
    Document d;
    d.Parse(parameters.c_str());
    activePreset.GetAllocator().Clear();
    activePreset.CopyFrom(d, activePreset.GetAllocator());
}
