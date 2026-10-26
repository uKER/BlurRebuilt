#include "StdAfx.h"

#include <Display2.h>
#include "Display2/Utils/Utils.h"
#include "fx/ShaderImpl.h"
#include <btl\Utility\hash.h>

namespace FX
{

extern Debug::Logger &g_Log;

/// SetPixelShader can cast up from Shader to PixelShader and then call Display with that
Shader *g_pRoot;

struct ShaderStats
{
	int nShadersLoaded;
	int nShadersShared;
	int nBytesLoaded;
	int nBytesShared;
};

ShaderStats g_Stats;

// #define PUT_PIXEL_SHADERS_IN_SYSTEM_MEMORY // putting this in means we *have* to use the new pixel shader patcher or things won't render correctly

/// Create a shader given a piece of microcode. Checks for duplicates and does refcounting
Shader *CreateShader(void *microcode, int cSize, E_ShaderType type)
{
	U32 iHash;
	// get the microcode out of the shader data and calculate hash
#if PLATFORM_PS3
	CellCgbProgram cgb;
	int32_t res = cellCgbRead(microcode, cSize, &cgb);
	assert(res == CELL_CGB_OK);
	iHash = Btl::HashRaw((U8*)cellCgbGetUCode(&cgb), cellCgbGetUCodeSize(&cgb));
#else
	iHash = Btl::HashRaw((U8*)microcode, cSize);
#endif
	// find existing
	for (Shader *p = g_pRoot; p; p=p->m_pNext)
	{
		if (p->m_iHash == iHash && p->m_cSize == cSize && p->m_type == type)
		{
			p->m_iRefCount++;
			g_Stats.nBytesShared += cSize;
			g_Stats.nShadersShared++;
			return p;
		}
	}

	// create new
	Shader *pShader;
	if (type == E_ShaderType_PixelShader)
	{
		PixelShader *p = new PixelShader;
#if PLATFORM_PS3
		U32 newFlags = 0;
		U8 *pData;
#if PUT_PIXEL_SHADERS_IN_SYSTEM_MEMORY
		Display::Device()->AllocMemory(128, cSize, &p->m_pData, Display::E_AllocHeap_System, Display::E_AllocType_Default);
		memcpy(p->m_pData, microcode, cSize);
		Display::Device()->m_Display2.CreatePixelShader(cSize, (U8*)p->m_pData, Display::E_Usage_Indirect|Display::E_Usage_System, p);
#else
		p->m_pData = new U8[cSize];
		memcpy(p->m_pData, microcode, cSize);
		Display::Device()->m_Display2.CreatePixelShader(cSize, (U8*)p->m_pData, Display2::E_Flags_System, p);
#endif
#else
		Display::Device()->m_Display2.CreatePixelShader(cSize, (U8*)microcode, 0, p);
#endif
		pShader = p;
	}
	else
	{
		VertexShader *p = new VertexShader;
#if PLATFORM_PS3
		p->m_pData = new U8[cSize];
		memcpy(p->m_pData, microcode, cSize);
		Display::Device()->m_Display2.CreateVertexShader(cSize, (U8*)p->m_pData, 0, p);
#else
		Display::Device()->m_Display2.CreateVertexShader(cSize, (U8*)microcode, 0, p);
#endif
		pShader = p;
	}
	pShader->m_iHash = iHash;
	pShader->m_iRefCount = 1;
	pShader->m_pNext = g_pRoot;
	if (g_pRoot)
		g_pRoot->m_pPrev = pShader;
	pShader->m_pPrev = NULL;
	pShader->m_cSize = cSize;
	pShader->m_type = type;
	g_Stats.nBytesLoaded += cSize;
	g_Stats.nBytesShared += cSize;
	g_Stats.nShadersLoaded++;
	g_Stats.nShadersShared++;
	g_pRoot = pShader;
	return pShader;
}

/// Free a previously created shader. does refcounting
void FreeShader(Shader *p, E_ShaderType type)
{
	assert(p->m_iRefCount > 0);
	assert(p->m_type == type);
	g_Stats.nBytesShared -= p->m_cSize;
	g_Stats.nShadersShared--;
	if (--p->m_iRefCount > 0)
		return;

	if (type == E_ShaderType_PixelShader)
	{
#if PLATFORM_PS3
#if PUT_PIXEL_SHADERS_IN_SYSTEM_MEMORY
		Display::Device()->FreeMemory(p->m_cSize, p->m_pData, Display::E_AllocHeap_System);
#else
		delete [] p->m_pData;
#endif
#endif
		Display2::Device()->DestroyPixelShader((PixelShader*)p);
	}
	else
	{
#if PLATFORM_PS3
		delete [] p->m_pData;
#endif
		Display2::Device()->DestroyVertexShader((VertexShader*)p);
	}

	g_Stats.nBytesLoaded -= p->m_cSize;
	g_Stats.nShadersLoaded--;
	// delete
	if (p->m_pPrev)
		p->m_pPrev->m_pNext = p->m_pNext;
	else
		g_pRoot = p->m_pNext;
	if (p->m_pNext)
		p->m_pNext->m_pPrev = p->m_pPrev;

	if (p->m_type == E_ShaderType_PixelShader)
		delete (PixelShader*)p;
	else
		delete (VertexShader*)p;
}

}
