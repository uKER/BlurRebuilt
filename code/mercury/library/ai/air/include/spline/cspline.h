#ifndef _SPLINE_AI_C_H
#define _SPLINE_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <spline/ispline.h>


namespace airacingtech
{

	BEGIN_COMPONENT_DEC(CSplineAI, ISplineAI)
			
		void Add(Maths::HVec3& samplePoint)
		{
			m_SamplePoints.PushBack(samplePoint);
			SetLength((int)m_SamplePoints.Size());
			SetInvLength((float)(1.0f / m_Length));
		}

		inline int GetLength() const {return m_Length;}
		inline int GetSize() const {return (int) m_SamplePoints.Size();}
		inline float GetInvLength() const {return m_InvLength;}
		
		void SetSamplePoints(const Btl::Vector<Maths::HVec3>& spoints)
		{
			m_SamplePoints = spoints;
			SetLength((int)m_SamplePoints.Size());
			SetInvLength((float)(1.0f / m_Length));
		}

		void SetSamplePoints(const Btl::Vector<Maths::HVec3> *spoints)
		{
			m_SamplePoints = *spoints;
			SetLength((int)m_SamplePoints.Size());
			SetInvLength((float)(1.0f / m_Length));
		}

		

		void SetTangentNormal(const Btl::Vector<Maths::HVec3> *spoints)
		{
			m_TangentNormal = *spoints;
		}

		void SetTangentNormal(const Btl::Vector<Maths::HVec3> &spoints)
		{
			m_TangentNormal = spoints;
		}

		void AddTangent(Maths::HVec3& tangent)
		{
			m_TangentNormal.PushBack(tangent);
		}


		inline const Maths::HVec3& GetTangentNormal(int index) const
		{
			assert(index >= 0 && index < m_TangentNormal.Size());	
			return m_TangentNormal[index];
		}

		inline void SetLength(int length) {m_Length = length;}
		inline void SetInvLength(float invlength){m_InvLength = invlength;}
		inline void SetOpenClosed(bool oc) {m_Open = oc;}

		inline const Maths::HVec3& GetSamplePoint(int index) const
		{
			assert(index >= 0 && index < GetSize());	
			return m_SamplePoints[index];
		}

		inline const int GetSamplePointOffset(int start, int offset) const 
		{


			assert(start >= 0 && start < GetSize());
			int result = -1;

			if (IsOpen())
			{
				result = start + offset;
				if (result >= GetSize())
					result -= 1;

				if (result < 0)
					result = 0;

			}
			else
			{
				result = start + offset;

				if (result >= 0)
				{
					do 
					{
						if (result >= GetSize())
							result -= GetSize();

					} while(result > GetSize());
				}
				else if (result < 0)
				{
					do 
					{
						if (result < 0)
							result += GetSize();

					} while(result < 0 );
				}
			}

			assert(result >= 0 && result < GetSize());

			return result;
		}	

		int GetNearestSamplePointIndex(Maths::HVec3 &pos) const
		{
			float maxDist =  FLT_MAX;
			int result = -1;
			int s = GetSize();
			for(int i=0; i<s; i++)
			{
				Maths::HVec3 point1 = GetSamplePoint(i);

				float dist = (pos - point1 ).LengthSquared();

				if (dist < maxDist)
				{
					maxDist = dist;
					result = i;
				}
			}
			return result;
		};

		int GetNearestSamplePointIndexXZ(Maths::HVec3 &pos) const
		{
			float maxDist =  FLT_MAX;
			int result = -1;
			int s = GetSize();

			Maths::HVec3 position(pos.X(), 0.0f, pos.Z());

			for(int i=0; i<s; i++)
			{
				Maths::HVec3 point1 = GetSamplePoint(i);
				Maths::HVec3 point(point1.X(), 0.0f, point1.Z());

				float dist = (position - point ).LengthSquared();

				if (dist < maxDist)
				{
					maxDist = dist;
					result = i;
				}
			}
			return result;
		};

		Btl::Vector<Maths::HVec3> *GetSamplePoints()
		{
			return &m_SamplePoints;
		}

		Btl::Vector<Maths::HVec3> &GetSamplePointsRef()
		{
			return m_SamplePoints;
		}

		inline const bool IsOpen() const {return m_Open;}


		inline Maths::HVec3 GetDirection(int index, int offset) const
		{
			assert(index >= 0 && index < GetSize());
			
			Maths::HVec3 point1 = GetSamplePoint(index);
			int next = GetSamplePointOffset(index, offset);
			Maths::HVec3 point2 = GetSamplePoint(next);

			Maths::HVec3 direction = point2 - point1;
			direction.Normalise();

			return direction;
		}

		void JoinLooped();

	protected:
		Btl::Vector<Maths::HVec3>	m_SamplePoints;
		Btl::Vector<Maths::HVec3>	m_TangentNormal;
		int								m_Length;
		float							m_InvLength;
		bool							m_Open;


	END_COMPONENT_DEC
}




#endif //_SPLINE_AI_C_H