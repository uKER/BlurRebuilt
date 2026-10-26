#ifndef FUZZY_POLICY_H
#define FUZZY_POLICY_H



#ifdef _MSC_VER
#pragma once
#endif

#include <Fuzzylib/fuzzylibtwo/FuzzySet.h>
#include <Fuzzylib/fuzzylibtwo/FuzzyVariable.h>
#include <Fuzzylib/fuzzylibtwo/FuzzyRule.h>
#include <Fuzzylib/fuzzylibtwo/FuzzyOperators.h>
#include <Fuzzylib/fuzzylibtwo/FzSet.h>
#include <ComponentSystem/CObjectId.h>
#include <vector>
#include <map>


namespace airacingtech
{
	namespace fuzzy_lib_two
	{
		class FuzzyPolicy
		{
		private:

			typedef std::map<CObjectId, FuzzyVariable*> VarMap;
			VarMap                   m_Variables;
			std::vector<FuzzyRule*>   m_Rules;

			inline void SetConfidencesOfConsequentsToZero()
			{
				std::vector<FuzzyRule*>::iterator curRule = m_Rules.begin();
				for (curRule; curRule != m_Rules.end(); ++curRule)
				{
					(*curRule)->SetConfidenceOfConsequentToZero();
				}
			}

		public:

			

			~FuzzyPolicy()
			{
				VarMap::iterator curVar = m_Variables.begin();
				for (curVar; curVar != m_Variables.end(); ++curVar)
				{
					delete curVar->second;
				}

				std::vector<FuzzyRule*>::iterator curRule = m_Rules.begin();
				for (curRule; curRule != m_Rules.end(); ++curRule)
				{
					delete *curRule;
				}
			}

			
			FuzzyVariable& CreateFLV(const CObjectId& VarName)
			{
				m_Variables[VarName] = new FuzzyVariable();

				return *m_Variables[VarName];
			}


			
			void AddRule(FuzzyTerm& antecedent, FuzzyTerm& consequence)
			{
				m_Rules.push_back(new FuzzyRule(antecedent, consequence));
			}

			
			inline void Fuzzify(const CObjectId& NameOfFLV, float val)
			{
				assert ( (m_Variables.find(NameOfFLV) != m_Variables.end()));
				m_Variables[NameOfFLV]->Fuzzify(val);
			}

			inline float DeFuzzify(const CObjectId& key)
			{
				
				assert ( (m_Variables.find(key) != m_Variables.end()));
				SetConfidencesOfConsequentsToZero();

				std::vector<FuzzyRule*>::iterator curRule = m_Rules.begin();
				for (curRule; curRule != m_Rules.end(); ++curRule)
				{
					(*curRule)->Calculate();
				}

				return m_Variables[key]->DeFuzzifyMaxAv();
			}
		};
	}
}



#endif //FUZZY_POLICY_H
