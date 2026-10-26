#ifndef ATTK_BOX_FILTER_H
#define ATTK_BOX_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	class C_BoxFilter
	{
	public:
		FORCE_INLINE F32 Evaluate(U32 i, U32 j, U32 k) const
		{
			assert(i < GetMatrixSize());
			assert(j < GetMatrixSize());

			//Return 1 / the sum of our matrix coefficients (3x3 box matrix therefore would be 9)
	//		return 1.f / 9.f;
			return 1.f;
		}

		FORCE_INLINE U32 GetNumFilterElements() const
		{
	//		return 9;
			return 1;
		}

		FORCE_INLINE U32 GetMatrixSize() const
		{
	//		return 3;
			return 1;
		}

		FORCE_INLINE F32 GetOutputOffset() const
		{
			return 0;
		}
	};
}

#endif
