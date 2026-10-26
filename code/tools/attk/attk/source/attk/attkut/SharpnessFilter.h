#ifndef ATTK_SHARPNESS_FILTER_H
#define ATTK_SHARPNESS_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	class C_SharpnessFilter
	{
	public:
		FORCE_INLINE F32 Evaluate(U32 i, U32 j, U32 k) const
		{
			assert(i < 3);
			assert(j < 3);

			static F32 matrix[3][3] = { 
				{ 0, -1.f, 0}, 
				{ -1.f, 5.f, -1.f},
				{ 0, -1.f, 0} 
			};

			return 1.f * matrix[i][j];
		}

		FORCE_INLINE U32 GetNumFilterElements() const
		{
			return 9;
		}

		FORCE_INLINE U32 GetMatrixSize() const
		{
			return 3;
		}

		FORCE_INLINE F32 GetOutputOffset() const
		{
			return 0;
		}
	};
}

#endif
