#ifndef FUZZY_OPERATORS_H
#define FUZZY_OPERATORS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Fuzzylib/fuzzylibtwo/FuzzyTerm.h>
#include <vector>

namespace airacingtech
{
	namespace fuzzy_lib_two
	{
		class FzAND : public FuzzyTerm
		{
		private:

			
			std::vector<FuzzyTerm*> m_Terms;

		
			FzAND& operator=(const FzAND&);

		public:

			

			~FzAND()
			{
				std::vector<FuzzyTerm*>::iterator curTerm;
				for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
				{
					delete *curTerm;
				}
			}

		 
			FzAND(const FzAND& fa)
			{
				std::vector<FuzzyTerm*>::const_iterator curTerm;
				for (curTerm = fa.m_Terms.begin(); curTerm != fa.m_Terms.end(); ++curTerm)
				{
					m_Terms.push_back((*curTerm)->Clone());
				}
			}


			FzAND(FuzzyTerm& op1, FuzzyTerm& op2)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
			}


			FzAND(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
				m_Terms.push_back(op3.Clone());
			}


			FzAND(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
				m_Terms.push_back(op3.Clone());
				m_Terms.push_back(op4.Clone());
			}

			FzAND(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4, FuzzyTerm& op5)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
				m_Terms.push_back(op3.Clone());
				m_Terms.push_back(op4.Clone());
				m_Terms.push_back(op5.Clone());
			}

			FzAND(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4, FuzzyTerm& op5, FuzzyTerm& op6)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
				m_Terms.push_back(op3.Clone());
				m_Terms.push_back(op4.Clone());
				m_Terms.push_back(op5.Clone());
				m_Terms.push_back(op6.Clone());
			}


			FuzzyTerm* Clone()const{return new FzAND(*this);}


			float GetDOM()const
			{
				float smallest = FLT_MAX;

				std::vector<FuzzyTerm*>::const_iterator curTerm;
				for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
				{
					smallest = Maths::Min(smallest, (*curTerm)->GetDOM());
				}

				return smallest;
			}



			void ORwithDOM(float val)
			{
				std::vector<FuzzyTerm*>::iterator curTerm;
				for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
				{
					(*curTerm)->ORwithDOM(val);
				}
			}


			void ClearDOM()
			{
				std::vector<FuzzyTerm*>::iterator curTerm;
				for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
				{
					(*curTerm)->ClearDOM();
				}
			}

		};



		class FzOR : public FuzzyTerm
		{
		private:
			std::vector<FuzzyTerm*> m_Terms;
			FzOR& operator=(const FzOR&);

		public:

			

			~FzOR()
			{
				std::vector<FuzzyTerm*>::iterator curTerm;
				for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
				{
					delete *curTerm;
				}
			}

		 
			FzOR(const FzOR& fa)
			{
				std::vector<FuzzyTerm*>::const_iterator curTerm;
				for (curTerm = fa.m_Terms.begin(); curTerm != fa.m_Terms.end(); ++curTerm)
				{
					m_Terms.push_back((*curTerm)->Clone());
				}
			}


			FzOR(FuzzyTerm& op1, FuzzyTerm& op2)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
			}


			FzOR(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
				m_Terms.push_back(op3.Clone());
			}


			FzOR(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
				m_Terms.push_back(op3.Clone());
				m_Terms.push_back(op4.Clone());
			}

			FzOR(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4, FuzzyTerm& op5)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
				m_Terms.push_back(op3.Clone());
				m_Terms.push_back(op4.Clone());
				m_Terms.push_back(op5.Clone());
			}


			FzOR(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4, FuzzyTerm& op5, FuzzyTerm& op6)
			{
				m_Terms.push_back(op1.Clone());
				m_Terms.push_back(op2.Clone());
				m_Terms.push_back(op3.Clone());
				m_Terms.push_back(op4.Clone());
				m_Terms.push_back(op5.Clone());
				m_Terms.push_back(op6.Clone());
			}


			FuzzyTerm* Clone()const{return new FzOR(*this);}

				float GetDOM()const
				{
				float largest = FLT_MIN;

				std::vector<FuzzyTerm*>::const_iterator curTerm;
				for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
				{
					largest = Maths::Max(largest, (*curTerm)->GetDOM());
				}

				return largest;
			}

			//unused
			void ClearDOM(){assert(0);}
			void ORwithDOM(float val){assert(0);}
		};

		class FzNOT : public FuzzyTerm
		{
		private:
			std::vector<FuzzyTerm*> m_Terms;
			FzNOT& operator=(const FzNOT&);

		public:

			

			~FzNOT()
			{
				std::vector<FuzzyTerm*>::iterator curTerm;
				for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
				{
					delete *curTerm;
				}
			}


			FzNOT(const FzNOT& fa)
			{
				std::vector<FuzzyTerm*>::const_iterator curTerm;
				for (curTerm = fa.m_Terms.begin(); curTerm != fa.m_Terms.end(); ++curTerm)
				{
					m_Terms.push_back((*curTerm)->Clone());
				}
			}

			FzNOT(FuzzyTerm& op1)
			{
				m_Terms.push_back(op1.Clone());
			}

			FuzzyTerm* Clone()const{return new FzNOT(*this);}

			float GetDOM()const
			{
				float largest = FLT_MIN;
				assert((int)m_Terms.size() == 1);

				std::vector<FuzzyTerm*>::const_iterator curTerm;
				for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
				{
					largest = 1.0f - (*curTerm)->GetDOM();
				}

				return largest;
			}

			//unused
			void ClearDOM(){assert(0);}
			void ORwithDOM(float val){assert(0);}
		};
	}
}



#endif

