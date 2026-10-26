#ifndef SAMPLES_SECTION_MATHS_TEST_H
#define SAMPLES_SECTION_MATHS_TEST_H

#ifdef _MSC_VER
#pragma once
#endif

#if !defined(PLATFORM_SPU)
#include "TestSections/TestSection.h"
#endif 

#include <System/Scheduler/jobqueue.h>

#if !defined(PLATFORM_SPU)

namespace Framework
{

class C_TestSectionSPU : public C_TestSection
{
public:

    C_TestSectionSPU( const Util::C_String& name );
    ~C_TestSectionSPU();
    
    virtual void OnUpdate( /* const Maths::C_TimeS& timeStep */ ) override;
    virtual void OnRender3DViewWithAA( const S32 viewIndex ) override;
    
private:
	Scheduler::GroupHandle		m_JobQueueGroup;
	Scheduler::PolicyHandle		m_JobQueuePolicy;
};

}; // namespace Samples

#endif

extern Scheduler::jqModule SpuTest;

#endif // SAMPLES_SECTION_MATHS_TEST_H
