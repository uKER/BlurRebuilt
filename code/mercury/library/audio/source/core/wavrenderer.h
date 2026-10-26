#ifndef _WAVRENDERER_H_
#define _WAVRENDERER_H_

//#include "windows.h"

namespace Audio
{
	namespace Core
	{
		class Generator;

////////////////////////////////////////////////////////////////////////////////
//	WavRenderer
////////////////////////////////////////////////////////////////////////////////
        class WavRenderer
		{
		public:
			WavRenderer()
			{
//                m_hFile = INVALID_HANDLE_VALUE;
                m_pOutputBuffer = NULL;
			}
			audio_virtual ~WavRenderer() {}

			bool RenderToFile(char* pchFilename, Audio::Core::Generator* pInput, F32 fLength);

		private:
			//HANDLE m_hFile;
            //Audio::File m_File;

            S16* m_pOutputBuffer;
		};
	};
};

#endif // _WAVRENDERER_H_

