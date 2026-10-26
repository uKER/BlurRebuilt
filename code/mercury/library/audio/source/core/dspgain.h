#ifndef _DSPGAIN_H_
#define _DSPGAIN_H_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Gain
////////////////////////////////////////////////////////////////////////////////
		    class Gain : public Effect
		    {
		    public:
				enum ParamIn
				{
					eGain
				};

		    public:
				static bool bDisableAll;

		    public:
			    Gain( const char* pchInfo, U16 uChannels );
			    ~Gain() {}

			    bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline void SetParam(ParamIn eParam, F32 fVal)
				{
					switch (eParam)
					{
					case eGain:
						m_fGain = fVal;
						break;
					}
				}

		    private:
                F32 m_fGain;
			    F32 m_fPrevGain;
		    };
	    }
	}
}

#endif // _DSPGAIN_H_

