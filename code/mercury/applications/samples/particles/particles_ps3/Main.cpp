#include "stdafx.h"
#include "Main.h"

#include "Samples/Section/SectionEmpty.h"
#include "Samples/Section/SectionSkyDome.h"
//#include "Samples/Section/SectionParticles.h"
//#include "Samples/Section/SectionPostEffects.h"

void PreStaticInit();

namespace Samples
{

C_Sample::C_Sample()
{
	m_Name = "ParticlesTestApp";
}

C_Sample::~C_Sample()
{
}

bool C_Sample::MyCreate()
{
	AddSection(new C_SectionLauncher<C_SectionEmpty>("Empty"));
	//AddSection(new C_SectionLauncher<C_SectionPostEffects>("PostEffects"));
	AddSection(new C_SectionLauncher<C_SectionSkyDome>("SkyDome"));
	//AddSection(new C_SectionLauncher<C_SectionParticles>("Particles"));
	
	return true;
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
