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

class DrumRackDBD : DrumRackSynth {
public:
    void Process(const DrumRackProcessData &data) override;
    void Init(const DrumRackInitData *initdata) override;

private:
	float dbd_out[32];
	plaits::SyntheticBassDrum dbd;
	bool dbd_trig_prev {false};
	atomic<int32_t> db_trigger, trig_db_trigger;
	atomic<int32_t> db_mute, trig_db_mute;
	atomic<int32_t> db_lev, cv_db_lev;
	atomic<int32_t> db_pan, cv_db_pan;
	atomic<int32_t> db_fx1, cv_db_fx1;
	atomic<int32_t> db_fx2, cv_db_fx2;
	atomic<int32_t> db_accent, cv_db_accent;
	atomic<int32_t> db_f0, cv_db_f0;
	atomic<int32_t> db_tone, cv_db_tone;
	atomic<int32_t> db_decay, cv_db_decay;
	atomic<int32_t> db_dirty, cv_db_dirty;
	atomic<int32_t> db_fm_env, cv_db_fm_env;
	atomic<int32_t> db_fm_dcy, cv_db_fm_dcy;
};
