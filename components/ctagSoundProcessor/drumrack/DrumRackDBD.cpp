#include "DrumRackSynth.hpp"
#include "DrumRackDBD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackDBD::Init(const DrumRackInitData *initdata) {
    dbd.Init();

    initdata->rack->registerParam(initdata->prefix, "db_trigger", [&](const int val){ db_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "db_trigger", [&](const int val){ trig_db_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "db_mute", [&](const int val){ db_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "db_mute", [&](const int val){ trig_db_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "db_lev", [&](const int val){ db_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "db_lev", [&](const int val){ cv_db_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "db_pan", [&](const int val){ db_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "db_pan", [&](const int val){ cv_db_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "db_fx1", [&](const int val){ db_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "db_fx1", [&](const int val){ cv_db_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "db_fx2", [&](const int val){ db_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "db_fx2", [&](const int val){ cv_db_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "db_accent", [&](const int val){ db_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "db_accent", [&](const int val){ cv_db_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "db_f0", [&](const int val){ db_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "db_f0", [&](const int val){ cv_db_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "db_tone", [&](const int val){ db_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "db_tone", [&](const int val){ cv_db_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "db_decay", [&](const int val){ db_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "db_decay", [&](const int val){ cv_db_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "db_dirty", [&](const int val){ db_dirty = val;});
	initdata->rack->registerCv(initdata->prefix, "db_dirty", [&](const int val){ cv_db_dirty = val;});
	initdata->rack->registerParam(initdata->prefix, "db_fm_env", [&](const int val){ db_fm_env = val;});
	initdata->rack->registerCv(initdata->prefix, "db_fm_env", [&](const int val){ cv_db_fm_env = val;});
	initdata->rack->registerParam(initdata->prefix, "db_fm_dcy", [&](const int val){ db_fm_dcy = val;});
	initdata->rack->registerCv(initdata->prefix, "db_fm_dcy", [&](const int val){ cv_db_fm_dcy = val;});

}

// void DrumRackDBD::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackDBD", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackDBD::Process(const DrumRackProcessData &data) {
	 
MK_BOOL_PAR(bDBMute, db_mute)
    MK_BOOL_PAR(bDBTrig, db_trigger)
    if (bDBTrig != dbd_trig_prev){
        dbd_trig_prev = bDBTrig;
    }
    else{
        bDBTrig = false;
    }

    MK_FLT_PAR_ABS_PAN(fDBPan, db_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDBLev, db_lev, 4095.f, 2.f); fDBLev *= fDBLev;
    MK_FLT_PAR_ABS(fDBFX1Send, db_fx1, 4095.f, maxFXSendLevelDly); fDBFX1Send *= fDBFX1Send;
    MK_FLT_PAR_ABS(fDBFX2Send, db_fx2, 4095.f, maxFXSendLevelRev); fDBFX2Send *= fDBFX2Send;

    if (bDBMute || fDBLev < minVolume) {
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

    // mixRenderOutputMono(dbd_out, fDBLev, fDBPan, fDBFX1Send, fDBFX2Send);
    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
