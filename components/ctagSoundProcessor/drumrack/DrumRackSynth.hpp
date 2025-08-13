#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <atomic>

#include "../ctagSoundProcessor.hpp"


#define maxFXSendLevelDly 4.f
#define maxFXSendLevelRev 2.f
#define minVolume 0.000001f

using namespace std;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorDrumRack;
        
        typedef uint8_t *(DrumRackAllocator)(size_t size);

        struct DrumRackProcessData {
            float *buf;
            float *cv;
            uint8_t *trig;
        };

        struct DrumRackInitData {
            const char *prefix;
            DrumRackAllocator *allocator;
            ctagSoundProcessorDrumRack *rack;
            // RegisterCv *registerCv;
            // RegisterParam *registerParam;
            // RegisterTrig *registerTrig;
            // map<string, function<DrumRackParameterSetter>> *pMapPar;
            // map<string, function<DrumRackParameterSetter>> *pMapCv;
            // map<string, function<DrumRackParameterSetter>> *pMapTrig;
        };

        class DrumRackSynth {
        public:
            virtual void Process(const DrumRackProcessData &data) = 0;
            virtual void Init(const DrumRackInitData *initdata) = 0;

        protected:
            int const bufSz = 32;
        };
    }
}