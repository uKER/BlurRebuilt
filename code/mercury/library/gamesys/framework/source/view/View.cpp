#include "FrameworkPch.h"
#include "Framework/View/View.h"
#include <camera2.h>

namespace Framework
{

C_View::C_View()
{
	m_pDebugFeed = new Cam2::C_DebugFeed();
	assert(m_pDebugFeed);
}

C_View::~C_View()
{
	SAFE_DELETE_PTR(m_pDebugFeed);
}

void C_View::Update(const S32 index, const F32 frameTime)
{
	assert(m_pDebugFeed);
	if (!m_pDebugFeed)
		return;

 	m_pDebugFeed->Update(frameTime);
}

}; // namespace Framework
