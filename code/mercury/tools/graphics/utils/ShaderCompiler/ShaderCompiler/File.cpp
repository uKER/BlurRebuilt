#include "StdAfx.h"
#include "File.h"

namespace ShaderCompiler
{

C_File::C_File() :
	m_pStream(NULL)
{
}

C_File::~C_File()
{
	Close();
}

void C_File::Read(size_t& size, int** ppData)
{
	assert(m_pStream);

	fseek(m_pStream, 0, SEEK_END);
	long fileSize = ftell(m_pStream);
	fseek(m_pStream, 0, SEEK_SET);

	size = (fileSize + 3) / 4;

	*ppData = new int[size];
	fread(*ppData, sizeof(int), size, m_pStream);
}

void C_File::Read(int& size, int** ppData)
{
	assert(m_pStream);

	fseek(m_pStream, 0, SEEK_END);
	long fileSize = ftell(m_pStream);
	fseek(m_pStream, 0, SEEK_SET);

	size = (fileSize + 3) / 4;

	*ppData = new int[size];
	fread(*ppData, sizeof(int), size, m_pStream);
}

void C_File::Write(const size_t value)
{
	assert(m_pStream);
	sprintf_s(m_Buffer, 128, "%i", value);
	fwrite(m_Buffer, sizeof(char), strlen(m_Buffer), m_pStream);
}

void C_File::Write(const int value)
{
	assert(m_pStream);
	sprintf_s(m_Buffer, 128, "%i", value);
	fwrite(m_Buffer, sizeof(char), strlen(m_Buffer), m_pStream);
}

void C_File::Write(const char* value)
{
	assert(m_pStream);
	fwrite(value, sizeof(char), strlen(value), m_pStream);
}

void C_File::Write(const std::string& value)
{
	assert(m_pStream);
	fwrite(value.c_str(), sizeof(char), value.length(), m_pStream);
}

bool C_File::Open(const std::string& name, const bool write)
{
	Close();

	fopen_s(&m_pStream, name.c_str(), write ? "wb+" : "rb+");
	if(m_pStream)
	{
		return true;
	}
	else
	{
		DBError("Failed to open file %s\n", name.c_str());
		return false;
	}
}

void C_File::Close()
{
	if(m_pStream)
	{
		fclose(m_pStream);
		m_pStream = NULL;
	}
}

}; // namespace ShaderCompiler
