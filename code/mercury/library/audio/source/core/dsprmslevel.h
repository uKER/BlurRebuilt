#ifndef _DSPRMSLEVEL_H_
#define _DSPRMSLEVEL_H_


namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
		    class RMSLevel : public Effect
		    {
		    public:
				enum ParamIn
				{
					eLength
				};

				enum ParamOut
				{
					eLevel,
					eLeftLevel,
					eRightLevel
				};

		    public:
				static bool bDisableAll;

		    public:
			    RMSLevel( const char* pchInfo, U16 uChannels );
			    ~RMSLevel() {}

                bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline void SetParam(ParamIn eParam, F32 fVal)
				{
					switch (eParam)
					{
					case eLength:
						m_fLength = Maths::Clamp(fVal, 0.01f, 1.f);
						break;
					}
				}

				inline F32 GetParam(ParamOut eParam)
				{
					switch (eParam)
					{
					case eLevel:
						return m_fLevel;

					case eLeftLevel:
						return m_fLeftLevel;
					
					case eRightLevel:
						return m_fRightLevel;

					default:
						return 0.f;
					}
				}

		    private:
				F32 m_fLevel;

				F32 m_fLeftLevel;
				F32 m_fRightLevel;

				F32 m_fMean[6];

				F32 m_fLength;

				F32 m_fPrevLength;

                F32 m_fA;
                F32 m_fB;

                F32 m_fPrevLevel[6];
			};
	    };
	};
};

#endif // _DSPRMSLEVEL_H_

