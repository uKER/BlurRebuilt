#include "stdafx.h"
#include "Movie/Movie.h"
#include <System/Memory.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_RENDER_MOVIE
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_RENDER_MOVIE
#endif

namespace Movie
{

C_Movie::C_Movie() : I_Renderer("Movie")
{
}

C_Movie::~C_Movie()
{
	DestroyRenderer();
}

bool C_Movie::OnCreate()
{
	//LOGINFO(g_Log, "C_Movie::OnCreate(%s)\n", m_FileName.Chars());

	RESOURCE_MEMORY_ID("Movie");

	MEMORY_TRACKER_PUSH("mercury.movie");

	/*if(!CreateBink(m_Params))
	{
		MEMORY_TRACKER_POP("mercury.movie");
		return false;
	}*/

	MEMORY_TRACKER_POP("mercury.movie");

	return true;
}

bool C_Movie::OnCreateVolatile()
{
	//LOGINFO(g_Log, "C_Movie::OnCreateVolatile(%s)\n", m_FileName.Chars());

	RESOURCE_MEMORY_ID("Movie");

	MEMORY_TRACKER_PUSH("mercury.movie");

	if(!CreateBink(m_Params))
	{
		MEMORY_TRACKER_POP("mercury.movie");
		return false;
	}
	
	if(!CreateTextures())
	{
		MEMORY_TRACKER_POP("mercury.movie");
		return false;
	}

	MEMORY_TRACKER_POP("mercury.movie");

	return true;
}

void C_Movie::OnDestroyVolatile()
{
	//LOGINFO(g_Log, "C_Movie::OnDestroyVolatile(%s)\n", m_FileName.Chars());

	DestroyTextures();

	DestroyBink();
}

void C_Movie::OnDestroy()
{
	//LOGINFO(g_Log, "C_Movie::OnDestroy(%s)\n", m_FileName.Chars());

	//DestroyBink();
}

bool C_Movie::Open(const Util::C_String& fileName, const C_MovieParams& params)
{
	m_FileName = fileName;
	m_Params = params;

	if(!Create())
		return false;
	if(!CreateVolatile())
	{
		// [Blur2PC] no Bink on PC: drop the involatile state so device resets skip this renderer
		Destroy();
		return false;
	}

	return true;
}

void C_Movie::Close()
{

	DestroyVolatile();
	Destroy();
}

}; // namespace Movie
