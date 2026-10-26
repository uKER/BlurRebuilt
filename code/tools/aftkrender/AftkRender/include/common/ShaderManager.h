#ifndef AFTK_RENDER_SHADER_MANAGER
#define AFTK_RENDER_SHADER_MANAGER

#ifdef _MSC_VER
#pragma once
#endif


#include "common/resourcePtr.h"
#include "Synchronise.h"

namespace AftkRender
{
	class C_ShaderManager
	{
	public:

		C_ShaderManager();
		~C_ShaderManager();

		bool Initialise();
		void Clear();

		template <typename T>
		C_ResourcePtr<Display::C_VertexShader> LoadVertexShader(const char* path, const U32 size, const T& shaderStruct)
		{
			Synchronise<C_ShaderManager> sync(this);

			U32 pathHash = Ut::HashKeyOpString::GetHash(path);
			Ut::OpenHashMap<U32, U32, Ut::HashKeyOpSmall<U32>>::Iterator it 
				= m_VertexShaderMap.Find(pathHash);
			if (it != m_VertexShaderMap.End())
				return m_VertexShaderResourceArray[it->GetValue()];

			C_ResourcePtr<Display::C_VertexShader> ptr = new Display::C_VertexShader();
			if (Display::CreateVertexShaderFromFunc(path, 0, ptr, size, shaderStruct))
			{
				m_VertexShaderResourceArray.PushBack(ptr);
				m_VertexShaderMap[pathHash] = m_VertexShaderResourceArray.Size()-1;
				return ptr;
			}
			else
				return 0;
		}

		template <typename T>
		C_ResourcePtr<Display::C_PixelShader> LoadPixelShader(const char* path, const U32 size, const T& shaderStruct)
		{
			Synchronise<C_ShaderManager> sync(this);

			U32 pathHash = Ut::HashKeyOpString::GetHash(path);
			Ut::OpenHashMap<U32, U32, Ut::HashKeyOpSmall<U32>>::Iterator it 
				= m_PixelShaderMap.Find(pathHash);
			if (it != m_PixelShaderMap.End())
				return m_PixelShaderResourceArray[it->GetValue()];

			C_ResourcePtr<Display::C_PixelShader> ptr = new Display::C_PixelShader();
			if (Display::CreatePixelShaderFromFunc(path, 0, ptr, size, shaderStruct))
			{
				m_PixelShaderResourceArray.PushBack(ptr);
				m_PixelShaderMap[pathHash] = m_PixelShaderResourceArray.Size()-1;
				return ptr;
			}
			else
				return 0;
		}

		C_ResourcePtr<Display::C_VertexShader> GetVertexShader(const char* path);
		C_ResourcePtr<Display::C_PixelShader> GetPixelShader(const char* path);


	private:

		Ut::UtVector< Ut::DynamicVector< C_ResourcePtr<Display::C_VertexShader> > > m_VertexShaderResourceArray;
		Ut::UtVector< Ut::DynamicVector< C_ResourcePtr<Display::C_PixelShader> > > m_PixelShaderResourceArray;

		Ut::OpenHashMap< U32, U32, Ut::HashKeyOpSmall<U32> > m_VertexShaderMap;
		Ut::OpenHashMap< U32, U32, Ut::HashKeyOpSmall<U32> > m_PixelShaderMap;

		DECLARE_SYNCHRONISED();
	};

}


#endif