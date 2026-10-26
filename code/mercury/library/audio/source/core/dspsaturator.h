#ifndef _DSPSATURATOR_H_
#define _DSPSATURATOR_H_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Saturator
////////////////////////////////////////////////////////////////////////////////
		    class Saturator : public Effect
		    {
		    public:
				enum ParamIn
				{
					eDrive,
					eClip,
					eLevel
				};

		    public:
				static bool bDisableAll;

		    public:
			    Saturator( const char* pchInfo, U16 uChannels );
			    ~Saturator() {}

			    bool Process(void* pFrameData, const ProcessInfo& info, bool bProcess);

				inline void SetParam(ParamIn eParam, F32 fVal)
				{
					switch (eParam)
					{
					case eDrive:
						m_fDrive = Maths::Max(fVal, 0.f);
						break;

					case eClip:
						m_fClip = Maths::Clamp(fVal, 0.f, 1.f);
						break;

					case eLevel:
						m_fLevel = fVal;
						break;
					}
				}

				inline void SetDriveParam( F32 fVal ) { m_fDrive = Maths::Max(fVal, 0.f); }

		   public: //private:
                F32 m_fDrive;
			    F32 m_fClip;
                F32 m_fLevel;

                F32 m_fPrevDrive;
			    F32 m_fPrevClip;
                F32 m_fPrevLevel;
			};
	    }
	}
}

#endif // _DSPSATURATOR_H_

