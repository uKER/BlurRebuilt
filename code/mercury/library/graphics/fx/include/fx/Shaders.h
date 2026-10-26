#ifndef __FX_SHADERS_H__
#define __FX_SHADERS_H__

namespace FX
{

enum E_ShaderType
{
	E_ShaderType_PixelShader,
	E_ShaderType_VertexShader
};

/// shader base class
class Shader
{
public:
	Shader *m_pNext;
	Shader *m_pPrev;
	U32 m_iRefCount;
	U32 m_iHash;
	U32 m_cSize;
#if PLATFORM_PPU_SPU
	U8 *m_pData;
#endif
	E_ShaderType m_type;
	int iDummy;
};

/// Create a shader given a piece of microcode. Checks for duplicates and does refcounting
Shader *CreateShader(void *microcode, int cSize, E_ShaderType type);

/// Free a previously created shader. does refcounting
void FreeShader(Shader *p, E_ShaderType type);

}

#endif
