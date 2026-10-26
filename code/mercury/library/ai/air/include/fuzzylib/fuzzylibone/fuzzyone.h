#ifndef _FUZZY_LIB_ONE
#define _FUZZY_LIB_ONE

#ifdef _MSC_VER
#pragma once
#endif

namespace airacingtech
{
	namespace fuzzy_lib_one
	{
		/*#define FUZZY_MIN(x, y) (x<y) ? x:y
		#define FUZZY_MAX(x, y) (x>y) ? x:y;*/
		#define FUZZY_MIN(x, y) Maths::Min(x, y)
		#define FUZZY_MAX(x, y) Maths::Max(x, y);

		inline Maths::HF32 FuzzyRightShoulder(const Maths::HF32 PCREF value, const Maths::HF32 PCREF x0, 
			const Maths::HF32 PCREF x1)
		{
			/*float result = 0.0f;
			float x = value;

			if(x <= x0)
			{
				result = 0.0f;
			}
			else if(x >= x1)
			{
				result = 1.0f;
			}
			else
			{
				float denom = x1 - x0;
				result = (x/denom)-(x0/denom);
			}

			return result;*/

			const Maths::HF32 denom(x1 - x0);
			return Maths::Select(value.LE(x0), Maths::Constants<Maths::HF32>::Zero(),
				Maths::Select(value.GE(x1), Maths::Constants<Maths::HF32>::One(),
				(value - x0)/denom));
		}
		

		inline Maths::HF32 FuzzyLeftShoulder(const Maths::HF32 PCREF value, const Maths::HF32 PCREF x0, const Maths::HF32 PCREF x1)
		{
			/*float result = 0.0f;
			float x = value;

			if(x <= x0)
			{
				result = 1.0f;
			}
			else if(x >= x1)
			{
				result = 0.0f;
			}
			else
			{
				float denom = x1 - x0;
				result = (-x/denom)+(x1/denom);
			}

			return result;*/
			const Maths::HF32 denom(x1 - x0);
			return Maths::Select(value.LE(x0), Maths::Constants<Maths::HF32>::One(),
				Maths::Select(value.GE(x1), Maths::Constants<Maths::HF32>::Zero(),
				(x1 - value)/denom));
		}

		inline Maths::HF32 FuzzyTriangle(const Maths::HF32 PCREF value, const Maths::HF32 PCREF x0, const Maths::HF32 PCREF x1, const Maths::HF32 PCREF x2)
		{
			/*float result = 0.0f;
			float x = value;

			if(x <= x0)
			{
				result = 0.0f;
			}
			else if(x == x1)
			{
				result = 1.0f;
			}
			else if((x>x0) && (x<x1))
			{
				float denom = x1 - x0;
				result = (x/(x1-x0))-(x0/(x1-x0));
			}
			else
			{
				float denom = x2 - x1;
				result = (-x/denom)+(x2/denom);
			}
			return result;*/

			const Maths::HF32 denom1(x1 - x0);
			const Maths::HF32 denom2(x2 - x1);
			const Maths::Scalar_Bool bTest(value.GT(x0) & value.LT(x1));
			const Maths::HF32 denom(Maths::Select(bTest, denom1, denom2));
			const Maths::HF32 numerator(Maths::Select(bTest, value - x0, x2 - value));
			return Maths::Select(value.LE(x0), Maths::Constants<Maths::HF32>::Zero(),
				Maths::Select(value.EQ(x1), Maths::Constants<Maths::HF32>::One(),
				numerator/denom));
		}

		inline Maths::HF32 FuzzyTrapezoid(const Maths::HF32 PCREF value, const Maths::HF32 PCREF x0, const Maths::HF32 PCREF x1, const Maths::HF32 PCREF x2, const Maths::HF32 PCREF x3)
		{
			/*float result = 0.0f;
			float x = value;

			if(x <= x0)
			{
				result = 0.0f;
			}
			else if((x>=x1) && (x<=x2))
			{
				result = 1.0f;
			}
			else if((x>x0) && (x<x1))
			{
				float denom = x1 - x0;
				result = (x/denom)-(x0/denom);
			}
			else
			{
				float denom = x3 - x2;
				result = (-x/denom)+(x3/denom);
			}
			return result;*/
			const Maths::HF32 denom1(x1 - x0);
			const Maths::HF32 denom2(x3 - x2);
			const Maths::Scalar_Bool bTest(value.GT(x0) & value.LT(x1));
			const Maths::HF32 denom(Maths::Select(bTest, denom1, denom2));
			const Maths::HF32 numerator(Maths::Select(bTest, value - x0, x3 - value));
			return Maths::Select(value.LE(x0), Maths::Constants<Maths::HF32>::Zero(),
				Maths::Select(value.GE(x1) & value.LE(x2), Maths::Constants<Maths::HF32>::One(),
				numerator/denom));
		}

		inline Maths::HF32 FuzzyAND(const Maths::HF32 PCREF A, const Maths::HF32 PCREF B)
		{
			return FUZZY_MIN(A, B);
		}

		inline Maths::HF32 FuzzyOR(const Maths::HF32 PCREF A, const Maths::HF32 PCREF B)
		{
			return FUZZY_MAX(A, B);
		}

		inline Maths::HF32 FuzzyNOT(const Maths::HF32 PCREF A)
		{
			return Maths::Constants<Maths::HF32>::One() - A;
		}
	}
}


#endif  // _FUZZY_LIB_ONE
