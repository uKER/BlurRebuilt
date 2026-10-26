#include "stdafx.h"
#include "Main.h"

#include "Samples/Section/SectionEmpty.h"
#include "Samples/Section/SectionSkyDome.h"
#include "Samples/Section/SectionParticles.h"
#include "Samples/Section/SectionPostEffects.h"
#include "Samples/Section/SectionPostEffects.h"

#include "System/Serialise.h"
#include "Xt/XtGame.h"

XT_APPLICATION
	XT_DEPENDS_ON(RenderDynamicEffect)
	XT_DEPENDS_ON(RenderLightEffect)
	XT_DEPENDS_ON(RenderParticles)
	XT_DEPENDS_ON(RenderTracerEffect)
XT_END_APPLICATION

XtDb*				gXtDb;
XtPack*				gXtPack;
XtCallbackContext	gXtContext('P');

void OpenFlask()
{
	XtInitialise();

	Sys::I_File* f;

#if defined(PLATFORM_XENON)
	if (f = Sys::C_FileSystem::Instance().Open("game:\\DynamicEffect.bin"))
#elif defined(PLATFORM_PC)
	if (f = Sys::C_FileSystem::Instance().Open("DynamicEffect.bin"))
#elif defined(PLATFORM_PS3)
	if ((f = Sys::C_FileSystem::Instance().Open("DynamicEffect.bin")))
#endif
	{
		U32 length = (U32)f->Length();
		U8* data = new U8[length];

		if (data)
		{
			if (f->Read(data, length))
			{
				gXtDb	= XtDbCreate(XtGetApplicationSchema(), 2000, &gXtContext);
				gXtPack	= XtPackOpen(gXtDb, data, length);

				XtNetworkStart(gXtDb);
			}
		}

		delete data;
		delete f;
	}
}

void PreStaticInit();

namespace Samples
{

C_Sample::C_Sample()
{
	m_Name = "DynamicEffectTestApp";
}

C_Sample::~C_Sample()
{
}

bool C_Sample::MyCreate()
{
	// open Xt
	OpenFlask();

	// add sections
	AddSection(new C_SectionLauncher<C_SectionPostEffects>("PostEffects"));
	AddSection(new C_SectionLauncher<C_SectionEmpty>("Empty"));
	//AddSection(new C_SectionLauncher<C_SectionPostEffects>("PostEffects"));
	AddSection(new C_SectionLauncher<C_SectionSkyDome>("SkyDome"));
	AddSection(new C_SectionLauncher<C_SectionParticles>("Particles"));
	
	return true;
}

void C_Sample::MyDestroy()
{
	XtPackClose(gXtDb, gXtPack);
	XtDbDestroy(gXtDb);
}

void C_Sample::MyUpdate()
{
	XtNetworkUpdate();
}

}; // namespace Samples

int main(int argc, char *argv[])
{
	PreStaticInit();

	Samples::C_Sample sample;
	
	if (sample.Create())
	{
		sample.Run();
	}
	else
	{
		LOGFATAL(Samples::g_Log, "Failed to create the sample\n");
	}
}
