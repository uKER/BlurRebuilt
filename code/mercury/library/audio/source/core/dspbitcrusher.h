#ifndef _DSPBITCRUSHER_
#define _DSPBITCRUSHER_

namespace Audio
{
	namespace Core
	{
	    namespace DSP
	    {
////////////////////////////////////////////////////////////////////////////////
//	BitCrusher
////////////////////////////////////////////////////////////////////////////////
		    class BitCrusher : public Effect
		    {
		    public:
				enum ParamIn
				{
					eDepth,
					eLevel
				};

		    public:
				static bool bDisableAll;

		    public:
			    BitCrusher( const char* pchInfo, U16 uChannels );
			    ~BitCrusher() {}

			    bool Process( void* pFrameData, const ProcessInfo& info, bool bProcess );

				inline void SetParam( ParamIn eParam, F32 fVal )
				{
					switch (eParam)
					{
					case eDepth:
						m_fQuantize = Maths::Power( 2.f, Maths::Clamp( fVal, 0.f, 32.f ) );
						break;

					case eLevel:
						m_fLevel = fVal;
						break;
					}
				}

		   private:
                F32 m_fQuantize;
                F32 m_fLevel;
			};
	    }
	}
}

#endif // _DSPBITCRUSHER_

