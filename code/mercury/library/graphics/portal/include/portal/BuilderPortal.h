#ifndef PORTAL_BUILDERPORTAL_H
#define PORTAL_BUILDERPORTAL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Portal
{
	// Call these forward.
	class C_BuilderRoom;

	//! Portal-system portal builder object.
	class C_BuilderPortal
	{
	public:
		//! Specified c'tor, take pointer to containing room.
		//@param pParent Pointer to the parent room.
		explicit C_BuilderPortal(C_BuilderRoom* pParent);
		//! D'tor.
		~C_BuilderPortal();

		//! Create a new polygon in the portal.
		//@return Pointer to the newly created vertex.
		Maths::HVec3*			CreateNewPolygonVertex();
		//! Return the counterpart.
		//@return Pointer to the counterpart portal.
		const C_BuilderPortal*	GetCounterpart() const;
		//! Return a pointer to the parent room.
		//@return Pointer to the parent room.
		const C_BuilderRoom*	GetParent() const;
		//! Return the plane.
		//@return Planar for the plane.
		const Maths::C_Plane&	GetPlane() const;
		//! Return the number of vertices in the portal's polygon.
		//@return The number of vertices in the portal.
		const U32				GetVertexCount() const;
		//! Return the vertex at specified index.
		//@param iIndex Return the vertex at a specified index.
		//@return World-space vertex.
		const Maths::HVec3&		GetVertexAtIndex(U32 iIndex) const;
		//! Set the portals counter part.
		//@param pCounterpart Pointer to another portal.
		void					SetCounterpart(C_BuilderPortal* pCounterpart);
		//! Set the plane.
		//@param rkPlane Plane that this portal lays in.
		void					SetPlane(const Maths::C_Plane& rkPlane);

	private:
		Maths::C_Plane					m_Plane;			//!< Plane the portal's polygon lies in.
		C_BuilderPortal*				m_pCounterpart;		//!< Portal's counterpart -- where it links to.
		C_BuilderRoom*					m_pParent;			//!< Parent portal.
		Btl::Vector<Maths::HVec3>		m_vVerts;			//!< Verts for the portal.
	
		/// Default c'tor.
		explicit C_BuilderPortal();
		
	};
};

#endif // PORTAL_BUILDERPORTAL_H
