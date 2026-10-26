#ifndef RENDER_OCCLUSION_TEST_H
#define RENDER_OCCLUSION_TEST_H

#if _MSC_VER > 1000
#pragma once
#endif


#include "Display/Occlusion/OcclusionQuery.h"



namespace OcclusionTests
{



class C_Visibility
{

public:

	C_Visibility();
	~C_Visibility();

	bool	Create(const U32 framesToLag);
	void	Destroy();

	bool	IsVisible();
	void	Ask(const Maths::HMat43 &toWorld, const Maths::HVec3 &scale);

private:

	struct C_Query
	{
		Display::C_OcclusionQuery 	m_OcclusionQuery;
		bool						m_Started;
	};

	C_Query *	m_pQueries;
	U32			m_CurrentQuery;

	bool		m_IsVisible;

	U32			m_FramesToLag;
};



class C_PercentageVisible
{

public:

	C_PercentageVisible();
	~C_PercentageVisible();

	bool	Create(const U32 framesToLag);
	void	Destroy();

	void	Clear();

	F32		Visibility();
	void	Ask(const Maths::HMat43 &toWorld, const Maths::HVec3 &scale);

private:

	struct C_Query
	{
		Display::C_OcclusionQuery 	m_OcclusionQuery;
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
		Display::C_OcclusionQuery	m_ExtraOcclusionQuery;
#endif
		bool						m_Started;
	};

	C_Query *	m_pQueries;
	U32			m_CurrentQuery;

	F32			m_Visibility;

	U32			m_FramesToLag;
};



void Begin(const bool show = false);
void End();



};



#endif
