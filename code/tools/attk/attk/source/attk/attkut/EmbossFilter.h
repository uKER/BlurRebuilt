#ifndef ATTK_EMBOSS_FILTER_H
#define ATTK_EMBOSS_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	class C_EmbossFilter
	{
	public:
		FORCE_INLINE F32 Evaluate(U32 i, U32 j, U32 k) const
		{
			assert(i < 3);
			assert(j < 3);

			static F32 matrix[3][3] = { 
				{ 2.f, 0, 0}, 
				{ 0, -1.f, 0},
				{ 0, 0, -1.f} 
			};

			//As the sum of our coefficients is zero we cannot divide by this sum
			//by way of a normalising factor. Instead we add 50% to the output value;

			return matrix[i][j];
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
			return 0.5f;
		}
	};
}

#endif
