#include "stdafx.h"
#include "Render\Occlusion\OcclusionTests.h"



#ifdef USER_STEPHENM
#pragma optimize("", off) // USER_STEPHENM
#endif

 

namespace OcclusionTests
{



void Begin(const bool show)
{
	Display::OcclusionQueryRender()->Bind(show);
}



void End()
{
	Display::OcclusionQueryRender()->Unbind();
}



C_Visibility::C_Visibility() :
	m_pQueries(NULL),
	m_CurrentQuery(0),
	m_IsVisible(false),
	m_FramesToLag(1)
{
}



C_Visibility::~C_Visibility()
{
}



bool C_Visibility::Create(const U32 framesToLag)
{
	assert(framesToLag > 0);
	m_FramesToLag	=	framesToLag;
	m_pQueries		=	new C_Query[m_FramesToLag];

	for (U32 ii = 0; ii < m_FramesToLag; ++ii)
	{
		m_pQueries[ii].m_Started	=	false;
		if ( !m_pQueries[ii].m_OcclusionQuery.Create() )
		{
			return false;
		}
	}

	return true;
}



void C_Visibility::Destroy()
{
	if (m_pQueries)
	{	
		for (U32 ii = 0; ii < m_FramesToLag; ++ii)
		{
			m_pQueries[ii].m_OcclusionQuery.Destroy();
		}

		delete[] m_pQueries;
		m_pQueries = NULL;
	}
}



void C_Visibility::Ask(const Maths::HMat43 &toWorld, const Maths::HVec3 &scale)
{
	assert(m_pQueries);
	m_pQueries[m_CurrentQuery].m_OcclusionQuery.Begin();
	{
		Display::OcclusionQueryRender()->Render(toWorld, scale);
	}
	m_pQueries[m_CurrentQuery].m_OcclusionQuery.End();

	m_pQueries[m_CurrentQuery].m_Started	=	true;
	m_CurrentQuery							=	(m_CurrentQuery + 1) % m_FramesToLag;
}



bool C_Visibility::IsVisible()
{
	if (m_pQueries && m_pQueries[m_CurrentQuery].m_Started)
	{
		Display::E_OcclusionQueryResult eResult	=	m_pQueries[m_CurrentQuery].m_OcclusionQuery.Visible();
		if (Display::E_OcclusionQueryResult_Failed != eResult)
		{
			m_IsVisible = true;
		}
		else if (Display::E_OcclusionQueryResult_NotVisible == eResult)
		{
			m_IsVisible = false;
		}
		m_pQueries[m_CurrentQuery].m_Started = false;
	}

	return m_IsVisible;
}



C_PercentageVisible::C_PercentageVisible() :
	m_pQueries(NULL),
	m_CurrentQuery(0),
	m_Visibility(0.0f),
	m_FramesToLag(1)
{
}



C_PercentageVisible::~C_PercentageVisible()
{
}



bool C_PercentageVisible::Create(const U32 framesToLag)
{
	m_FramesToLag	=	framesToLag;
	m_pQueries		=	new C_Query[m_FramesToLag];

	for (U32 ii = 0; ii < m_FramesToLag; ++ii)
	{
		m_pQueries[ii].m_Started	=	false;
		if ( !m_pQueries[ii].m_OcclusionQuery.Create() )
		{
			return false;
		}
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
		if ( !m_pQueries[ii].m_ExtraOcclusionQuery.Create() )
		{
			return false;
		}
#endif
	}

	return true;
}



void C_PercentageVisible::Destroy()
{
	if (m_pQueries)
	{	
		for (U32 ii = 0; ii < m_FramesToLag; ++ii)
		{
			m_pQueries[ii].m_OcclusionQuery.Destroy();
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
			m_pQueries[ii].m_ExtraOcclusionQuery.Destroy();
#endif
		}

		delete[] m_pQueries;
		m_pQueries = NULL;
	}
}



void C_PercentageVisible::Clear()
{
	assert(m_pQueries);
	m_Visibility = 0.0f;
	for (U32 queryNo = 0; queryNo < m_FramesToLag; ++queryNo)
		m_pQueries[queryNo].m_Started = false;
}



void C_PercentageVisible::Ask(const Maths::HMat43 &toWorld, const Maths::HVec3 &scale)
{
	if (m_pQueries)
	{
#if defined (PLATFORM_PC) || defined(PLATFORM_PS3)
		Display::Device()->SetDepthEnable(Display::E_Bool_False);

		m_pQueries[m_CurrentQuery].m_ExtraOcclusionQuery.Begin();
		{
			Display::OcclusionQueryRender()->Render(toWorld, scale);
		}
		m_pQueries[m_CurrentQuery].m_ExtraOcclusionQuery.End();

		Display::Device()->SetDepthEnable(Display::E_Bool_True);
#endif

		m_pQueries[m_CurrentQuery].m_OcclusionQuery.Begin();
		{
			Display::OcclusionQueryRender()->Render(toWorld, scale);
		}
		m_pQueries[m_CurrentQuery].m_OcclusionQuery.End();

		m_pQueries[m_CurrentQuery].m_Started	=	true;
		m_CurrentQuery							=	(m_CurrentQuery + 1) % m_FramesToLag;
	} // end if m_pQueries
}



F32 C_PercentageVisible::Visibility()
{
	if (m_pQueries && m_pQueries[m_CurrentQuery].m_Started)
	{
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
		U32 pixelsPassed = 0;
		U32 pixelsTotal = 0;
		Display::E_OcclusionQueryResult ePassedResult	=	m_pQueries[m_CurrentQuery].m_OcclusionQuery.Visible(pixelsPassed);
		Display::E_OcclusionQueryResult eTotalResult	=	m_pQueries[m_CurrentQuery].m_ExtraOcclusionQuery.Visible(pixelsTotal);
		Display::E_OcclusionQueryResult eFailed			=	Display::E_OcclusionQueryResult_Failed;
		if (eFailed != ePassedResult && eFailed != eTotalResult)
		{
			if (pixelsPassed + pixelsTotal > 0)
			{
				m_Visibility = Maths::Clamp01( F32(pixelsPassed) / F32(pixelsTotal) );
			}
			else
			{
				m_Visibility = 0.0f;
			}
		}
#endif
		m_pQueries[m_CurrentQuery].m_Started = false;
	}

	return m_Visibility;
}



}; // end namespace OcclusionTests
