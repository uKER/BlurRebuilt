#ifndef ATTK_BARTLETT_FILTER_H
#define ATTK_BARTLETT_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	class C_BartlettFilter
	{
	public:
		FORCE_INLINE F32 Evaluate(U32 i, U32 j, U32 k) const
		{
			assert(i < 5);
			assert(j < 5);

			static F32 matrix[5][5] = { 
				{ 1, 2, 3, 2, 1}, 
				{ 2, 4, 6, 4, 2},
				{ 3, 6, 9, 6, 3}, 
				{ 2, 4, 6, 4, 2},
				{ 1, 2, 3, 2, 1}
			};

			//As the sum of our coefficients is zero we cannot divide by this sum
			//by way of a normalising factor. Instead we add 50% to the output value;

			return (1.f/81.f) *matrix[i][j];
		}

		FORCE_INLINE U32 GetNumFilterElements() const
		{
			return 25;
		}

		FORCE_INLINE U32 GetMatrixSize() const
		{
			return 5;
		}

		FORCE_INLINE F32 GetOutputOffset() const
		{
			return 0;
		}
	};
}

#endif
