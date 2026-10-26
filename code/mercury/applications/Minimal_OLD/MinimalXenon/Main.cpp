#include "StdAfx.h"
#include "Main.h"
#include "Samples/Section/SectionEmpty.h"
#include "Samples/Section/SectionSkyDome.h"

namespace Samples
{

C_Sample::C_Sample()
{
}

C_Sample::~C_Sample()
{
}

bool C_Sample::MyCreate()
{
	AddSection(new C_SectionLauncher<C_SectionSkyDome>("SkyDome"));
	AddSection(new C_SectionLauncher<C_SectionEmpty>("Empty"));

	//m_pSection->EnterSection();

	return true;
}

}; // namespace Samples

void __cdecl main()
{
	Samples::C_Sample sample;
	
	sample.ReadConfig("Config.ini");

	if(sample.Create())
	{
		sample.Run();
	}
	else
	{
		LOGFATAL(Samples::g_Log, "Failed to create the sample\n");
	}
}
