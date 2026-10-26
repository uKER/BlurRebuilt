#pragma once


class Constant
{

public:
	Constant() { m_strName = 0; m_f32Value = 0.f; }
	~Constant() { if (m_strName) delete [] m_strName; }

	void								Set(char* strName, F32 f32Value)
	{
		m_strName = new char[strlen(strName) + 1];
		strcpy(m_strName, strName);

		m_f32Value = f32Value;
	};
	F32									GetValue() { return m_f32Value; }
	bool								IsCalled(char* strComp) { if (! strComp) return false; else return (strcmp(m_strName, strComp) == 0); }

private:
	char*								m_strName;
	F32									m_f32Value;
};