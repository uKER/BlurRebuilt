#ifndef IMPOSTER_IMPOSTERPROCESS_H
#define IMPOSTER_IMPOSTERPROCESS_H


#ifdef _MSC_VER
#pragma once
#endif

#if defined(IMPOSTER_DLL_EXPORT)
	#define IMPOSTER_DECL __declspec(dllexport)
#elif defined(IMPOSTER_DLL_IMPORT)
	#define IMPOSTER_DECL __declspec(dllimport)
#else
	#define IMPOSTER_DECL
#endif

namespace Aftk
{



}

namespace AftkRender
{
	class C_Scene;
	class C_ViewContext;
	class C_Application;
}

namespace Imposter
{
	IMPOSTER_DECL bool ProcessAftk(
		Aftk::C_Scene& aftkScene, 
		const Btl::String& workingFolder, 
		bool useTextureAtlas=false,
		Aftk::C_Scene *pAftkBaseScene=0, 
		Progress::I_ProgressBar* pProgress=0);
};

#endif