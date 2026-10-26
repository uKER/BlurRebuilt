#ifndef _SPLINE_AI_I_H
#define _SPLINE_AI_I_H

#include <ComponentSystem/componentlib.h>

namespace airacingtech
{

	struct SplineInitialisationParams
	{
		Btl::Vector<Maths::HVec3>	m_controlPoints;
		Btl::Vector<float>			m_sampleSpeed;
		bool						m_resample;
	};



	BEGIN_INTERFACE_DEC(ISplineAI)

		virtual int GetLength() const = 0;
		virtual int GetSize() const = 0;
		virtual float GetInvLength() const = 0;
		virtual void SetInvLength(float invlength) = 0;
		virtual void SetLength(int length) = 0;
		virtual const Maths::HVec3& GetSamplePoint(int index) const = 0;
		virtual int GetNearestSamplePointIndex(Maths::HVec3 &pos) const = 0;
		virtual int GetNearestSamplePointIndexXZ(Maths::HVec3 &pos) const = 0;
		virtual Btl::Vector<Maths::HVec3> *GetSamplePoints() = 0;
		virtual Btl::Vector<Maths::HVec3> &GetSamplePointsRef() = 0;
		virtual const int GetSamplePointOffset(int start, int offset) const = 0;
		virtual const bool IsOpen() const = 0;
		virtual const Maths::HVec3& GetTangentNormal(int index) const = 0;
		virtual void Add(Maths::HVec3& samplePoint) = 0;
		virtual void SetOpenClosed(bool oc) = 0;
		virtual void AddTangent(Maths::HVec3& tangent) = 0;
		virtual Maths::HVec3 GetDirection(int index, int offset) const = 0;
	
	END_INTERFACE_DEC
	
}

#endif //_SPLINE_AI_I_H