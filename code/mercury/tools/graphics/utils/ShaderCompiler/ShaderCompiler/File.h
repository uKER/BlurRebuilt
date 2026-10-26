#ifndef SHADERCOMPILER_FILE_H
#define SHADERCOMPILER_FILE_H

#pragma once

namespace ShaderCompiler
{

//! C_File

class C_File
{
public:

	C_File();
	~C_File();

	void Read(size_t& size, int** ppData);
	void Read(int& size, int** ppData);
	
	void Write(const size_t value);
	void Write(const int value);
	void Write(const char* value);
	void Write(const std::string& value);

	bool Open(const std::string& name, const bool write);
	void Close();

private:

	char	m_Buffer[128];
	FILE*	m_pStream;
};

}; // namespace ShaderCompiler

#endif // SHADERCOMPILER_FILE_H
