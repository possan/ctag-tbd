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


// Base class for sound processors + interface

#pragma once

#define BUF_SZ 32

#define MK_BOOL_PAR(outname, inname) \
    bool outname = inname;\
    if(trig_##inname != -1) outname = data.trig[trig_##inname] == 1 ? false : true;

#define MK_BOOL_PAR_NOCV(outname, inname) \
    bool outname = inname;

#define MK_FLT_PAR_ABS(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * scale;

#define MK_FLT_PAR_ABS_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_FLT_PAR_ABS_ADD(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname += fabsf(data.cv[cv_##inname]) * scale;

#define MK_FLT_PAR_ABS_ADD_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_FLT_PAR_ABS_SFT(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname = (fabsf(data.cv[cv_##inname]) - 0.5f) * 2.f * scale;

#define MK_FLT_PAR_ABS_SFT_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_FLT_PAR(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname = data.cv[cv_##inname] * scale;

#define MK_FLT_PAR_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_INT_PAR_ABS(outname, inname, scale) \
    int outname = inname;\
    if(cv_##inname != -1) outname = static_cast<int>(fabsf(data.cv[cv_##inname]) * scale);

#define MK_INT_PAR_ABS_NOCV(outname, inname, scale) \
    int outname = inname * scale / 4096;

#define MK_INT_PAR(outname, inname, scale) \
    int outname = inname;\
    if(cv_##inname != -1) outname = static_cast<int>(data.cv[cv_##inname] * scale);

#define MK_INT_PAR_NOCV(outname, inname, scale) \
    int outname = inname;

#define MK_FLT_PAR_ABS_MIN_MAX(outname, inname, norm, out_min, out_max) \
    float outname = inname/norm * (out_max-out_min)+out_min; \
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * (out_max-out_min)+out_min;

#define MK_FLT_PAR_ABS_MIN_MAX_NOCV(outname, inname, norm, out_min, out_max) \
    float outname = inname/norm * (out_max-out_min)+out_min;

#define MK_FLT_PAR_ABS_PAN(outname, inname, norm, scale)  \
    float outname = (inname/norm+1.f)/2.f * scale; \
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * scale;

#define MK_FLT_PAR_ABS_PAN_NOCV(outname, inname, norm, scale)  \
    float outname = (inname/norm+1.f)/2.f * scale;

#define CC_TO_MAP_KEY(ch, cc) (((ch) * 256) + (cc))

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include "ctagSPDataModel.hpp"
#include "ctagSPAllocator.hpp"

using namespace std;

namespace CTAG {
    namespace SP {
        struct ProcessData {
            float *buf;
            float *cv;
            uint8_t *trig;
        };

        class ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) = 0; // pure virtual --> must be implemented by derived

            // plugins will need to make sure not to use more than blocksize bytes of data
            virtual void Init(std::size_t blockSize, void *blockPtr) = 0;

            virtual ~ctagSoundProcessor() {};

            // void* operator new (std::size_t size) {
            //     return ctagSPAllocator::Allocate(size);
            // }
            // void operator delete (void *ptr) noexcept {
            //     // arena allocator will just reset the arena
            // }
            // void* operator new[] (std::size_t size) = delete;
            // void* operator new[] (std::size_t size, const std::nothrow_t& tag) = delete;
            // void operator delete[] (void *ptr) noexcept = delete;

            int GetAudioBufferSize() { return bufSz; }

            void SetProcessChannel(int ch) { processCh = ch; }

            const char *GetCStrJSONParamSpecs() const { return model->GetCStrJSONParams(); }

            virtual const char *GetCStrID() { return id.c_str(); }
            virtual const string& GetID() { return id; }

            void SetParamValue(const string &id, const string &key, const int val) {
                setParamValueInternal(id, key, val); // as immediate as possible
                model->SetParamValue(id, key, val);
            }

            void SetChannelParamsFromCStrJSON(const string &json) {
                rapidjson::Document d;
                d.Parse(json.c_str());
                if (!d.IsObject()) {
                    ESP_LOGE("SP", "Invalid JSON for SetChannelParamsFromCStrJSON");
                    return;
                }
                if (!d.HasMember("params")) {
                    ESP_LOGE("SP", "Invalid JSON for SetChannelParamsFromCStrJSON");
                    return;
                }

                ESP_LOGD("SP", "SetChannelParamsFromCStrJSON %s", json.c_str());

                Value &params = d["params"];
                for (auto &v : params.GetArray()) {
                    if (!v.HasMember("id")) continue;
                    if (!v["id"].IsString()) continue;
                    string id = v["id"].GetString();

                    if (v.HasMember("current")) {
                        int current = v["current"].GetInt();
                        setParamValueInternal(id, "current", current);
                        model->SetParamValue(id, "current", current);
                    }

                    if (v.HasMember("cv")) {
                        int cv = v["cv"].GetInt();
                        setParamValueInternal(id, "cv", cv);
                        model->SetParamValue(id, "cv", cv);
                    }

                    if (v.HasMember("trig")) {
                        int trig = v["trig"].GetInt();
                        setParamValueInternal(id, "trig", trig);
                        model->SetParamValue(id, "trig", trig);
                    }
                }
            }

            const char *GetCStrJSONPresets() { return model->GetCStrJSONPresets(); }

            const char *GetCStrJSONAllPresetData() { return model->GetCStrJSONAllPresetData(); }

            bool GetIsStereo() const { return isStereo; }

            void SavePreset(const string &name, const int number) { model->SavePreset(name, number); }

            void LoadPreset(const int number) {
                model->LoadPreset(number); // first get the data into the model
                loadPresetInternal();
            }

            std::string GetActivePluginParameters() { return model->GetActivePluginParameters(); }
            void SetActivePluginParameters(std::string const& p) {
                model->SetActivePluginParameters(p);
                loadPresetInternal();
            }


            virtual void handleMidiNoteOff(const uint8_t channel, const uint8_t note, const uint8_t vel) {
                // override if needed
                // ESP_LOGI("SP", "Not overriden MIDI: note off %d, %d, %d", channel, note, vel);
            };

            virtual void handleMidiNoteOn(const uint8_t channel, const uint8_t note, const uint8_t vel) {
                // override if needed
                // ESP_LOGI("SP", "Not overriden MIDI: note on %d, %d, %d", channel, note, vel);
            };

            virtual void handleMidiAftertouch(const uint8_t channel, const uint8_t note, const uint8_t vel) {
                // override if needed
                // ESP_LOGI("SP", "Not overriden MIDI: aftertouch %d, %d, %d", channel, note, vel);
            };

            virtual void handleMidiControlChange(const uint8_t channel, const uint8_t control, const uint8_t value) {
                // override if needed
                // ESP_LOGI("SP", "Not overriden MIDI: CC %d, %d, %d", channel, control, value);
            };

            virtual void handleMidiPatchChange(const uint8_t channel, const uint8_t patch) {
                // override if needed
                // ESP_LOGI("SP", "Not overriden MIDI: Patch Change %d, %d", channel, patch);
            };

            virtual void handleMidiPitchBend(const uint8_t channel, const uint16_t bend) {
                // override if needed
                // ESP_LOGI("SP", "Not overriden MIDI: pitch bend %d, %d", channel, bend);
            };

        protected:

            virtual void knowYourself() = 0;

            virtual void setParamValueInternal(const string &id, const string &key, const int val) {
                //printf("%s, %s, %d\n", id.c_str(), key.c_str(), val);
                if (key.compare("current") == 0) {
                    auto it = pMapPar.find(id);
                    if (it != pMapPar.end()) {
                        (it->second)(val);
                    }
                    return;
                }
                if (key.compare("cv") == 0) {
                    if (val >= -1 && val < N_CVS) {
                        auto it = pMapCv.find(id);
                        if (it != pMapCv.end()) {
                            (it->second)(val);
                        }
                    }
                    return;
                }
                if (key.compare("trig") == 0) {
                    if (val >= -1 && val < N_TRIGS) {
                        auto it = pMapTrig.find(id);
                        if (it != pMapTrig.end()) {
                            (it->second)(val);
                        }
                    }
                    return;
                }
            };

            virtual void loadPresetInternal() {
                // iterate all parameters, take names from parameter map (first element)
                for (const auto &kv: pMapPar) {
                    setParamValueInternal(kv.first, "current", model->GetParamValue(kv.first, "current"));
                    // check if cv and trig are set in preset, if so set in processor param
                    if (model->IsParamCV(kv.first)) {
                        //ESP_LOGW("MOdel", "IntParam %s, %d", name.c_str(), model->GetParamValue(name, "cv"));
                        setParamValueInternal(kv.first, "cv", model->GetParamValue(kv.first, "cv"));
                    } else if (model->IsParamTrig(kv.first)) {
                        //ESP_LOGW("MOdel", "BoolParam %s, %d", name.c_str(), model->GetParamValue(name, "trig"));
                        setParamValueInternal(kv.first, "trig", model->GetParamValue(kv.first, "trig"));
                    }
                }
            };

            bool isStereo = false;
            int const bufSz = BUF_SZ;
            int processCh = 0;
            int instance {0};
            std::unique_ptr<ctagSPDataModel> model = nullptr;
            string id = "";
            map<string, function<void(const int)>> pMapPar;
            map<string, function<void(const int)>> pMapCv;
            map<string, function<void(const int)>> pMapTrig;
            map<const int, string> pMapCC;
        };
    }
}