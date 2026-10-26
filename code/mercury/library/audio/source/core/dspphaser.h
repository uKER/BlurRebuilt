#ifndef _DSPPHASER_H_
#define _DSPPHASER_H_


namespace Audio

{
	const int PHASER_DELAY_BUFFER_LENGTH = 64;
	const int FX_MAX_CHANNELS = 6;
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Phaser
////////////////////////////////////////////////////////////////////////////////
			class Phaser : public Effect
		    {
		    public:
				enum ParamIn
				{
					eDry,
					eWet,
					eAngle,
					eSpeed,
					eWidth,
					eFeedback
				};

		    public:
				static bool bDisableAll;

		    public:
			    Phaser( const char* pchInfo, U16 uChannels );
			    ~Phaser() {}

                bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline void SetParam(ParamIn eParam, F32 fVal)
				{
					switch (eParam)
					{
					case eDry:
						m_fDry = fVal;
						break;

					case eWet:
						m_fWet = fVal;
						break;

					case eAngle:
						m_fAngle = fVal;
						break;

					case eSpeed:
						m_fSpeed = fVal;
						break;

					case eWidth:
						m_fWidth = fVal;
						break;

					case eFeedback:
						m_fFeedback = fVal;
						break;
					}
				}

		    private:
				F32 m_fDry;
				F32 m_fWet;
				F32 m_fAngle;
				F32 m_fSpeed;
				F32 m_fWidth;
				F32 m_fFeedback;

				U32 m_uCurrOffset;

				F32 m_afDelayBuffer[FX_MAX_CHANNELS][PHASER_DELAY_BUFFER_LENGTH];
            };
	    };
	};
};

#endif // _DSPPHASER_H_

