#include "DrumRackSynth.hpp"
#include "DrumRackDBD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackDBD::Init(const DrumRackInitData *initdata) {
    dbd.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ db_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_db_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ db_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_db_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ db_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_db_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "tone", [&](const int val){ db_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "tone", [&](const int val){ cv_db_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ db_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_db_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "dirty", [&](const int val){ db_dirty = val;});
	initdata->rack->registerCv(initdata->prefix, "dirty", [&](const int val){ cv_db_dirty = val;});
	initdata->rack->registerParam(initdata->prefix, "fm_env", [&](const int val){ db_fm_env = val;});
	initdata->rack->registerCv(initdata->prefix, "fm_env", [&](const int val){ cv_db_fm_env = val;});
	initdata->rack->registerParam(initdata->prefix, "fm_dcy", [&](const int val){ db_fm_dcy = val;});
	initdata->rack->registerCv(initdata->prefix, "fm_dcy", [&](const int val){ cv_db_fm_dcy = val;});

    this->enabled = false;
}

void DrumRackDBD::Process(const DrumRackProcessData &data) {
    MK_BOOL_PAR(bDBTrig, db_trigger)
    if (bDBTrig != dbd_trig_prev){
        dbd_trig_prev = bDBTrig;
    }
    else{
        bDBTrig = false;
    }

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS(fDBAccent, db_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fDBF0, db_f0, 4095.f, 0.0005f, 0.01f)
    MK_FLT_PAR_ABS(fDBTone, db_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDBDecay, db_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDBDirty, db_dirty, 4095.f, 5.f)
    MK_FLT_PAR_ABS(fDBFmEnv, db_fm_env, 4095.f, 5.f)
    MK_FLT_PAR_ABS(fDBFmDcy, db_fm_dcy, 4095.f, 4.f)
    dbd.Render(
        false,
        bDBTrig,
        fDBAccent,
        fDBF0,
        fDBTone,
        fDBDecay,
        fDBDirty,
        fDBFmEnv,
        fDBFmDcy,
        dbd_out,
        32);
}
