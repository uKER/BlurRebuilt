#include "CorePch.h"
#include "core\utility\utility.h"

using namespace Ut;

void UtilityTests()
{
	// alignment
	class Test1
	{
		U32 x1;
		U32 x2;
		U32 x3;
	};

	int a0 = ALIGNMENT_OF(SameSizeAndAlignment<U8>);
	int a1 = ALIGNMENT_OF(SameSizeAndAlignment<U16>);
	int a2 = ALIGNMENT_OF(SameSizeAndAlignment<U32>);
	int a3 = ALIGNMENT_OF(SameSizeAndAlignment<Test1>);

	int s0 = sizeof(SameSizeAndAlignment<U8>);
	int s1 = sizeof(SameSizeAndAlignment<U16>);
	int s2 = sizeof(SameSizeAndAlignment<U32>);
	int s3 = sizeof(SameSizeAndAlignment<Test1>);
}