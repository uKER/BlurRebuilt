#ifndef ATTK_IPLUGIN_H
#define ATTK_IPLUGIN_H

#ifdef _MSC_VER_
#pragma once
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace Attk
{
	class I_Plugin
	{
	public:
		virtual bool CanRead(const char* pFileName, C_Status* pStatus) const
		{
			if (pStatus)
				*pStatus = E_Status_Success;
			return false;
		}

		virtual bool CanWrite(const char* pFileName, C_Status* pStatus) const
		{
			if (pStatus)
				*pStatus = E_Status_Success;
			return false;
		}

		virtual C_Status ReadHeader(
			C_BaseImage& imageHeader,
			const char* pFileName)
		{
			return E_Status_NotImplemented;
		}

		virtual C_Status Read(
			C_Image& image,
			const char* pFileName, 
			const C_Options* pOptions = 0,
			C_Progress* pProgress = 0)
		{
			return E_Status_NotImplemented;
		}
		
		virtual C_Status Read(
			C_CubeMap& cubeMap,
			const char* pFileName, 
			const C_Options* pOptions = 0,
			C_Progress* pProgress = 0)
		{
			return E_Status_NotImplemented;
		}

		virtual C_Status Write(
			const C_Image& image,
			const char* pFileName, 
			const C_Options* pOptions = 0,
			C_Progress* pProgress = 0)
		{
			return E_Status_NotImplemented;
		}
	};
}

#endif

#endif
