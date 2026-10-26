#ifndef PORTAL_TRACERESULTS_H
#define PORTAL_TRACERESULTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/Configuration.h>


namespace Portal
{
	//! Trace results objects.
	class ALIGN16 C_TraceResults
	{
	public:
		//! C'tor.
		__forceinline explicit C_TraceResults();
		//! D'tor.
 		__forceinline ~C_TraceResults();

		//! Return the distance along the ray at which a collision occured.
		//@return The distance along the ray that an intersection occured. Only valid if hit is true. 
		__forceinline NOALIAS const Maths::HF32			GetDistance() const __restrict;
		//! Return the end point of the ray.
		//@return World-space end point of the ray.
		__forceinline NOALIAS const Maths::C_Vertex&	GetEndPoint() const __restrict;
		//! Return the room index of the end point of the ray.
		//@return The index of the room that the end of the ray resides in.
		__forceinline NOALIAS const U32					GetFinalRoomIndex() const __restrict;
		//! Return the world-space origin of the ray.
		//@return The world-space origin.
		__forceinline NOALIAS const Maths::C_Vertex&	GetOrigin() const __restrict;
		//! Return the world normal at the point of intersection.
		//@return The world-space normal at the point of intersection.
		__forceinline NOALIAS const Maths::C_Normal&	GetWorldNormal() const __restrict;
		//! Return the world position at the point of intersection.
		//@return The world-space position at the point of intersection. 
		__forceinline NOALIAS const Maths::C_Vertex&	GetWorldPosition() const __restrict;
		//! Return true if ray has hit something.
		//@return Returns true if the ray intersected the portal system's room geometry.
		__forceinline NOALIAS const bool				HasHit() const __restrict;
		//! Set the distance along the ray at which point the collision occured.
		//@param fDistance The distance along the ray that an intersection occured.
		__forceinline void								SetDistance(const Maths::HF32 fDistance);
		//! Set the end point of the ray.
		//@param rkEndPoint World-space end point of the ray.
		__forceinline void								SetEndPoint(const Maths::C_Vertex& rkEndPoint);
		//! Set the final room index.
		//@param The index of the room for the end point of the ray.
		__forceinline void								SetFinalRoomIndex(const U32 iRoomIndex);
		//! Set the hit flag.
		//@param Boolean value indicating if the ray intersected the portal system's room geometry.
		__forceinline void								SetHasHit(const bool bHasHit);
		//! Set the origin of the ray.
		//@param rkEndPoint World-space origin of the ray.
		__forceinline void								SetOrigin(const Maths::C_Vertex& rkEndPoint);
		//! Set the world normal at the point of itnersection.
		//@param The world-space normal at the point of intersection.
		__forceinline void								SetWorldNormal(const Maths::C_Normal& rkNormal);
		//! Set the world position at the point of intersection.
		//@param The world-space position at the point of intersection.
		__forceinline void								SetWorldPosition(const Maths::C_Vertex& rkVertex);
		
	private:
		Maths::C_Normal		m_WorldNormal;				//!< World normal of the collision.
		Maths::C_Vertex		m_Origin;					//!< World-space origin of the ray.
		Maths::C_Vertex		m_EndPoint;					//!< World-space end point of the ray.
		Maths::C_Vertex		m_WorldPos;					//!< World position of the collision.
		Maths::HF32			m_fDistance;				//!< Distance along direction vector that the collision occured.
		U32					m_iFinalRoom;				//!< Final room.
		bool				m_bHit;						//!< Set to true if a collision occured.
	};

	// Include source code for inlined methods.
	#include "TraceResults.inl"
}

#endif // PORTAL_TRACERESULTS_H
