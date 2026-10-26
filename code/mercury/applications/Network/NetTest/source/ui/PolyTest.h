#ifndef POLYTEST_H
#define POLYTEST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Graphics/Simple2d.h"
#include "ui/Graphics/Polygons/UIPolygonClasses.h"

#include <btl/Containers/containers.h>

#if defined( USER_WILLB )
#define _ADOBE_TEST_
#endif

namespace UI
{

// ----------
// C_PolyTest
// ----------
class C_PolyTest
{
public:
	C_PolyTest();
	~C_PolyTest();

public:
	void		Init();
	void		Cleanup();
	void		Recalculate();
	void		Render();
	void		Update();


private:
	void		ResetInput();

private:
	Btl::Vector< C_UIClipPolygon* >		m_Input;
	Btl::Vector< C_UIClipPolygon* >		m_Result[ E_UIPolyClip_Count ];

#ifdef _ADOBE_TEST_
	C_UITriListRenderInfo										m_RenderInfo;
#endif
};

};	// namespace UI

#endif
