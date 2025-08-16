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

class DrumRackFMB {
public:
    void Process(const DrumRackProcessData &data);
    void Init(const DrumRackInitData *initdata);
	bool enabled;
	float fmb_out[BUF_SZ];

private:
	CTAG::SYNTHESIS::FmKick fmb;
	bool fmb_trig_prev {false};
	atomic<int16_t> fmb_trigger, trig_fmb_trigger;
	atomic<int16_t> fmb_use_ratio_mode, trig_fmb_use_ratio_mode;
	atomic<int16_t> fmb_mod_env_sync, trig_fmb_mod_env_sync;
	atomic<int16_t> fmb_f_b, cv_fmb_f_b;
	atomic<int16_t> fmb_d_b, cv_fmb_d_b;
	atomic<int16_t> fmb_f_m, cv_fmb_f_m;
	atomic<int16_t> fmb_I, cv_fmb_I;
	atomic<int16_t> fmb_d_m, cv_fmb_d_m;
	atomic<int16_t> fmb_b_m, cv_fmb_b_m;
	atomic<int16_t> fmb_A_f, cv_fmb_A_f;
	atomic<int16_t> fmb_d_f, cv_fmb_d_f;
};
