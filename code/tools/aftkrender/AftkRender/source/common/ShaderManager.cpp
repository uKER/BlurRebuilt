#include "AftkRenderPch.h"
#include "common/ShaderManager.h"

using namespace AftkRender;

IMPLEMENT_SYNCHRONISED(C_ShaderManager);

C_ShaderManager::C_ShaderManager()
{
}

C_ShaderManager::~C_ShaderManager()
{
	Clear();
}

bool C_ShaderManager::Initialise()
{
	return true;
}

void C_ShaderManager::Clear()
{
	Synchronise<C_ShaderManager> sync(this);
	
	for (S32 i=0;i<m_VertexShaderResourceArray.Size(); ++i)
		Display::Device()->Destroy(m_VertexShaderResourceArray[i]);

	for (S32 i=0; i<m_PixelShaderResourceArray.Size(); ++i)
		Display::Device()->Destroy(m_PixelShaderResourceArray[i]);

	m_VertexShaderResourceArray.Clear();
	m_PixelShaderResourceArray.Clear();

	m_VertexShaderMap.Clear();
	m_PixelShaderMap.Clear();
}

C_ResourcePtr<Display::C_VertexShader> C_ShaderManager::GetVertexShader(const char* path)
{
	Synchronise<C_ShaderManager> sync(this);
	
	U32 pathHash = Ut::HashKeyOpString::GetHash(path);
	Ut::OpenHashMap< U32, U32, Ut::HashKeyOpSmall<U32> >::Iterator it
		= m_VertexShaderMap.Find(pathHash);

	if (it == m_VertexShaderMap.End())
		return 0;
	else
		return m_VertexShaderResourceArray[it->GetValue()];
}

C_ResourcePtr<Display::C_PixelShader> C_ShaderManager::GetPixelShader(const char* path)
{
	Synchronise<C_ShaderManager> sync(this);

	U32 pathHash = Ut::HashKeyOpString::GetHash(path);
	Ut::OpenHashMap< U32, U32, Ut::HashKeyOpSmall<U32> >::Iterator it
		= m_PixelShaderMap.Find(pathHash);

	if (it == m_PixelShaderMap.End())
		return 0;
	else
		return m_PixelShaderResourceArray[it->GetValue()];
}