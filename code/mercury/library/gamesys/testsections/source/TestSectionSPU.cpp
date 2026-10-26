#include "stdafx.h"
#include "TestSections/TestSectionSPU.h"
#include <Render/Render/GridRender.h>

namespace Framework
	{

	C_TestSectionSPU::C_TestSectionSPU(const Util::C_String& name) : C_TestSection(name)
	{
		Scheduler::Policy policy[] = {7, 7, 7, 7, 7, 7, 7, 7 };
		m_JobQueuePolicy	= Scheduler::CreatePolicy(policy);
		Scheduler::Init();
		Scheduler::Start();
		
		Scheduler::SetGroupPoolSize(128);
		m_JobQueueGroup = Scheduler::CreateGroup();
	}

	C_TestSectionSPU::~C_TestSectionSPU()
	{
		Scheduler::DestroyGroup(m_JobQueueGroup);
	}

	void C_TestSectionSPU::OnUpdate()
	{
		Scheduler::AddTask(SpuTest, 0, 0, m_JobQueueGroup, m_JobQueuePolicy, 0, 0);
		return;
	}
	
	void C_TestSectionSPU::OnRender3DViewWithAA(const S32 viewIndex)
	{
 		Render::DrawGrid();
		return;
	}

}; // namespace Framework
