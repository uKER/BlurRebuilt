#include "AudioPch.h"


bool AnalyzeComponentNameCommon(std::string strName, NamedGroupLevel* pNamedGroup, ComponentStringBreakdown& stringBreakdown)
{
	bool bError = false;

	char* strCompName = (char*)(strName.data());
	U32 u32GroupsToUnnest = 0;

	while(strCompName[0] == ':')
	{
		u32GroupsToUnnest++;
		strCompName = &(strCompName[1]);
	}

	while (u32GroupsToUnnest && pNamedGroup)
	{
		u32GroupsToUnnest--;
		pNamedGroup = pNamedGroup->GetParent();
	}

	stringBreakdown.strNameIncludingGroup[0] = 0;
	
	if (pNamedGroup)
		sprintf(stringBreakdown.strNameIncludingGroup, "%s.%s", pNamedGroup->GetName(), strCompName);
	else
		sprintf(stringBreakdown.strNameIncludingGroup, "%s", strCompName);

	// component name might be appended by string denoting output type, in brackets
	// scan for this
	bool bHasOutputString = false;
	char* strOutputString = 0;
	U32 u32CompNameLength = strlen(stringBreakdown.strNameIncludingGroup);

	for (char* pOutputScan = stringBreakdown.strNameIncludingGroup; pOutputScan[0] != 0 && (! bError); pOutputScan = &(pOutputScan[1]))
	{
		if (*pOutputScan == '(')
		{
			if (bHasOutputString)	// shouldn't be set twice
				bError = true;
			else
			{
				bHasOutputString = true;
				strOutputString = &(pOutputScan[1]);
				u32CompNameLength = pOutputScan - stringBreakdown.strNameIncludingGroup;
			}
		}
		else if (*pOutputScan == ')')
		{
			if (! bHasOutputString)
				bError = true;
			else if (pOutputScan[1] != 0)
				bError = true;
		}
	}

	stringBreakdown.strNameIncludingGroup[u32CompNameLength] = 0;

	U32 u32CloseBracketIndex = 0;
	if (bHasOutputString && (! bError))
	{
		for (; strOutputString[u32CloseBracketIndex] != ')'; u32CloseBracketIndex++)
		{
			//if (strOutputString[u32CloseBracketIndex] < '0' || strOutputString[u32CloseBracketIndex] > '9')
			//	bError = true;
		}
	}

	if (bHasOutputString && (! u32CloseBracketIndex))
		bError = true;

	if ((! bError) && bHasOutputString)
	{
		strncpy(stringBreakdown.strOutputString, strOutputString, u32CloseBracketIndex);
		stringBreakdown.strOutputString[u32CloseBracketIndex] = 0;
	}

	return (! bError);
}