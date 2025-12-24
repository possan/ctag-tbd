#include <atomic>
#include "ctagSoundProcessor.hpp"
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
#include "rack/RackDBD.hpp"
#include "rack/RackABD.hpp"
#include "rack/RackDSD.hpp"
#include "rack/RackASD.hpp"
#include "rack/RackHH1.hpp"
#include "rack/RackHH2.hpp"
#include "rack/RackFMB.hpp"
#include "rack/RackRimshot.hpp"
#include "rack/RackClap.hpp"
#include "rack/RackRompler.hpp"
#include "rack/RackTBD03.hpp"
#include "rack/RackPolyPad.hpp"
#include "rack/RackMO.hpp"
#include "rack/RackWTOsc.hpp"
#include "rack/RackInput.hpp"
#include "rack/RackFxDelay.hpp"
#include "rack/RackFxReverb.hpp"
#include "rack/RackFxMaster.hpp"
#include "rack/RackChannelMixer.hpp"






#define BUF_SZ 32

#define MK_BOOL_PAR_NOCV(outname, inname) \
    bool outname = inname;

#define MK_FLT_PAR_ABS_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_FLT_PAR_ABS_ADD_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_FLT_PAR_ABS_SFT_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_FLT_PAR_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_INT_PAR_ABS_NOCV(outname, inname, scale) \
    int outname = inname * scale / 4096;

#define MK_INT_PAR_NOCV(outname, inname, scale) \
    int outname = inname;

#define MK_FLT_PAR_ABS_MIN_MAX_NOCV(outname, inname, norm, out_min, out_max) \
    float outname = inname/norm * (out_max-out_min)+out_min;

#define MK_FLT_PAR_ABS_PAN_NOCV(outname, inname, norm, scale)  \
    float outname = (inname/norm+1.f)/2.f * scale;

#define CC_TO_MAP_KEY(ch, cc) (((ch) * 256) + (cc))





namespace CTAG {
    namespace SP {
        typedef void (DrumRackParameterSetter)(const int value);

		class ctagSoundProcessorPicoSeqRack : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            // no ctor, use Init() instead, is called from factory after successful creation
            virtual void Init(std::size_t blockSize, void *blockPtr) override;
            virtual ~ctagSoundProcessorPicoSeqRack();

	        void registerParam(const char *prefix, const char *suffix, function<DrumRackParameterSetter> setter);
			void registerParam(const PickSeqRackInitData *initdata, const char *suffix, function<DrumRackParameterSetter> setter);
			void registerParamAndCC(const PickSeqRackInitData *initdata, const char *suffix, int cc, function<DrumRackParameterSetter> setter);

			void parseIncomingMidiMessages(const uint8_t *buf, const size_t len);
            void handleMidiNoteOn(const uint8_t channel, const uint8_t note, const uint8_t vel);
            void handleMidiNoteOff(const uint8_t channel, const uint8_t note, const uint8_t vel);
            void handleMidiControlChange(const uint8_t channel, const uint8_t control, const uint8_t value);
            void handleMidiAftertouch(const uint8_t channel, const uint8_t note, const uint8_t vel);
            void handleMidiPatchChange(const uint8_t channel, const uint8_t patch);
            void handleMidiPitchBend(const uint8_t channel, const uint16_t bend);

        private:
            virtual void knowYourself() override;

            map<const int, string> pMapCC;

			// rack components
			RackChannelMixer ch1;
			RackDBD ch1_db;
			RackABD ch1_ab;

			RackChannelMixer ch2;
			RackFMB ch2_fmb1;
			RackFMB ch2_fmb2;

			RackChannelMixer ch3;
			RackDSD ch3_ds;
			RackASD ch3_as;

			RackChannelMixer ch4;
			RackHH1 ch4_hh1;
			RackHH2 ch4_hh2;

			RackChannelMixer ch5;
			RackRimshot ch5_rs;

			RackChannelMixer ch6;
			RackClap ch6_cl;

			RackRompler ch7_ro;
			RackChannelMixer ch7;

			RackRompler ch8_ro;
			RackChannelMixer ch8;

			RackTBD03 ch9_td3;
			RackChannelMixer ch9;

