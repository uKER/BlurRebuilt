#ifndef ATTK_DEVIL_PLUGIN_H
#define ATTK_DEVIL_PLUGIN_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace Attk
{
	class C_DevILPlugin
	{
	public:
		virtual bool CanRead(const char* pFileName, C_Status* pStatus) const;

		virtual bool CanWrite(const char* pFileName, C_Status* pStatus) const;

		virtual C_Status ReadHeader(
			C_BaseImage& imageHeader,
			const char* pFileName);

		virtual C_Status Read(
			C_Image& image,
			const char* pFileName, 
			const C_Options* pOptions = 0,
			C_Progress* pProgress = 0);
		
		virtual C_Status Read(
			C_CubeMap& cubeMap,
			const char* pFileName, 
			const C_Options* pOptions = 0,
			C_Progress* pProgress = 0);

		virtual C_Status Write(
			const C_Image& image,
			const char* pFileName, 
			const C_Options* pOptions = 0,
			C_Progress* pProgress = 0);
	};
}

#endif

#endif
