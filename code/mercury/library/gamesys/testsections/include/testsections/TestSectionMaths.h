#ifndef SAMPLES_SECTION_MATHS_TEST_H
#define SAMPLES_SECTION_MATHS_TEST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace Framework
{

class C_TestSectionMaths : public C_TestSection
{
public:

    C_TestSectionMaths( const Util::C_String& name );
    ~C_TestSectionMaths();
    
    virtual void OnUpdate( /* const Maths::C_TimeS& timeStep */ ) override;
    virtual void OnRender3DViewWithAA( const S32 viewIndex ) override;
    
private:
    
    Maths::VF32             m_Time;
    Maths::Mat43            m_Matrix[40];
    Maths::Quaternion       m_Quaternion[30];
    Maths::QuatTrans        m_QuatTrans[4];
    Maths::DualQuaternion   m_DualQuaternion[4];
    Maths::Point3           m_Point[9];
    Maths::Line             m_Line[4];
    Maths::Plane            m_Plane[2];
    Maths::Sphere           m_Sphere[2];
    Maths::Triangle         m_Triangle[2];
    Maths::AxisBox          m_AxisBox[2];
};

}; // namespace Samples

#endif // SAMPLES_SECTION_MATHS_TEST_H