			RackTBD03 ch10_td3;
			RackChannelMixer ch10;

			RackMO ch11_mo;
			RackChannelMixer ch11;

			RackWTOsc ch12_wtosc;
			RackMO ch12_mo;
			RackChannelMixer ch12;

			RackChannelMixer ch13;
			RackRompler ch13_ro;

			RackChannelMixer ch14;
			RackRompler ch14_ro;

			RackChannelMixer ch15;
			RackPolyPad ch15_pp;

			RackInput ch16_in;
			RackChannelMixer ch16;

			// RackFxReverb fx_reverb;
			// RackFxDelay fx_delay;
			// RackFxMaster fx_master;

            // compressor
            chunkware_simple::SimpleComp sumCompressor;
            float fCompMUPGain_pre {0.f};
            float side_l {0.f};
            float side_r {0.f};

			void mixRenderOutputMono(float *source, float level, float pan, float fx1, float fx2);
			void mixRenderOutputStereo(float *source, float level, float pan, float fx1, float fx2);

			void preprocessFX1(const ProcessData& data);
			void preprocessFX2(const ProcessData& data);
			void preprocessMaster(const ProcessData& data);

			void renderMasterOutput(const ProcessData& data);

            // delay
            float *delayBuffer_l, *delayBuffer_r;
            const uint32_t delayBufferSizeMax {88200};
            uint32_t writeIndex {0};
            float readPos {0.0f}, readPosFiltered {0.0f};
            float delayOffset {0.0f};
            float duck {0.f};
            float delayTime_ms {0.0f};
            bool pre_sync {false};
            // float fDelayTime {0.0f};
            float delaySamples {32};
            float fSyncTimeStamp {0.0f};
            int32_t timer {0}, pre_timer {0};
            stmlib::OnePole lp_l, hp_l;
            stmlib::OnePole lp_r, hp_r;
			int last_scaledbpm { 1200 };
			float last_msPerBeat { 500.0f };

            // reverb
            float *reverbBuffer;
            mifx::Reverb reverb;
			int framecounter;

        	float combined_out[BUF_SZ*2];
        	float send1_out[BUF_SZ*2];
        	float send2_out[BUF_SZ*2];

            // romplers
            CTAG::SP::HELPERS::ctagSampleRom sampleRom;

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
	atomic<int32_t> fx1_time_ms; //, cv_fx1_time_ms;
	atomic<int32_t> fx1_sync; //, trig_fx1_sync;
	atomic<int32_t> fx1_freeze; //, trig_fx1_freeze;
	atomic<int32_t> fx1_tape_digital; //, trig_fx1_tape_digital;
	atomic<int32_t> fx1_st_width; //, cv_fx1_st_width;
	atomic<int32_t> fx1_fx_send; //, cv_fx1_fx_send;
	atomic<int32_t> fx1_feedback; //, cv_fx1_feedback;
	atomic<int32_t> fx1_base; //, cv_fx1_base;
	atomic<int32_t> fx1_width; //, cv_fx1_width;
	atomic<int32_t> fx2_time; //, cv_fx2_time;
	atomic<int32_t> fx2_lp; //, cv_fx2_lp;
	atomic<int32_t> c_thres; //, cv_c_thres;
	atomic<int32_t> c_ratio; //, cv_c_ratio;
	atomic<int32_t> c_atk; //, cv_c_atk;
	atomic<int32_t> c_rel; //, cv_c_rel;
	atomic<int32_t> c_lpf; //, trig_c_lpf;
	atomic<int32_t> c_gain; //, cv_c_gain;
	atomic<int32_t> c_mix; //, cv_c_mix;
	atomic<int32_t> c_dly_level; //, cv_c_dly_level;
	atomic<int32_t> c_rev_level; //, cv_c_rev_level;
	atomic<int32_t> sum_mute; //, trig_sum_mute;
	atomic<int32_t> sum_lev; //, cv_sum_lev;
	atomic<int32_t> fx1_amount; //, cv_fx1_amount;
	atomic<int32_t> fx2_amount; //, cv_fx2_amount;
	atomic<int32_t> global_bpm_hi;
	atomic<int32_t> global_bpm_lo;
	// sectionHpp
        };
    }
}

