#ifndef DISPLAY_VIEW_VIEW_H
#define DISPLAY_VIEW_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/View/ViewTypes.h"
#include "Display/View/Viewport.h"
#include "Display/View/ViewSetup.h"
#include "Display/View/ViewFrustum.h"
#include "Display/View/ViewSettings.h"

namespace Display
{

//! View.

class C_View
{
public:

	C_View();
	C_View(const S32 index, const U32 type, const S32 userIndex);
	~C_View();

	const S32 GetIndex() const;
	const U32 GetType() const;
	const S32 GetUserIndex() const;
	const C_Viewport& GetViewport() const;
	const C_ViewFrustum& GetViewFrustum() const;
	const C_ViewSettings2D& GetViewSettings2D() const;
	const C_ViewSettings3D& GetViewSettings3D() const;

	void SetIndex(const S32 index);
	void SetType(const U32 type);
	void SetUserIndex(const S32 userIndex);
	void SetViewFrustum(const C_ViewFrustum& viewFrustum);
	
	void Setup(const C_ViewSetup& viewSetup, const U32 deviceWidth, const U32 deviceHeight);

	void SetPreviousFromCurrent();
	void CalculateProjectionMatrix(const C_ViewSetup3D& vs3d, Maths::HMat44& projection);
	void CalculateWorldToWindow(const C_ViewSetup& viewSetup, const Maths::HMat44& projection);

private:
	
	void SetViewport(const C_ViewSetup& viewSetup, const U32 deviceWidth, const U32 deviceHeight);
	void SetViewSettings2D(const C_ViewSetup& viewSetup, const U32 deviceWidth, const U32 deviceHeight);
	void SetViewSettings3D(const C_ViewSetup& viewSetup);

public:

	S32					m_Index;
	U32					m_Type;
	S32					m_UserIndex;
	C_Viewport			m_Viewport;
	C_ViewFrustum		m_ViewFrustum;
	C_ViewSettings2D	m_ViewSettings2D;
	C_ViewSettings3D	m_ViewSettings3D;
};

}; // namespace Display

#include "Display/View/View.inl"

#endif // DISPLAY_VIEW_VIEW_H
