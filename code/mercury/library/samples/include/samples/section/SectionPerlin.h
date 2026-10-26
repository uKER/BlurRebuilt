#ifndef SAMPLES_SECTIONPERLIN_H
#define SAMPLES_SECTIONPERLIN_H

#ifdef _MSC_VER
#pragma once
#endif



#include "Samples/Section/Section.h"
#include "Render/Effects/Perlin/Perlin.h"

namespace Samples
{



class C_SectionPerlin : public C_Section
{
public:

	C_SectionPerlin(const Util::C_String& name);
	~C_SectionPerlin();

	void	OnUpdate(const C_Input& input, const C_Timer& timer);
	void	OnRender(const C_Input& input, const C_Timer& timer);

private:

	Perlin::C_Perlin	m_Perlin;

};



}; // namespace Samples



#endif // SAMPLES_SECTIONPERLIN_H
