#pragma once

#include "DrumRackSynth.hpp"
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

class DrumRackClap : DrumRackSynth {
public:
    void Process(const DrumRackProcessData &data) override;
    void Init(const DrumRackInitData *initdata) override;

private:
	CTAG::SYNTHESIS::Clap cl;
	float cl_out[32];
	bool cl_trig_prev {false};
    atomic<int16_t> cl_trigger, trig_cl_trigger;
	atomic<int16_t> cl_mute, trig_cl_mute;
	atomic<int16_t> cl_lev, cv_cl_lev;
	atomic<int16_t> cl_pan, cv_cl_pan;
	atomic<int16_t> cl_fx1, cv_cl_fx1;
	atomic<int16_t> cl_fx2, cv_cl_fx2;
	atomic<int16_t> cl_f0, cv_cl_f0;
	atomic<int16_t> cl_tone, cv_cl_tone;
	atomic<int16_t> cl_decay, cv_cl_decay;
	atomic<int16_t> cl_scale, cv_cl_scale;
	atomic<int16_t> cl_transient, cv_cl_transient;
};
