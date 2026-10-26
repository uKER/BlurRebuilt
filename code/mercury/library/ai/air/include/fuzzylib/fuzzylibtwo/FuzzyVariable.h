#ifndef FLV_H
#define FLV_H


#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/CObjectId.h>
#include <Fuzzylib/fuzzylibtwo/fuzzyvariable.h>
#include <Fuzzylib/fuzzylibtwo/fuzzyoperators.h>
#include <Fuzzylib/fuzzylibtwo/FzSet.h>
#include <vector>
#include <map>


namespace airacingtech
{
	namespace fuzzy_lib_two
	{
		class FuzzySet;
		class FzSet;
		class FuzzyPolicy;

		class FuzzyVariable
		{
		private:  
			typedef std::map<CObjectId, FuzzySet*>  MemberSets;
		    
			FuzzyVariable(const FuzzyVariable&);
			FuzzyVariable& operator=(const FuzzyVariable&);

			MemberSets   m_MemberSets;
			float        m_dMinRange;
			float        m_dMaxRange;

			void AdjustRangeToFit(float minBound, float maxBound)
			{
				if (minBound < m_dMinRange) 
					m_dMinRange = minBound;
				if (maxBound > m_dMaxRange) 
					m_dMaxRange = maxBound;
			}

		public:

			~FuzzyVariable()
			{
				MemberSets::iterator it;
				for (it = m_MemberSets.begin(); it != m_MemberSets.end(); ++it)
				{
					delete it->second;
				}
			}

			FuzzyVariable():m_dMinRange(0.0),m_dMaxRange(0.0){}
		  
			

			FzSet AddTriangularSet(CObjectId name, float x0, float x1, float x2)
			{
				m_MemberSets[name] = new FuzzySetTriangle(x0,x1,x2);
				AdjustRangeToFit(x0, x2);
				return FzSet(*m_MemberSets[name]);
			}


			FzSet AddLeftShoulderSet(CObjectId name, float x0,float x1,float x2)
			{
				m_MemberSets[name] = new FuzzySetLeftShoulder(x0, x1, x2);
				AdjustRangeToFit(x0, x2);
				return FzSet(*m_MemberSets[name]);
			}

			FzSet AddRightShoulderSet(CObjectId name, float x0,float x1, float x2)
			{
				m_MemberSets[name] = new FuzzySetRightShoulder(x0, x1, x2);
				AdjustRangeToFit(x0, x2);
				return FzSet(*m_MemberSets[name]);
			}

			FzSet AddTrapezoidSet(CObjectId name,float x0,float x1,float x2,float x3)
			{
				m_MemberSets[name] = new FuzzySetTrapezoid(x0, x1, x2, x3);
				AdjustRangeToFit(x0, x3);
				return FzSet(*m_MemberSets[name]);
			}
		  
			inline void Fuzzify(float val)
			{    
				//assert ( (val >= m_dMinRange) && (val <= m_dMaxRange));
				MemberSets::const_iterator curSet;
				for (curSet = m_MemberSets.begin(); curSet != m_MemberSets.end(); ++curSet)
				{
				  curSet->second->SetDOM(curSet->second->CalculateDOM(val));
				}
			}
			inline float DeFuzzifyMaxAv()const
			{
				float bottom = 0.0f;
				float top    = 0.0f;

				MemberSets::const_iterator curSet;
				for (curSet = m_MemberSets.begin(); curSet != m_MemberSets.end(); ++curSet)
				{
					bottom += curSet->second->GetDOM();

					top += curSet->second->GetRepresentativeVal() * curSet->second->GetDOM();
				}

				
				if (bottom == 0.0f)
					return 0.0;

				return top / bottom;   
			}

		};
	}
}

#endif
