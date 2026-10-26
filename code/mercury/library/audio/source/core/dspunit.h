#ifndef _DSPUNIT_H_
#define _DSPUNIT_H_

namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	DSPUnit
//	TODO: get rid, move to transformer
////////////////////////////////////////////////////////////////////////////////
		class DSPUnit
		{
		public:
			DSPUnit()
			{
				m_uNumInputs = 0;
				m_apInputs = NULL;
			}
			audio_virtual ~DSPUnit() {}

			bool Init(U32 uNumInputs);

			audio_virtual SetInput(DSPUnit* pInput, U32 uInput) = 0;

		protected:
			U32 m_uNumInputs;

			DSPUnit** m_apInputs;
		};
	};
};

#endif // _DSPUNIT_H_