#ifndef SHADER_COMPILER_H
#define SHADER_COMPILER_H

#pragma once

//! C_ShaderCompiler

struct C_ShaderCompiler
{
	C_ShaderCompiler();
	~C_ShaderCompiler();
	
	void GetVariable(size_t& varSize, char*& varData, const char* pPlatform);

	int Compile();

	bool LoadDefines(const Util::C_String& filePath);
	bool LoadCpyFileDir(const Util::C_String& filePath);
	bool InitCpyFilePath();
	bool InitCpyFileExt();

	bool SetPlatform(const char* platform);
	bool SetSrcFilePath(const char* srcFilePath);
	bool SetProfileFromSrcFilePath();

	Util::C_String						m_Platform;
	Util::C_String						m_Profile;
	Util::C_String						m_Entry;

	Util::C_String						m_SrcFilePath;
	Util::C_String						m_SrcFileDir;
	Util::C_String						m_SrcFileName;

	Util::C_String						m_DstFilePath;
	Util::C_String						m_DstFileDir;
	Util::C_String						m_DstFileName;

	Util::C_String						m_CpyFileDir;
	Util::C_String						m_CpyFilePath;
	Util::C_String						m_CpyFileExt;

	std::vector<Util::C_String>		m_Defines;

	char							m_Args[1024];
};

#endif // SHADER_COMPILER_H
