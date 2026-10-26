#include "StdAfx.h"
#include "ShaderConstantsCompiler.h"

#define NUM_CONSTANTS 256

C_ShaderConstantsCompiler::C_ShaderConstantsCompiler(string filePath) :
	m_FilePath(filePath),
	m_Index(0),
	m_Stored(-1),
	m_Reserved(-1),
	m_Increment(1),
	m_IgnoreReserved(false)
{
	m_Set.resize(NUM_CONSTANTS);
	m_ReservedNames.resize(NUM_CONSTANTS);

	for(int i = 0; i < NUM_CONSTANTS; ++i)
		m_Set[i] = false;
}

C_ShaderConstantsCompiler::~C_ShaderConstantsCompiler()
{
	m_ReservedList.clear();
}

void C_ShaderConstantsCompiler::IsReserved(const int index)
{
	if (m_IgnoreReserved)
	{
		return;
	}
	else if (m_Reserved > -1 && m_Set[index])
	{
		DBError("IsReserved() Index has already been used %i by %s\n", index, m_ReservedNames[index].c_str());
	}
	else if (index < 0)
	{
		DBError("IsReserved() Cannot reserve a negative index %i\n", index);
	}
	else
	{
		for (int i = 0; i < (int)m_ReservedList.size(); i++)
		{
			if (index >= m_ReservedList[i].m_Begin && index <= m_ReservedList[i].m_End)
			{
				DBError("IsReserved() Index already reserved %i by %s\n", index, m_ReservedNames[m_ReservedList[i].m_Begin].c_str());
			}
		}
	}
}

void C_ShaderConstantsCompiler::BeginReserved(const int index)
{
	if (index < 0)
	{
		DBError("BeginReserved() Cannot reserve a negative index %i\n", index);
	}
	else
	{
		if (m_Reserved >= 0)
		{
			DBError("BeginReserved() Begin and end calls to reserved must match for index %i\n", index);
		}
		else
		{
			m_Reserved = index;
		}
	}
}

void C_ShaderConstantsCompiler::EndReserved(const int index)
{
	if (index < 0)
	{
		DBError("EndReserved() Cannot reserve a negative index %i\n", index);
	}
	else
	{
		if (m_Reserved < 0)
		{
			DBError("EndReserved() Begin and end calls to reserved must match for index %i\n", index);
		}
		else
		{
			m_ReservedList.push_back(C_Reserved(m_Reserved, index));
			m_Reserved = -1;
		}
	}
}

void C_ShaderConstantsCompiler::Compile(string filename, ofstream &output)
{
	ifstream input(filename.c_str());

	if (!input)
		DBError("Could not open input file %s\n", filename.c_str());
	if (!output)
		DBError("Could not open output file\n");

	string s;
	while (input >> s) 
	{
		if (s.substr(0, 2) == "//")
		{
			getline(input, s);
		}
		else if (s == "#ignore_reserved")
		{
			m_IgnoreReserved = true;
		}
		else if (s == "#unignore_reserved")
		{
			m_IgnoreReserved = false;
		}
		else if (s == "#increment")
		{
			m_Increment = 1;
		}
		else if (s == "#decrement")
		{
			m_Increment = -1;
		}
		else if (s == "#set")
		{
			input >> m_Index;
		}
		else if (s == "#include")
		{
			input >> s;
			
			string filedir = filename.substr(0, filename.find_last_of("\\") + 1);
			string filename1 = (s.substr(1, s.length() - 2));
			string filename2 = filedir + filename1;

			Compile(filename2, output);
		}
		else if (s == "#store")
		{
			m_Stored = m_Index;
		}
		else if (s == "#restore")
		{
			m_Index = m_Stored;
		}
		else if (s == "#skip")
		{
			int count;
			input >> count;
			m_Index += count - 1;
		}
		else if (s == "#block")
		{
			int count;
			input >> count;
			for(int i = 0; i < count; ++i)
			{
				IsReserved(m_Index);
				m_Set[m_Index] = true;
				m_Index += m_Increment;
			}
		}
		else if (s == "#align")
		{
			unsigned int boundarySize;
			input >> boundarySize;
			if(m_Increment == 1)
				m_Index = m_Index + boundarySize - (m_Index%boundarySize);
			else
				m_Index = m_Index - (m_Index%boundarySize);
		}
		else if (s == "#begin")
		{
			m_Stored = m_Index;
			input >> m_Index;
			BeginReserved(m_Index);
		}
		else if (s == "#end")
		{
			EndReserved(m_Index - 1);
			m_Index = m_Stored;
		}
		else
		{
			if (s[0] == 'C')
			{
				IsReserved(m_Index);

				output << "#define " << s << ' ' << m_Index << "\n";
				output << "#define " << 'C' << s << ' ' << 'c' << m_Index << "\n";
				
				m_Set[m_Index] = true;
				m_ReservedNames[m_Index] = s;

				m_Index += m_Increment;
			}
			else if (s[0] == 'S')
			{
				IsReserved(m_Index);

				output << "#define " << s << ' ' << m_Index << "\n";
				output << "#define " << 'S' << s << ' ' << 's' << m_Index << "\n";
				
				m_Set[m_Index] = true;
				m_ReservedNames[m_Index] = s;

				m_Index += m_Increment;
			}
		}
	}
}
