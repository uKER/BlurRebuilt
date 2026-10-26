#ifndef XSI_IO_MAIN_H
#define XSI_IO_MAIN_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_Main : public Aftk::C_Ftk
	{
	public:
		C_Main() {}

		~C_Main() {}

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

	private:
	};
}

#endif
