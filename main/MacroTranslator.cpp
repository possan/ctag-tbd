#include "MacroTranslator.hpp"
#include "esp_log.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "SynthDefinition.hpp"
#include "TrackDefinition.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


MacroTranslator::MacroTranslator() {
    synthDefinitionModel = nullptr;
    macroSoundDefinitionModel = nullptr;
    macroDeviceDefinitionModel = nullptr;

    for (int i = 0; i < 16; i++) {
        trackToMidiChannel[i] = -1;
        trackBaseCC[i] = 0;
        trackMachineId[i] = "";
        definition[i] = nullptr;
        trackDirty[i] = false;

        for (int j = 0; j < 32; j++) {
            trackParameterValues[i][j] = 0;
        }
    }
};

MacroTranslator::~MacroTranslator() {
};

void MacroTranslator::SetTrackMachine(const int trackIndex, const std::string synthID) {
    if (synthID == trackMachineId[trackIndex]) {
        return;
    }

    ESP_LOGD("MacroTranslator", "Track %d machine set to %s",
        trackIndex, synthID.c_str());
    trackMachineId[trackIndex] = synthID;

    SynthDefinition *synthDef = synthDefinitionModel->GetSynthDefinition(synthID);
    if (synthDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Synth definition not found for id %s",
            synthID.c_str());
        return;
    }

    int idx = 0;

    TrackDefinition *trackDef = synthDefinitionModel->GetTrackDefinition(trackIndex);
    if (trackDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Track definition not found for track index %d",
            trackIndex);
        return;
    }

    trackToMidiChannel[trackIndex] = trackDef->midiChannel;
    // midiChannelToTrack[trackDef->midiChannel] = trackIndex;
    trackBaseCC[trackIndex] = trackDef->baseCC;

    idx = 0;
    for(auto par : synthDef->parameters) {
        ESP_LOGI("MacroTranslator", "Processing parameter %s, type %d, cc %d",
        par->id.c_str(), par->type, par->cc);

        trackParameterValues[trackIndex][idx] = par->defaultValue;

        if (par->type == SynthParameterType_CC) {
            soundProcessor->handleMidiControlChange(
                trackDef->midiChannel,
                trackBaseCC[trackIndex] + par->cc,
                par->defaultValue
            );
        }

        idx ++;
    }

    trackDirty[trackIndex] = true;
}

void MacroTranslator::SetTrackMacroDefinition(const int trackIndex, MacroDeviceDefinition *def) {
    // ESP_LOGI("MacroTranslator", "Setting track %d macro definition 0x%08X",
    // trackIndex, (uintptr_t)def);
    if (def != nullptr) {
        ESP_LOGI("MacroTranslator", "Macro def: \"%s\" \"%s\" \"%s\"",
            def->id.c_str(), def->name.c_str(), def->synthId.c_str());
    }

    if (definition[trackIndex] != nullptr) {
        delete definition[trackIndex];
        definition[trackIndex] = nullptr;
    }

    if (def == nullptr) {
        SetTrackMachine(trackIndex, "");
        return;
    }

    SynthDefinition *synthDef =
        synthDefinitionModel->GetSynthDefinition(def->synthId);

    if (synthDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Invalid synth referenceSynth definition not found for id %s",
            def->synthId.c_str());
    }

    // TrackDefinition *trackDef = synthDefinitionModel->GetTrackDefinition(trackIndex);
    // if (trackDef == nullptr) {
    //     ESP_LOGE("MacroTranslator", "Track definition not found for track index %d",
    //         trackIndex);
    //     return;
    // }

    // Document d1;
    // d1.SetObject();
    // if (def->SerializeJSONInto(d1)) {
    // ESP_LOGI("MacroTranslator", "dummy 5");
    // ESP_LOGI("MacroTranslator", "Mem 1 freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
    //     heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
    //     heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
    //     heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
    //     heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    MacroDeviceDefinition *defcopy = def->copy();

    // ESP_LOGI("MacroTranslator", "Mem 2 freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
    //     heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
    //     heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
    //     heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
    //     heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    
    // if (defcopy == nullptr) {
    definition[trackIndex] = defcopy;
    // ESP_LOGI("MacroTranslator", "dummy 6");
    // } else {
    //     ESP_LOGE("MacroTranslator", "Failed to deserialize macro definition into JSON");
    //     delete defcopy;
    //     ESP_LOGI("MacroTranslator", "dummy 7");
    // }
    // } else {
    //     ESP_LOGE("MacroTranslator", "Failed to serialize macro definition into JSON");
    // }
    // ESP_LOGI("MacroTranslator", "dummy 8");


    SetTrackMachine(trackIndex, def->synthId);


    // ESP_LOGI("MacroTranslator", "dummy 9");

}

