#ifndef SHADERCONSTANTSCOMPILER_H
#define SHADERCONSTANTSCOMPILER_H

#pragma once

class C_ShaderConstantsCompiler
{
public:

	C_ShaderConstantsCompiler(string filePath);
	~C_ShaderConstantsCompiler();

	void Compile(string filename, ofstream &output);

private:
	
	void IsReserved(const int index);
	void BeginReserved(const int index);
	void EndReserved(const int index);

public:

	string					m_FilePath;

private:

	int						m_Index;
	int						m_Stored;

	struct C_Reserved
	{
		C_Reserved(const int begin, const int end) : m_Begin(begin), m_End(end)
		{
		}

		int m_Begin;
		int m_End;
	};

	std::vector<C_Reserved>		m_ReservedList;
	int							m_Reserved;
	int							m_Increment;
	bool						m_IgnoreReserved;
	std::vector<bool>			m_Set;
	std::vector<std::string>	m_ReservedNames;
};

#endif // SHADERCONSTANTSCOMPILER_H
