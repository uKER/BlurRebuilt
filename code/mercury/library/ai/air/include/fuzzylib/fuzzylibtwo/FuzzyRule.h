#ifndef FUZZY_RULE_H
#define FUZZY_RULE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Fuzzylib/fuzzylibtwo/FuzzySet.h>
#include <Fuzzylib/fuzzylibtwo/FuzzyOperators.h>

namespace airacingtech
{
	namespace fuzzy_lib_two
	{
		class FuzzyRule
		{ 
		private:
			const FuzzyTerm*  m_pAntecedent;	
			FuzzyTerm*        m_pConsequence;
			FuzzyRule(const FuzzyRule&);
			FuzzyRule& operator=(const FuzzyRule&);

		public:

			FuzzyRule(const FuzzyTerm& ant,
					const FuzzyTerm& con):m_pAntecedent(ant.Clone()),
										  m_pConsequence(con.Clone())
			{}

			~FuzzyRule(){delete m_pAntecedent; delete m_pConsequence;}

			


			void SetConfidenceOfConsequentToZero(){m_pConsequence->ClearDOM();}

			void Calculate()
			{
				m_pConsequence->ORwithDOM(m_pAntecedent->GetDOM());
			}
		};
	}
}

#endif
