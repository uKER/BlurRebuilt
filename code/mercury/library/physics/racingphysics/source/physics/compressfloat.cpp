#include "stdafx.h"
#include "racingphysics/physics/compressfloat.h"

namespace RacingPhysics
{

void CompressFloatTests()
{
	float f1 = 0.f;
	float f2 = 0.5f;
	float f3 = 1.01f;
	float f4 = -0.5f;

	float range = 1.01f;

	for (int i = 0; i < 3; i++)
	{
		{
			U32 c1 = CompressFloat<U32, 32>(f1, 0.f, range);
			U32 c2 = CompressFloat<U32, 32>(f2, 0.f, range);
			U32 c3 = CompressFloat<U32, 32>(f3, 0.f, range);
			float uf1 = UncompressFloat<U32, 32>(c1, 0.f, range);
			float uf2 = UncompressFloat<U32, 32>(c2, 0.f, range);
			float uf3 = UncompressFloat<U32, 32>(c3, 0.f, range);
			int breakhere = 0;
		}

		{
			U32 c1 = CompressFloat<U32, 16>(f1, 0.f, range);
			U32 c2 = CompressFloat<U32, 16>(f2, 0.f, range);
			U32 c3 = CompressFloat<U32, 16>(f3, 0.f, range);
			float uf1 = UncompressFloat<U32, 16>(c1, 0.f, range);
			float uf2 = UncompressFloat<U32, 16>(c2, 0.f, range);
			float uf3= UncompressFloat<U32, 16>(c3, 0.f, range);
			int breakhere = 0;
		}

		{
			U32 c1 = CompressFloat<U32, 8>(f1, 0.f, range);
			U32 c2 = CompressFloat<U32, 8>(f2, 0.f, range);
			U32 c3 = CompressFloat<U32, 8>(f3, 0.f, range);
			float uf1 = UncompressFloat<U32, 8>(c1, 0.f, range);
			float uf2 = UncompressFloat<U32, 8>(c2, 0.f, range);
			float uf3 = UncompressFloat<U32, 8>(c3, 0.f, range);
			int breakhere = 0;
		}

		{
			U32 c1 = CompressFloat<U32, 4>(f1, 0.f, range);
			U32 c2 = CompressFloat<U32, 4>(f2, 0.f, range);
			U32 c3 = CompressFloat<U32, 4>(f3, 0.f, range);
			float uf1 = UncompressFloat<U32, 4>(c1, 0.f, range);
			float uf2 = UncompressFloat<U32, 4>(c2, 0.f, range);
			float uf3 = UncompressFloat<U32, 4>(c3, 0.f, range);
			int breakhere = 0;
		}

		{
			U32 c1 = CompressFloat<U32, 32>(f1, -range, range);
			U32 c2 = CompressFloat<U32, 32>(f2, -range, range);
			U32 c3 = CompressFloat<U32, 32>(f3, -range, range);
			U32 c4 = CompressFloat<U32, 32>(f4, -range, range);
			float uf1 = UncompressFloat<U32, 32>(c1, -range, range);
			float uf2 = UncompressFloat<U32, 32>(c2, -range, range);
			float uf3 = UncompressFloat<U32, 32>(c3, -range, range);
			float uf4 = UncompressFloat<U32, 32>(c4, -range, range);
			int breakhere = 0;
		}

		{
			U32 c1 = CompressFloat<U32, 8>(f1, -range, range);
			U32 c2 = CompressFloat<U32, 8>(f2, -range, range);
			U32 c3 = CompressFloat<U32, 8>(f3, -range, range);
			U32 c4 = CompressFloat<U32, 8>(f4, -range, range);
			float uf1 = UncompressFloat<U32, 8>(c1, -range, range);
			float uf2 = UncompressFloat<U32, 8>(c2, -range, range);
			float uf3 = UncompressFloat<U32, 8>(c3, -range, range);
			float uf4 = UncompressFloat<U32, 8>(c4, -range, range);
			int breakhere = 0;
		}

		f1 *= 1000.f;
		f2 *= 1000.f;
		f3 *= 1000.f;
		f4 *= 1000.f;
		range *= 1000.f;
	}
}

}