void MacroTranslator::SetTrackParameter(const int trackIndex, int parameterIndex, int32_t value) {
    if (trackIndex < 0 || trackIndex >= 16) {
        // ESP_LOGE("MacroTranslator", "Track index out of range: %d", trackIndex);
        return;
    }

    if (parameterIndex < 0 || parameterIndex >= 32) {
        // ESP_LOGE("MacroTranslator", "Parameter index out of range: %d", parameterIndex);
        return;
    }

    // ESP_LOGI("MacroTranslator", "Track %d, Parameter %d = %d",
    //     trackIndex, parameterIndex, value);
    trackParameterValues[trackIndex][parameterIndex] = value;
    trackDirty[trackIndex] = 1;
}


void MacroTranslator::SetTrackParametersFromJSON(const std::string &parametersJSON) {
    Document d;

    d.Parse(parametersJSON.c_str());
    if (d.HasParseError()) {
        ESP_LOGE("MacroTranslator", "Failed to parse parameters JSON: %s", parametersJSON.c_str());
        return;
    }

    if (!d.IsObject()) {
        ESP_LOGE("MacroTranslator", "Parameters JSON is not an object: %s", parametersJSON.c_str());
        return;
    }

    int trackIndex = 0;
    if (d.HasMember("track")) {
        trackIndex = d["track"].GetInt();
    } else {
        ESP_LOGE("MacroTranslator", "Parameters JSON does not contain 'trackIndex': %s", parametersJSON.c_str());
        return;
    }

    if (d.HasMember("macro")) {
        std::string macro = d["macro"].GetString();
        MacroDeviceDefinition *def =
            macroDeviceDefinitionModel->LoadMacroDeviceDefinition(macro);
        ESP_LOGI("MacroTranslator", "Setting track %d macro definition to %s => 0x%08X", trackIndex, macro.c_str(), (uintptr_t)def);
        this->SetTrackMacroDefinition(trackIndex, def);
        delete def;
    }

    if (d.HasMember("machine")) {
        std::string machine = d["machine"].GetString();
        ESP_LOGI("MacroTranslator", "Setting track %d machine to: %s", trackIndex, machine.c_str());
        this->SetTrackMachine(trackIndex, machine);
    }

    if (d.HasMember("parameters")) {
        const Value& params = d["parameters"];
        if (!params.IsArray()) {
            ESP_LOGE("MacroTranslator", "'parameters' member is not an array: %s", parametersJSON.c_str());
            return;
        }

        int values[16] = {};
        for(int k=0; k<16; k++) {
            values[k] = -1;
        }
        int idx = 0;
        for (auto &v : params.GetArray()) {
            if (v.IsInt()) {
                values[idx] = v.GetInt();
                trackParameterValues[trackIndex][idx] = values[idx];
                trackDirty[trackIndex] = 1;
                // ESP_LOGD("MacroTranslator", "Set track %d parameter %d to %d",
                //     trackIndex, idx, values[idx]);
            }

            idx ++;
        }

        // MacroDeviceDefinition *def = definition[trackIndex];
        // if (def == nullptr) {
        //     ESP_LOGE("MacroTranslator", "No macro definition for track %d", trackIndex);
        //     return;
        // }
    }
}


