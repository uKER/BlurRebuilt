#ifndef MATHS_HVEC_MATHS_TEST_H
#define MATHS_HVEC_MATHS_TEST_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

#if defined( USER_JDENTON )

enum E_MathsTest
{
    E_MathsTest_Verify = 0,
    E_MathsTest_Create = 1
};

void Test( const E_MathsTest type );

#endif // #if defined( USER_JDENTON )

}; // namespace Maths


#endif // #ifndef MATHS_HVEC_MATHS_TEST_H

