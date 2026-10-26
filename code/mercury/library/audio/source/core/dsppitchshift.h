

#ifndef _DSPPitchShift_H_
#define _DSPPitchShift_H_



#include "../SoundTouch/SoundTouch.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)	// AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

using namespace soundtouch;

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	PitchShift
////////////////////////////////////////////////////////////////////////////////
		    class PitchShift : public Effect
		    {
		    public:
				enum ParamIn
				{
					ePitch,
					eSequenceLength,
					eSeekWindowLength,
					eOverlapLength
				};

				enum StateIn
				{
					eActive
				};

				enum ParamOut
				{
 
				};

		    public:
				static bool bDisableAll;

		    public:
#ifndef PLATFORM_SPU
			    PitchShift(const char* pchInfo,U32 u32NumChannels);
			    ~PitchShift() {}
#endif

			    bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline void SetParam(ParamIn eParam, F32 fVal)
				{
					switch (eParam)
					{
					case ePitch:
						m_fPitch = fVal;
						break;

					case eSequenceLength:
						stSoundTouch.setSetting(SETTING_SEQUENCE_MS, (uint)fVal);
						break;

					case eSeekWindowLength:
						stSoundTouch.setSetting(SETTING_SEEKWINDOW_MS, (uint)fVal);
						break;

					case eOverlapLength:
						stSoundTouch.setSetting(SETTING_OVERLAP_MS, (uint)fVal);
						break;
					}
				}

				inline void SetState(StateIn eState, U32 uVal)
				{
					switch (eState)
					{
					case eActive:
						m_bActive = (uVal != 0) ? true : false;
						break;
					}
				}

				inline F32 GetParam(ParamOut eParam)
				{
					// jkc- added to stop warnings on ps3 compile (function body was empty)
					return 0.f;
				}


		    private:
				bool m_bActive;
				
				SoundTouch stSoundTouch;
				
				int m_idelayCounter;
				int m_bufferUnderrunDelay;
				float m_fPitch;
				float m_fPrevPitch;

			
		    };
	    }
	}
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


#endif // _DSPPitchShift_H_

