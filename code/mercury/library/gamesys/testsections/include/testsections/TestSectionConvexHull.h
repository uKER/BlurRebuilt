#ifndef TEST_CONVEXHULL_SECTION_H
#define TEST_CONVEXHULL_SECTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/debug/I_DebugRender.h>
#include <misc_shared/geometry/ConvexHull.h>
#include "TestSections/TestSection.h"

#define MAX_CONVEX_HULL_TEST_SOURCE_VERTICES 1000

namespace Framework
{

class C_TestSectionConvexHull : public C_TestSection
{
public:
	C_TestSectionConvexHull(const Util::C_String& name);
	~C_TestSectionConvexHull();
    
    void OnUpdate();    

	void OnRender3DViewWithoutAA(const S32 viewIndex);

	void WriteDebugVertexFile();
	void ReadDebugVertexFile();

	void Randomise();

	bool ShouldResetPoints();
	bool ShouldStepBuilding();
	bool ShouldRunAlgorithm();

private:

	S32							m_SourceVertexCount;
	ConvexHull::C_SourceVertex  m_SourceVertices[MAX_CONVEX_HULL_TEST_SOURCE_VERTICES];
	ConvexHull::C_Mesh*			m_pMesh;

	S32							m_Timeout;
	Maths::HMat43				m_WorldMatrix;
};

} // namespace Framework

#endif
