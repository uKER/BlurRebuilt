#ifndef _DSPPEAKLEVEL_H_
#define _DSPPEAKLEVEL_H_


namespace Audio

{
	//const int PeakLevel_DELAY_BUFFER_LENGTH = 64;
	//const int FX_MAX_CHANNELS = 6;
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	PeakLevel
////////////////////////////////////////////////////////////////////////////////
		    class PeakLevel : public Effect
		    {
		    public:
				enum ParamOut
				{
					eLevel
				};

		    public:
				static bool bDisableAll;

		    public:
			    PeakLevel( const char* pchInfo, U16 uChannels );
			    ~PeakLevel() {}

                bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline F32 GetParam(ParamOut eParam)
				{
					switch (eParam)
					{
					case eLevel:
						return m_fLevel;

					default:
						return 0.f;
					}
				}

		    private:
  				F32 m_fLevel;
            };
	    };
	};
};

#endif // _DSPPEAKLEVEL_H_

