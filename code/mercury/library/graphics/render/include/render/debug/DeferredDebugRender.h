#ifndef RENDER_DEFERREDDEBUGRENDER_H
#define RENDER_DEFERREDDEBUGRENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/debug/debugrender/DebugRenderItem.h>

namespace Render
{
namespace Debug
{
    
class C_DeferredDebugRender;
typedef C_DeferredDebugRender* C_DeferredDebugRenderPtr;

// most of the functionality is now provided by C_ItemDebugRenderer

// C_DeferredDebugRender

class C_DeferredDebugRender : public DbUtil::C_ItemDebugRenderer
{
public:
	C_DeferredDebugRender( DbUtil::I_DebugRenderPtr ImmediateRenderPtr, const U32 maxItems );
	virtual ~C_DeferredDebugRender();

	void                    Render();
	void                    Reset();

	void OnRender( DbUtil::C_DebugRenderItem* pDebugRenderItem ) OVERRIDE;

private:
	DbUtil::I_DebugRenderPtr									m_ImmediateRenderPtr;
	Ut::UtVector<Ut::DynamicVector<DbUtil::C_DebugRenderItem> >	m_RenderItemList;
	MT::Mutex													m_Mutex;

};

} // namespace Debug
} // namespace Render

#endif // RENDER_DEFERREDDEBUGRENDER_H
