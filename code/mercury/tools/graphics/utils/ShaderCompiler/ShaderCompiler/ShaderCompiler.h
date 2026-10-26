#ifndef SHADERCOMPILER_SHADERCOMPILER_H
#define SHADERCOMPILER_SHADERCOMPILER_H

#pragma once

namespace ShaderCompiler
{

//! E_ShaderPlatform

enum E_ShaderPlatform
{
	E_ShaderPlatform_PCD3D9,
	E_ShaderPlatform_XB2,
	E_ShaderPlatform_PS3,
	E_ShaderPlatform_Unknown,
};

//! E_ShaderProfile

enum E_ShaderProfile
{
	E_ShaderProfile_2_0,
	E_ShaderProfile_3_0,
	E_ShaderProfile_Unknown,
};

//! E_ShaderType

enum E_ShaderType
{
	E_ShaderType_Pixel,
	E_ShaderType_Vertex,
	E_ShaderType_Unknown,
};

//! C_Shader

class C_Shader
{
public:

	C_Shader();
	~C_Shader();

public:

	int							m_Size;

	char						m_Args[2048];

	E_ShaderProfile				m_Profile;

	std::string					m_DstFileName;
	std::string					m_DstFilePath;
};

//! C_ShaderCompiler

class C_ShaderCompiler
{
public:

	C_ShaderCompiler();
	~C_ShaderCompiler();

	bool Compile(int argc, _TCHAR* argv[]);

private:

	void GetVariable(size_t& varSize, char*& varData, const char* pPlatform);

	std::string GetOutputDir();
	std::string GetType();
	std::string GetProfile(const E_ShaderProfile profile);

	void BuildArgs(C_Shader& shader);

	bool Compile(C_Shader& shader);

	bool WriteShader(C_File& outputFile, const std::string& name, const int size, int* pData);

public:

	E_ShaderType				m_Type;
	E_ShaderPlatform			m_Platform;
	std::string					m_DstFileName;
	std::string					m_DstFileDir;
	std::string					m_DstFilePath;
	std::string					m_SrcFilePath;
	std::string					m_SrcFileDir;
	std::string					m_SrcFileName;
	std::string					m_Entry;
	std::vector<std::string>	m_Defines;
	std::vector<C_Shader>		m_Shaders;
};

}; // namespace ShaderCompiler

#endif // SHADERCOMPILER_SHADERCOMPILER_H
