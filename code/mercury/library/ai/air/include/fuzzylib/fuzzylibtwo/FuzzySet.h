#ifndef FUZZYSET_H
#define FUZZYSET_H

#ifdef _MSC_VER
#pragma once
#endif

namespace airacingtech
{
	namespace fuzzy_lib_two
	{

		class FuzzySet
		{
		protected: 
			float        m_dDOM;
			float        m_dRepresentativeValue;

		public:

			FuzzySet(float RepVal):m_dDOM(0.0), m_dRepresentativeValue(RepVal){}

		 
			virtual float CalculateDOM(float val)const = 0;
			void ORwithDOM(float val){if (val > m_dDOM) m_dDOM = val;}
			float GetRepresentativeVal()const{return m_dRepresentativeValue;}

			void ClearDOM(){m_dDOM = 0.0;}  
			float GetDOM()const{return m_dDOM;}
			void SetDOM(float val)
			{
				assert ((val <=1) && (val >= 0));
				m_dDOM = val;
			}

			
		};

		class FuzzySetRightShoulder : public FuzzySet
		{
		private:

			
			float   m_x0;
			float   m_x1;
			float	m_x2;

		public:

			FuzzySetRightShoulder(float x0, float x1, float x2):FuzzySet( ((x2 - x1) / 2.0f) + x1),
																m_x0(x0),
																m_x1(x1),
																m_x2(x2)
			{}

			

			inline float CalculateDOM(float val)const
			{
				float result = 0.0f;
				if(val <= m_x0)
				{
					result = 0.0f;
				}
				else if(val >= m_x1)
				{
					result = 1.0f;
				}
				else
				{
					float denom = m_x1 - m_x0;
					result = (val/denom)-(m_x0/denom);
				}

				return result;	
			}
		};

		class FuzzySetLeftShoulder : public FuzzySet
		{
		private:

			
			float   m_x0;
			float   m_x1;
			float	m_x2;

		public:

			FuzzySetLeftShoulder(float x0, float x1, float x2): FuzzySet( ((x1 - x0) / 2.0f) + x0),
															m_x0(x0),
															m_x1(x1),
															m_x2(x2)
			{}

			

			
			inline float CalculateDOM(float val)const
			{
			  float result = 0.0f;

			  if(val <= m_x1)
			  {
				  result = 1.0f;
			  }
			  else if(val >= m_x2)
			  {
				  result = 0.0f;
			  }
			  else
			  {
				  float denom = m_x2 - m_x1;
				  result = (-val/denom)+(m_x2/denom);
			  }

			  return result;
			}
		};

		class FuzzySetTriangle : public FuzzySet
		{
		private:

			
			float   m_x0;
			float   m_x1;
			float	m_x2;

		public:

			FuzzySetTriangle(float x0, float x1, float x2): FuzzySet( x1),
															m_x0(x0),
															m_x1(x1),
															m_x2(x2)
			{}

			
			

			inline float CalculateDOM(float val)const
			{
				
				float result = 0.0f;

				if((val <= m_x0) || (val >= m_x2))
				{
					result = 0.0f;
				}
				else if(val == m_x1)
				{
					result = 1.0f;
				}
				else if((val>m_x0) && (val<m_x1))
				{
					float denom = m_x1 - m_x0;
					result = (val/denom)-(m_x0/denom);
				}
				else
				{
					float denom = m_x2 - m_x1;
					result = (-val/denom)+(m_x2/denom);
				}
				return result;	
			}
		};


		class FuzzySetTrapezoid : public FuzzySet
		{
		private:
			float   m_x0;
			float   m_x1;
			float	m_x2;
			float	m_x3;

		public:

			FuzzySetTrapezoid(float x0, float x1, float x2, float x3):FuzzySet( ((x2 - x1) / 2.0f) + x1),
																		m_x0(x0),
																		m_x1(x1),
																		m_x2(x2),
																		m_x3(x3)
			{}

			

			inline float CalculateDOM(float val)const
			{
				float result = 0.0f;
				
				if ((val <= m_x0) || (val >= m_x3))
				{
					result = 0.0f;
				}
				else if((val >= m_x1) && (val <= m_x2))
				{
					result = 1.0f;
				}
				else if((val > m_x0) && (val < m_x1))
				{
					float denom = m_x1 - m_x0;
					result = (val/denom)-(m_x0/denom);
				}
				else
				{
					float denom = m_x3 - m_x2;
					result = (-val/denom)+(m_x3/denom);
				}
				return result;	
			}
		};

	}
}

#endif
