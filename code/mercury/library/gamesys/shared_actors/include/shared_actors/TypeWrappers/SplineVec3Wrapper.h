#ifndef SHARED_ACTORS_SPLINEVEC3_WRAPPER
#define SHARED_ACTORS_SPLINEVEC3_WRAPPER

#ifdef _MSC_VER
#pragma once
#endif

struct XtSplineVec3;

namespace DbUtil
{
	class I_DebugRender;
}

namespace Curve
{
	class C_ActorCurve3D;
	class C_Hermite3DActorDataProxy;
	class C_Hermite3DActorProxy;
}

namespace SharedActors
{
	class C_SplineVec3Wrapper
	{
	public:
		C_SplineVec3Wrapper();
		~C_SplineVec3Wrapper();

		bool Initialise(const XtSplineVec3* pXtData, const Maths::HMat43& createMat);

		void DebugRender(DbUtil::I_DebugRender* pDebugRenderPtr) const;

		//Please don't use this for new code - See Ken for details.
		void GetForwardFacingSplinePos_UseCalcUHack(
			Maths::C_QuatTrans& quatTrans,
			F32 progress, 
			F32 speed, 
			F32 deltaTime) const;

		//Please don't use this for new code - See Ken for details.
		F32	FollowerFindNearestPoint_UseCalcUHack(
			const Maths::C_Vertex& position, 
			U32 closestKey, 
			Maths::HVec3 &closestPos, 
			Maths::HVec3 &tangent) const;

		//Please don't use this for new code - See Ken for details.
		F32	FollowerGetArcDistance_UseCalcUHack(
			const Maths::C_Vertex& position, 
			U32 closestKey) const;

		F32 GetForwardFacingSplinePos(
			Maths::C_QuatTrans& quatTrans,
			F32 progress, 
			F32 offset,
			F32 speed, 
			F32 deltaTime,
			bool wrap) const;
			
		F32 GetForwardFacingSplinePos(
			Maths::C_QuatTrans& quatTrans,
			F32 arcProgress) const;			

		F32 GetSplinePos(
			Maths::HVec3& trans,
			F32 progress, 
			F32 offset,
			F32 speed,
			F32 deltaTime) const;

		S32	FollowerFindNearestKnotIndex(
			const Maths::C_Vertex& position, 
			U32 closestKey) const;

		S32	FollowerFindStartKnotIndex(
			const Maths::C_Vertex& position) const;

		const Curve::C_ActorCurve3D&	
			GetCurveEval() const {return *m_pCurveEval;}

		Curve::C_Hermite3DActorDataProxy&
			GetCurveData() const {return *m_pCurveData;}

		bool IsClosed() const {return m_isClosed;}

	private:
		Curve::C_Hermite3DActorDataProxy*				m_pCurveData;
		Curve::C_ActorCurve3D*							m_pCurveEval;
		bool											m_isClosed;
	};
}

#endif
