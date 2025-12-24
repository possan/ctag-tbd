#pragma once

#include "RackSynth.hpp"
#include "plaits/dsp/drums/analog_bass_drum.h"
#include "plaits/dsp/drums/analog_snare_drum.h"
#include "plaits/dsp/drums/synthetic_bass_drum.h"
#include "plaits/dsp/drums/synthetic_snare_drum.h"
#include "plaits/dsp/drums/hi_hat.h"
#include "braids/analog_oscillator.h"
#include "braids/signature_waveshaper.h"
#include "braids/macro_oscillator.h"
#include "braids/settings.h"
#include "braids/quantizer.h"
#include "filters/ctagDiodeLadderFilter.hpp"
#include "filters/ctagDiodeLadderFilter2.hpp"
#include "filters/ctagDiodeLadderFilter3.hpp"
#include "filters/ctagDiodeLadderFilter4.hpp"
#include "filters/ctagDiodeLadderFilter5.hpp"
#include "filters/ctagFilterBase.hpp"
#include "synthesis/RomplerVoiceMinimal.hpp"
#include "synthesis/Clap.hpp"
#include "synthesis/Rimshot.hpp"
#include "synthesis/FmKick.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagADEnv.hpp"
#include "SimpleComp/SimpleComp.h"
#include "mifx/reverb.h"
#include "polypad/ChordSynth.hpp"

using namespace CTAG::SP;

class RackInput {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    float in_out[BUF_SZ * 2];
 
	// void handleMidiCC(uint8_t control, uint8_t value);

private:
    atomic<int16_t> in_mute, trig_in_mute;
	atomic<int16_t> in_lev, cv_in_lev;
	atomic<int16_t> in_pan, cv_in_pan;
	atomic<int16_t> in_fx1, cv_in_fx1;
	atomic<int16_t> in_fx2, cv_in_fx2;
};
