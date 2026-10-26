#ifndef MAYA_IO_MAIN_H
#define MAYA_IO_MAIN_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "../aftk/Ftk.h"

namespace MayaIo
{
	class C_Main : public Aftk::C_Ftk
	{
	public:
		Aftk::C_Status Read(
			const std::string& fileName, 
			Aftk::C_Scene *pAftkScene, 
			const char* pOptions = 0,
			Aftk::I_Progress* pProgressCallback = 0);

		Aftk::C_Status Write(
			const std::string &fileName, 
			U32 version, 
			const Aftk::C_Scene *pAftkScene, 
			const char* pOptions = 0,
			Aftk::I_Progress* pProgressCallback = 0);

		Aftk::C_Status ReadExternalScene(
			Aftk::C_Scene *pAftkScene,
			const char* pOptions = 0,
			Aftk::I_Progress* pProgressCallback = 0);

		Aftk::C_Status WriteExternalScene(
			const Aftk::C_Scene *pAftkScene,
			const char* pOptions = 0,
			Aftk::I_Progress* pProgressCallback = 0);

		void DisableAutoKey();

		void RestoreAutoKey();

		bool m_autoKey;
	};
}

#endif