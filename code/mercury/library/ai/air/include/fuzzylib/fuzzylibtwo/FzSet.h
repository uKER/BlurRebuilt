#ifndef PROXY_FUZZY_SET
#define PROXY_FUZZY_SET

#ifdef _MSC_VER
#pragma once
#endif

#include <Fuzzylib/fuzzylibtwo/FuzzyTerm.h>
#include <Fuzzylib/fuzzylibtwo/FuzzySet.h>

namespace airacingtech
{
	namespace fuzzy_lib_two
	{
		class FzSet : public FuzzyTerm
		{

		private:

			FuzzySet& m_Set;

		public:

			FzSet(FuzzySet& fs):m_Set(fs){}

			FuzzyTerm* Clone()const{return new FzSet(*this);}
			inline float GetDOM()const {return m_Set.GetDOM();}
			inline void ClearDOM(){m_Set.ClearDOM();}
			inline void ORwithDOM(float val){m_Set.ORwithDOM(val);}

			
		};
	}
}


#endif

