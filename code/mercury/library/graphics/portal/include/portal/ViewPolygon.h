#ifndef PORTAL_VIEWPOLYGON_H
#define PORTAL_VIEWPOLYGON_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/Configuration.h>

namespace Portal
{
	class C_Portal;

	//! A view polygon.
	class C_ViewPolygon
	{
	public:
		//! Default c'tor.
		explicit C_ViewPolygon();
		//! D'tor.
		~C_ViewPolygon();

		//! Add a new vertex to the polygon.
		//@param rkPos Vertex to add to the view polygon.
		void				AddVertex(const Maths::HVec3& rkPos);
		//! Clip the polygon with specified plane.
		//@param rkResult Resulting view polygon from the clipping.
		//@param rkPlane Plane to clip the view polygon against.
		void				Clip(C_ViewPolygon& rkResult, const Maths::C_Plane& rkPlane) const;
		//! Clip the polygon with specified portal.
		//@param rkResult Resulting view polygon from the clipping.
		//@param pPortal Portal to clip against.
		//@param rkCameraPos Position of the camera.
		void				Clip(C_ViewPolygon& rkResult, const C_Portal* pPortal,  const Maths::HVec3& rkCameraPos) const;
		//! Compute the screen-space area of the polygon.
		//@param rkViewProj The view-projection matrix.
		//@return The approximate screen-space area of the polygon.
		const Maths::HF32	GetScreenSpaceArea(const Maths::HMat44& rkViewProj) const;
		//! Debug render the polygon.
		//@param pDebugRender Pointer to debug renderer.
		//@param rkColour Colour to draw the view polygon.
		//@param bSolidPoly [Optional] Set to true if view polygon should be rendered as a solid polygon.
		void				DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::RGBA& rkColour, bool bSolidPoly = false) const;
		//! Return the number of vertices in the polygon.
		//@return The number of verticies in the view polygon.
		const U32			GetVertexCount() const;
		//! Reset the polygon -- clears underlying storage.
		void				Reset();		
		/// Assignment operator.
		//@param rkCopy The view polygon to copy.
	    void				operator= (const C_ViewPolygon& rkCopy);

	private:
		Btl::Vector<Maths::HVec3, Btl::VectorParameters_Static<PORTAL_CONFIG_MAXPOLYVERTS> > m_vVertexData;	//!< Vertex data for polygon.
	};
};

#endif // PORTAL_VIEWPOLYGON_H