void MacroTranslator::_parseIncomingMidiMessages(const uint8_t *buf, const size_t len) {
    // if (len > 1) {
    //     ESP_LOGI("MacroTranslator",
    //         "parseIncomingMidiMessages: %02X %02X %02X %02X %02X %02X %02X %02X %02X (%d)",
    //             buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], len);
    // }

    if (buf == nullptr || len < 1)  {
        return;
    }

    int left = len;
    int o = 0;

    while(left > 0) {
        uint8_t b0 = buf[o++];
        left --;

        if (b0 == 0) {
            // probably end of event stream
            return;
        }

        uint8_t inputchannel = (b0 & 0x0F);
        uint8_t cmd = (b0 & 0xF0);
        // int trackindex = midiChannelToTrack[inputchannel];

        switch(cmd) {
            case 0x80: // note off
            {
                if (left < 2) return; // not enough data

                uint8_t note = buf[o++];
                uint8_t velocity = buf[o++];
                left -= 2;

                soundProcessor->handleMidiNoteOff(inputchannel, note, velocity);
                break;
            }
            case 0x90: // note on
            {
                if (left < 2) return; // not enough data

                uint8_t note = buf[o++];
                uint8_t velocity = buf[o++];
                left -= 2;

                if (velocity > 0) {
                    // ESP_LOGI("MacroTranslator", "Note on, channe %d, note %d, velocity %d",
                    //     inputchannel, note, velocity);
                    soundProcessor->handleMidiNoteOn(inputchannel, note, velocity);
                } else {
                    soundProcessor->handleMidiNoteOff(inputchannel, note, velocity);
                }
                break;
            }
            case 0xA0: // aftertouch
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                break;
            }
            case 0xB0: // control change
            {
                if (left < 2) return; // not enough data

                uint8_t control = buf[o++];
                uint8_t value = buf[o++];
                left -= 2;

                for(int t=0; t<16; t++) {
                    if (trackToMidiChannel[t] == inputchannel) {
                        // First change machines if needed.
                        // soundProcessor->setTrackMachine(t, trackMachineId[t]);
                        int macrocc = (int)control - trackBaseCC[t];
                        macrocc -= 8; // input parameter CC's start at 8 too and...
                        // ESP_LOGI("MacroTranslator", "CC, track %d, control %d, value %d",
                        //     track, control, value);
                        this->SetTrackParameter(t, macrocc, value);
                    }
                }
                break;
            }
            case 0xC0: // program change
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++]; // not used?
                left -= 2;
                break;
            }
            case 0xE0: // pitch bend
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                break;
            }
            case 0xF0: // system common / real time
                // TODO handle sysex, clock, start, stop, continue, ...
                switch(b0) {
                    case 0xF0: // sysex start
                        return; // just ignore and bail out for now

                    case 0xF8: // timing clock
                    case 0xFA: // start
                    case 0xFB: // continue
                    case 0xFC: // stop
                    case 0xFE: // active sensing
                    case 0xFF: // system reset
                        // ignore for now
                        break;
                }
                break; // fail for now
            default:
                // for anything else, just stop parsing.
                return;
        }
    }
}


void MacroTranslator::TranslateInput(CTAG::SP::ProcessData *pd) {
    if (pd == nullptr)
        return;

    for(int t=0; t<16; t++) {
        if (trackDirty[t]) {
            // First change machines if needed.
            soundProcessor->setTrackMachine(t, trackMachineId[t]);
        }
    }

    _parseIncomingMidiMessages(pd->midi_bytes, pd->midi_bytes_length);

    for(int t=0; t<16; t++) {
        if (trackDirty[t]) {
            trackDirty[t] = false;

            // TODO: copy mapping instead.
            MacroDeviceDefinition *def = definition[t];

            // ESP_LOGI("MacroTranslator", "Track %d is dirty, def=0x%08X", t, (uintptr_t)def);

            if (def != nullptr) {
                // ESP_LOGI("MacroTranslator", "Using definition: %s, %s",
                //     def->id.c_str(), def->name.c_str());

                int idx = 0;
                for(auto om : def->outputMappings) {
                    int32_t finalvalue = om.startValue;

                    int cc = om.ctrl + trackBaseCC[t];
                    for(auto src : om.sources) {
                        int val = trackParameterValues[t][src.parameterIndex];
                        if (src.divider > 0) {
                            finalvalue += (val * src.multiplier) / src.divider;
                        } else {
                            finalvalue += val * src.multiplier;
                        }
                    }

                    // TODO: support NRPM

                    if (finalvalue < 0) finalvalue = 0;
                    if (finalvalue > 127) finalvalue = 127;

                    int midichannel = trackToMidiChannel[t];
                    // ESP_LOGI("MacroTranslator", "Track %d: (ch %d) setting synth parameter %d to %d (CC %d)",
                    //     t, midichannel, idx, finalvalue, cc);
                    if (cc != -1) {
                        soundProcessor->handleMidiControlChange(midichannel, cc, finalvalue);
                    }
                    idx ++;
                }
            }
        }
    }
}


void MacroTranslator::SerializeStateJSON(std::string *output) {
    Document d;

    d.SetObject();

    SerializeStateInto(d);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGW("MacroTranslator", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

bool MacroTranslator::SerializeStateInto(rapidjson::Document &doc) {
    Value tracksarray(kArrayType);
    doc.AddMember("tracks", tracksarray, doc.GetAllocator());
    for(int ti =0;ti<16;ti++) {
        Value trackjson(kObjectType);
        trackjson.AddMember("index", ti, doc.GetAllocator());
        trackjson.AddMember("machine", Value(trackMachineId[ti].c_str(), doc.GetAllocator()), doc.GetAllocator());
        if (definition[ti] != nullptr) {
            trackjson.AddMember("macro", Value(definition[ti]->id.c_str(), doc.GetAllocator()), doc.GetAllocator());
        } else {
            trackjson.AddMember("macro", "", doc.GetAllocator());
        }
        // trackjson.AddMember("preset", "", doc.GetAllocator());
        doc["tracks"].PushBack(trackjson, doc.GetAllocator());
    }
    return false;
}
