#pragma once


struct ComponentStringBreakdown
{
	char strNameIncludingGroup[256];
	char strOutputString[256];

	ComponentStringBreakdown()
	{
		strNameIncludingGroup[0] = 0;
		strOutputString[0] = 0;
	}
};


class NamedGroupLevel
{

public:
	NamedGroupLevel();
	~NamedGroupLevel();
	char*								GetName() { return m_strGroupName; }
	NamedGroupLevel*					GetParent() { return m_pParentGroup; }

private:
	bool								SetGroupString(char* strFullName);
	void								SetParent(NamedGroupLevel* pParent) { m_pParentGroup = pParent; }
	void								AddChild(NamedGroupLevel* pChild);
	void								AddSibling(NamedGroupLevel* pSibling);
	bool								SetFirstSibling(NamedGroupLevel* pFirst);
	NamedGroupLevel*					GetNextSibling() { return m_pNextSiblingGroup; }

	char*								m_strGroupName;
	NamedGroupLevel*					m_pParentGroup;
	NamedGroupLevel*					m_pFirstSiblingGroup;
	NamedGroupLevel*					m_pNextSiblingGroup;
	NamedGroupLevel*					m_pFirstChildGroup;

	friend class GroupNameManager;
};


class GroupNameManager
{

public:
	GroupNameManager();
	~GroupNameManager();

	char*								GetCurrentGroupName();
	void								AddGroupLevel(char* strNewGroupName);
	NamedGroupLevel*					GetSiblingGroup(char* strGroupName);
	NamedGroupLevel*					GetParentGroup();
	NamedGroupLevel*					GetCurrentGroupLevel() { return m_pCurrentGroupLevel; }
	bool								RemoveGroupLevel();
	bool								SetGroupLevel(NamedGroupLevel* pLevel);

private:
	NamedGroupLevel*					m_pFirstNamedGroupLevel;
	NamedGroupLevel*					m_pCurrentGroupLevel;
	char								m_strEmpty[1];
};


bool AnalyzeComponentNameCommon(std::string strName, NamedGroupLevel* pNamedGroup, ComponentStringBreakdown& stringBreakdown);
