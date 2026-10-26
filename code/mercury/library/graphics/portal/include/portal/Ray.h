#ifndef PORTAL_RAY_H
#define PORTAL_RAY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/Configuration.h>
#include <Portal/TraceResults.h>

#ifndef PLATFORM_SPU
// Call these forward:
namespace DbUtil
{
	class I_DebugRender;
	typedef I_DebugRender* I_DebugRenderPtr;
};
#endif // !PLATFORM_SPU

namespace Portal
{
	// Call these forward:
	class I_DeferredTracerFunctorBase;
	class C_System;
	class C_RegionData;
	class C_BSPContainer;
	class C_DeferredTracer;
	
	//! A single ray to trace through the portal system.
	class C_Ray
	{
	public:
		//! Default c'tor -- unitialised ray.
		explicit C_Ray();
		//! C'tor.
		//@param rkOrigin The world-space origin of the ray.
		//@param rkDirection Normalised direction vector for the ray.
		//@param fMaxDistance The maximum distance of the ray.
		__forceinline explicit C_Ray(const Maths::C_Vertex& rkOrigin, 
			const Maths::HVec3& rkDirection,		// Normalised direction vector.
			const Maths::HF32 fMaxDistance);
		//! C'tor.
		//@param rkOrigin The world-space origin of the ray.
		//@param rkEndPoint The world-space end point of the ray.
		__forceinline explicit C_Ray(const Maths::C_Vertex& rkOrigin,
			const Maths::C_Vertex& rkEndPoint);
		//! C'tor.
		//@param rkOrigin The world-space origin of the ray.
		//@param rkWorldVector the world-space vector of the ray.
		__forceinline explicit C_Ray(const Maths::C_Vertex& rkOrigin,
			const Maths::HVec3& rkWorldVector);
		//! D'tor.
		__forceinline ~C_Ray();

#ifndef PLATFORM_SPU
		//! Debug render the ray.
		//@param pDebugRender Pointer to an instance of the debug renderer.
		//@param rkColour The colour the ray should be rendered.
		void							DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::RGBA& rkColour) const;
#endif // !PLATFORM_SPU
		//! Return the world-space vector from origin to end point.
		//@return World-space vector from origin to end point.
		__forceinline NOALIAS const Maths::HVec3&		GetDirection() const __restrict;
		//! Return the world-space end point of the ray.
		//@return World-space end point of the ray.
		__forceinline NOALIAS const Maths::C_Vertex&	GetEndPoint() const __restrict;
		//! Magnitude of the vector from origin to the end point.
		//@return The length of the ray.
		__forceinline NOALIAS const Maths::HF32			GetLength() const __restrict;
		//! Return the world-space origin of the ray.
		//@return The world-space origin of the ray.
		__forceinline NOALIAS const Maths::C_Vertex&	GetOrigin() const __restrict;
		//! Return the solid region of the ray.
		//@return The "solid region" of the ray.
		__forceinline NOALIAS const U32					GetSolidRegion() const __restrict;
		//! Trace the ray immediately.
		//@param pContainer Pointer to the tracer object.
		//@return Results of the trace.
		C_TraceResults									Trace(C_BSPContainer* pContainer) const;
		//! Deferred trace of the ray.
		//@param pContainer Pointer to the tracer object.
		//@param pFunctor Pointer to function object base class that will be called when deferred ray trace is complete.
		void											TraceDeferred(C_DeferredTracer* pTracer, I_DeferredTracerFunctorBase* pFunctor) const;

	private:
		Maths::C_Vertex		m_Origin;				//!< World space starting point of the ray.
		Maths::C_Vertex		m_EndPoint;				//!< World space ending point of the ray.
		Maths::HVec3		m_Direction;			//!< World space direction vector of the ray.
		Maths::HF32			m_fDistance;			//!< Magnitude of vector from origin to end point.
		U32					m_iSolidRegion;			//!< The solid region of the ray.
	};

	// Include implementation of inline methods.
	#include "Ray.inl"
}

#endif // PORTAL_RAY_H
