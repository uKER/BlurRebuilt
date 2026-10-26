#include "stdafx.h"
#include <Render/Debug/Debug.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/DeferredDebugRender.h>
//#include <core/threads/mutex.h>

using namespace DbUtil;

using namespace Render::Debug;

// C_DeferredDebugRender

C_DeferredDebugRender::C_DeferredDebugRender( DbUtil::I_DebugRenderPtr ImmediateRenderPtr, const U32 maxItems )
{
	assert( maxItems > 0 );

	m_RenderItemList.Reserve(maxItems);

	m_ImmediateRenderPtr = ImmediateRenderPtr;
}

C_DeferredDebugRender::~C_DeferredDebugRender()
{

}

void C_DeferredDebugRender::Render()
{
	m_Mutex.Wait();

	Maths::HMat43 m;
	m.SetIdentity();

	PERF_EVENT("DeferredDebugRender::Render");

	for(S32 i = 0; i < m_RenderItemList.Size(); i++)
	{
		m_RenderItemList[i].DebugRender( m_ImmediateRenderPtr, m );
	}

	m_Mutex.Release();
}

void C_DeferredDebugRender::Reset()
{
	m_Mutex.Wait();

	m_RenderItemList.Clear();

	m_Mutex.Release();
}

void C_DeferredDebugRender::OnRender( C_DebugRenderItem* pDebugRenderItem )
{
	m_Mutex.Wait();

	m_RenderItemList.PushBack();
	C_DebugRenderItem& item = m_RenderItemList.Back();	
	item = *pDebugRenderItem;

	m_Mutex.Release();
}
