#ifndef _DSPSEND_H_
#define _DSPSEND_H_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	Gain
////////////////////////////////////////////////////////////////////////////////
		    class Send : public Effect
		    {
		    public:
				enum ParamIn
				{
					eLevel
				};

		    public:
				static bool bDisableAll;

		    public:
			    Send( const char* pchInfo, U16 uChannels );
			    ~Send() {}

			    bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch( eParam )
					{
					case eLevel:
						m_fLevel = fVal;
						break;
					}
				}

				void SetReturn( U32 uReturn ) { m_uReturn = uReturn; }

		    private:
                F32 m_fLevel;
			    F32 m_fPrevLevel;

				U32 m_uReturn;
		    };
	    }
	}
}

#endif // _DSPSEND_H_

