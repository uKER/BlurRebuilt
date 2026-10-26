#include "AudioPch.h"

using namespace Audio;

//AssertFunction *g_pCoreAssertFunction = NULL;

//void Audio::Assert(bool bExp, char* strMessage)
//{
//	if (! bExp)
//	{
//		if (strMessage)
//			printf("Assert- %s\n", strMessage);
//		else
//			printf("Hit an Assert.\n");
//
//		DebugBreak();
//	}
//}

//void Audio::Assert(bool bExp)
//{
//	if (! bExp)
//	{
//		printf("Hit an Assert.\n");
//
//		DebugBreak();
//	}
//}

Diagnostics AudioDataFile::m_Diagnostics;

 
void ChildDataContainer::SetData(void* pData, U32 u32Size)
{
	Assert(! m_pChild);
	m_pChild = new char[u32Size];
	memcpy(m_pChild, pData, u32Size);
	m_u32Size = u32Size;
}


void ChildDataList::AddChild(void* pData, U32 u32Size)
{
	ChildDataContainer* pContainer = new ChildDataContainer();
	pContainer->SetData(pData, u32Size);

	if (m_pLastContainer)
	{
		pContainer->SetPrev(m_pLastContainer);
		m_pLastContainer->SetNext(pContainer);
	}
	else
		m_pFirstContainer = pContainer;

	m_pLastContainer = pContainer;

	m_u32NumChildren++;
}


NamedGroupLevel::NamedGroupLevel()
{
	m_strGroupName = 0;
	m_pParentGroup = 0;
//	m_pFirstSiblingGroup = 0;
	m_pNextSiblingGroup = 0;
	m_pFirstChildGroup = 0;
}


NamedGroupLevel::~NamedGroupLevel()
{
	if (m_strGroupName)
		delete [] m_strGroupName;

	m_strGroupName = 0;

	if (m_pFirstChildGroup)
		delete m_pFirstChildGroup;

	m_pFirstChildGroup = 0;

	if (m_pNextSiblingGroup)
		delete m_pNextSiblingGroup;

	m_pNextSiblingGroup = 0;

//	m_pFirstSiblingGroup = 0;
}


bool NamedGroupLevel::SetGroupString(char* strFullName)
{
	if (m_strGroupName || (! strFullName) || (strlen(strFullName) == 0))
		return false;

	m_strGroupName = new char[strlen(strFullName) + 1];

	strcpy(m_strGroupName, strFullName);
	m_strGroupName[strlen(strFullName)] = 0;

	return true;
}


void NamedGroupLevel::AddChild(NamedGroupLevel* pChild)
{
	if (! m_pFirstChildGroup)
		m_pFirstChildGroup = pChild;
	else
		m_pFirstChildGroup->AddSibling(pChild);
}


void NamedGroupLevel::AddSibling(NamedGroupLevel* pSibling)
{
	if (m_pNextSiblingGroup)
		m_pNextSiblingGroup->AddSibling(pSibling);
	else
		m_pNextSiblingGroup = pSibling;

	//if (! m_pFirstSiblingGroup)
	//{
	//	m_pFirstSiblingGroup = this;
	//	pSibling->SetFirstSibling(this);
	//}
	//else
	//	pSibling->SetFirstSibling(m_pFirstSiblingGroup);
}


bool NamedGroupLevel::SetFirstSibling(NamedGroupLevel* pFirst)
{
	//if (m_pFirstSiblingGroup)
	//{
	//	DebugBreak();
	//	return false;
	//}

//	m_pFirstSiblingGroup = pFirst;

	return true;
}


GroupNameManager::GroupNameManager()
{
	m_pFirstNamedGroupLevel = 0;
	m_pCurrentGroupLevel = 0;
	m_strEmpty[0] = 0;
}


GroupNameManager::~GroupNameManager()
{
	if (m_pFirstNamedGroupLevel)
		delete m_pFirstNamedGroupLevel;

	m_pFirstNamedGroupLevel = 0;
}


char* GroupNameManager::GetCurrentGroupName()
{
	if (m_pCurrentGroupLevel)
		return m_pCurrentGroupLevel->GetName();
	else
		return m_strEmpty;
}


void GroupNameManager::AddGroupLevel(char* strNewGroupName)
{
	NamedGroupLevel* pNewGroup = new NamedGroupLevel();

	if (m_pCurrentGroupLevel)
	{
		char strWorkString[256] = {0};

		if (GetCurrentGroupName() && strlen(GetCurrentGroupName()))
			sprintf(strWorkString, "%s.%s", GetCurrentGroupName(), strNewGroupName);
		else
			sprintf(strWorkString, "%s", strNewGroupName);

		pNewGroup->SetGroupString(strWorkString);
		pNewGroup->SetParent(m_pCurrentGroupLevel);
		m_pCurrentGroupLevel->AddChild(pNewGroup);
	}
	else
	{
		char strWorkString[256] = {0};

		if (GetCurrentGroupName() && strlen(GetCurrentGroupName()))
			sprintf(strWorkString, "%s.%s", GetCurrentGroupName(), strNewGroupName);
		else
			sprintf(strWorkString, "%s", strNewGroupName);

		pNewGroup->SetGroupString(strWorkString);

		if (m_pFirstNamedGroupLevel)
			m_pFirstNamedGroupLevel->AddSibling(pNewGroup);
		else
			m_pFirstNamedGroupLevel = pNewGroup;
	}

	m_pCurrentGroupLevel = pNewGroup;
}


NamedGroupLevel* GroupNameManager::GetSiblingGroup(char* strGroupName)
{
	for (NamedGroupLevel* pCurr = m_pFirstNamedGroupLevel; pCurr; pCurr = pCurr->GetNextSibling())
	{
		if (strcmp(pCurr->GetName(), strGroupName) == 0)
			return pCurr;
	}

	return 0;
}


NamedGroupLevel* GroupNameManager::GetParentGroup()
{
	if (m_pCurrentGroupLevel)
		return m_pCurrentGroupLevel->GetParent();
	else
		return 0;
}


bool GroupNameManager::RemoveGroupLevel()
{
	if (! m_pCurrentGroupLevel)
		return false;

	m_pCurrentGroupLevel = m_pCurrentGroupLevel->GetParent();

	return true;
}


bool GroupNameManager::SetGroupLevel(NamedGroupLevel* pLevel)
{
	m_pCurrentGroupLevel = pLevel;

	return true;
}


ComponentData::ComponentData()
{
	m_u32DataSize = 0;
	m_pComponentData = 0;
	m_strName = 0;
	m_u32ComponentType = -1;
	m_pFirstChildList = 0;
	m_u32XmlLineNumber = 0;
	m_strXmlFile = 0;
	m_pNamedGroupLevel = 0;
}


ComponentData::ComponentData(const char* strName)
{
	m_u32DataSize = 0;
	m_pComponentData = 0;
	m_strName = 0;
	m_u32ComponentType = -1;
	m_pFirstChildList = 0;
	SetName(strName);
	m_u32XmlLineNumber = 0;
	m_strXmlFile = 0;
	m_pNamedGroupLevel = 0;
}


ComponentData::~ComponentData()
{
	if (m_strName)
		delete [] m_strName;

	if (m_strXmlFile)
		delete [] m_strXmlFile;

	if (m_pComponentData)
		delete m_pComponentData;

	if (m_pFirstChildList)
	{
		delete m_pFirstChildList;
		m_pFirstChildList = 0;
	}
}


bool ComponentData::SetName(const char* strName)
{
	if (m_strName || (! strName))
		return false;

	m_strName = new char[strlen(strName) + 1];
	strncpy(m_strName, strName, strlen(strName));

	m_strName[strlen(strName)] = 0;

	return true;
}


bool ComponentData::SetXmlFile(const char* strFile)
{
	if (m_strXmlFile || (! strFile))
		return false;

	m_strXmlFile = new char[strlen(strFile) + 1];
	strncpy(m_strXmlFile, strFile, strlen(strFile));

	m_strXmlFile[strlen(strFile)] = 0;

	return true;
}

/*
void ComponentData::AddChildData(U32 u32Id, ComponentData* pData)
{
	ChildDataList* pCompList = GetDataList(u32Id);

	if (! pCompList)
	{
		pCompList = new ChildDataList();
		pCompList->SetId(u32Id);
		pCompList->SetNext(m_pFirstChildList);
		m_pFirstChildList = pCompList;
	}

	pCompList->AddChild(pData);
}*/


void ComponentData::AddChildData(U32 u32Id, void* pData, U32 u32DataSize)
{
	ChildDataList* pCompList = GetDataList(u32Id);

	if (! pCompList)
	{
		pCompList = new ChildDataList();
		pCompList->SetId(u32Id);

		if (! m_pFirstChildList)
			m_pFirstChildList = pCompList;
		else
		{
			// place the new list in the linked list of lists based on it's id..
			ChildDataList* pPrevList = 0;
			for (ChildDataList* pCurrList = m_pFirstChildList; pCurrList; pCurrList = pCurrList->GetNext())
			{
				if (pCurrList->GetId() < u32Id)	// means our new list wants to go before this one...
				{
					if (pPrevList)
						pPrevList->SetNext(pCompList);

					pCompList->SetNext(pCurrList);

					if (pCurrList == m_pFirstChildList)
						m_pFirstChildList = pCompList;
				}

				pPrevList = pCurrList;
			}
		}

//		pCompList->SetNext(m_pFirstChildList);
//		m_pFirstChildList = pCompList;
	}

	pCompList->AddChild(pData, u32DataSize);
}


U32 ComponentData::GetNumChildren()
{
	U32 u32NumChildren = 0;

	for (ChildDataList* pCurrList = m_pFirstChildList; pCurrList; pCurrList = pCurrList->GetNext())
		u32NumChildren += pCurrList->GetNumChildren();

	return u32NumChildren;
}


ChildDataList* ComponentData::GetDataList(U32 u32Id)
{
	for (ChildDataList* pList = m_pFirstChildList; pList; pList = pList->GetNext())
	{
		if (pList->GetId() == u32Id)
			return pList;
	}

	return 0;
}


AudioDataFile::AudioDataFile()
{
	m_strObjectName = 0;
	m_u32CurrComponentIndex = -1;
	m_u32ComponentDataSize = 0;
	m_u32CurrCompNameStringTableSize = 0;
	m_strComponentOutputString[0] = 0;
}


AudioDataFile::~AudioDataFile()
{
	Release();
}


void AudioDataFile::Release()
{
	for (U32 i = 0; i < m_apComponentData.Size(); i++)
	{
		delete m_apComponentData[i];
		m_apComponentData[i] = 0;
	}

	m_apComponentData.Clear();

	for (U32 i = 0; i < m_aStrictParseAttrNames.Size(); i++)
	{
		m_aStrictParseAttrNames[i]->clear();
		delete m_aStrictParseAttrNames[i];
		m_aStrictParseAttrNames[i] = 0;
	}

	m_aStrictParseAttrNames.Clear();

	for (U32 i = 0; i < m_aNonComponentNameStrings.Size(); i++)
	{
		delete m_aNonComponentNameStrings[i];
		m_aNonComponentNameStrings[i] = 0;
	}

	for (U32 i = 0; i < m_aConstants.Size(); i++)
	{
		delete m_aConstants[i];
		m_aConstants[i] = 0;
	}

	m_aConstants.Clear();

	m_aNonComponentNameStrings.Clear();

	if (m_strObjectName)
		delete [] m_strObjectName;
}


//------------------------------------------------------------------------------
//  Insert
//------------------------------------------------------------------------------
void AudioDataFile::Insert(xml::TagNode* pTag)
{
    Audio::Iterator<xml::Node*> i;
    for (i = pTag->GetChildren().first(); !i.isPastEnd(); i.advance())
	{
        xml::TagNode* pChildTag = dynamic_cast<xml::TagNode*>(i.retrieve());

        if (pChildTag)
        {
            if (strcmp(pChildTag->GetType(), "Include") == 0)
            {
	            xml::Parser xmlParser;
                xml::Node* pInsertNode;

				xmlParser.SetErrorCallback(ReceiveParserError);

                std::string strFile;
	            GetValueFromKey(pChildTag->GetAttributes(), "file", strFile);

                printf("Including \'%s\'\n", strFile.data());

				if (xmlParser.Load(strFile.data(), &pInsertNode, m_Diagnostics.IsStrict()))
                {
                    //  set new node's parent
                    pInsertNode->SetParent(pTag);

                    //  add new
                    pTag->GetChildren().insert(pInsertNode, i);  //  TODO: insert with no iter adds to end...
                }
                else
                    printf("Failed Including\n");
            }

			if (! m_Diagnostics.CanContinue())
				return;

            //  recurse
            Insert(pChildTag);
        }
    }
}


bool AudioDataFile::LoadXml(char* strFilename, char* strError)
{
	xml::Parser xmlParser;
    xml::Node* pRootNode;

	xmlParser.SetErrorCallback(ReceiveParserError);

    try
    {
		if (xmlParser.Load(strFilename, (xml::Node**)&pRootNode, true))
        {
            //  TODO: verify
            Insert((xml::TagNode*)pRootNode);

            if (strcmp(((xml::TagNode*)pRootNode)->GetType(), "Object") != 0)
			{
				delete pRootNode;
				sprintf(strError, "First tag should be object tag");
				return false;
			}
		}
		else
		{
			delete pRootNode;
			return false;
		}

		if (! m_Diagnostics.CanContinue())
		{
			delete pRootNode;
			return false;
		}

		Enumerate(pRootNode);

        EndNode((xml::TagNode*)pRootNode);

		// check if any constant names clash with component names...
		for (U32 i = 0; i < m_aConstants.Size(); i++)
		{
			for (U32 i2 = 0; i2 < m_apComponentData.Size(); i2++)
			{
				if (m_aConstants[i]->IsCalled(m_apComponentData[i2]->GetName()))
				{
					char strError[256] = {0};
					sprintf(strError, "Constant %s has same name as a component", m_apComponentData[i2]->GetName());
					DoError(strError, m_apComponentData[i2]->GetLineNumber(), m_apComponentData[i2]->GetXmlFile());
				}
			}
		}

		ConvertNodeToAudioData(pRootNode);

		if (! m_Diagnostics.CanContinue())
		{
			delete pRootNode;
			return false;
		}

		// ensure all components have correct number of children
		for (U32 i = 0; i < m_apComponentData.Size(); i++)
		{
			if (m_apComponentData[i]->GetType() == Audio::Component::eProduct || m_apComponentData[i]->GetType() == Audio::Component::ePiecewiseLinearFunction)
			{
				if (m_apComponentData[i]->GetNumChildren() < 2)
					DoWarning("Component should have at least 2 children", m_apComponentData[i]->GetLineNumber(), m_apComponentData[i]->GetXmlFile());
			}
		}
	}
	catch (Error& err)
	{
		// hacky workaround, if err.Where() is passed back in DoError(..), is can contain garbage
		//  as it points to a member of the root node, which gets deleted the line before..
		static char strTemp[256] = {0};
		strncpy(strTemp, err.Where(), 255);
		strTemp[255] = 0;

		delete pRootNode;

		return DoError(err.What(), err.When(), strTemp, true);
	}

	delete pRootNode;

	if (m_Diagnostics.m_u32Warnings && m_Diagnostics.IsStrict())
		return false;
	else
		return true;
}


void AudioDataFile::Enumerate(xml::Node* pNode)
{
    xml::TagNode* pTag = dynamic_cast<xml::TagNode*>(pNode);

	if (pTag)
	{
		//	if a group
		if (strcmp(pTag->GetType(), "Group") == 0)
		{
            std::string strGroup;
            if (pTag->GetAttributes() && GetValueFromKey(pTag->GetAttributes(), "Name", strGroup))
            {
				m_GroupNameManager.AddGroupLevel((char*)(strGroup.data()));
                //m_strCurrGroup += strGroup;
                //m_strCurrGroup += ".";
            }
        }
		//	if a component
        else if (pTag->GetParent() &&
			(GetComponentType(pTag->GetType()) != -1) &&	// ..jkc
            (strcmp(pTag->GetType(), "Include") != 0) &&
            (strcmp(pTag->GetType(), "globalComInput") != 0))   //  ignore tags from keefs tool
		{
			std::string strFullName;
			std::string strName;

			if (pTag->GetAttributes())
            {
				//	only tags whose parents are components are allowed to not have a name..
				//	therefore any tag with 'object' or a 'group' as a parent should throw an error
				if (pTag->GetParent())
				{
					if (!GetValueFromKey(pTag->GetAttributes(), "Name", strName))
					{
						if (strcmp(pTag->GetParent()->GetType(), "Object") == 0
							|| strcmp(pTag->GetParent()->GetType(), "Group") == 0)
							throw Error("Component has no Name attribute", pTag->GetLine(), pTag->GetFilename());
					}
					GetValueFromKey(pTag->GetAttributes(), "Name", strName);
				}
            }
            else
                throw Error("Component has no attributes", pTag->GetLine(), pTag->GetFilename());

			//	has valid name attribute?
			if (strName.length() > 0)
			{
                int iIndex = 0;

//                strFullName = m_strCurrGroup + strName;
				char strTempString[256] = {0};

				if (strlen(m_GroupNameManager.GetCurrentGroupName()))
					sprintf(strTempString, "%s.%s", m_GroupNameManager.GetCurrentGroupName(), strName.data());
				else
					sprintf(strTempString, "%s", strName.data());

				strFullName = strTempString;

				//	is unique?
				for (U32 i = 0; i < m_apComponentData.Size(); i++)
				{
					char* strCompName = m_apComponentData[i]->GetName();
					if (! strCompName)
						continue;

					if (strcmp(strCompName, strFullName.data()) == 0)
                        throw Error("Component name is not unique", pTag->GetLine(), pTag->GetFilename());
				}

//                Audio::Script::ComponentInfo* pComInfo = new Audio::Script::ComponentInfo(strFullName, std::string(pTag->GetType()), iIndex);

                //  don't add globalComInput tag...
//				m_lComponentInfos.insert(pComInfo, m_lComponentInfos.last());

				AllocateComponent(strFullName.data(), GetComponentType(pTag->GetType()), pTag->GetFilename(), pTag->GetLine(), m_GroupNameManager.GetCurrentGroupLevel());

			}
			else
			{
				AllocateComponent(0, GetComponentType(pTag->GetType()), pTag->GetFilename(), pTag->GetLine());
			}
//            else
//              throw Error("Component name is empty", pTag->GetLine(), pTag->GetFilename());
		}
		else if (strcmp(pTag->GetType(), "Constant") == 0)
		{
			std::string strName;
			std::string strValue;

			if (! GetValueFromKey(pTag->GetAttributes(), "Name", strName))
				throw Error("Constant has no Name attribute", pTag->GetLine(), pTag->GetFilename());

			if (! GetValueFromKey(pTag->GetAttributes(), "ConstantVal", strValue))
				throw Error("Constant has no ConstantVal attribute", pTag->GetLine(), pTag->GetFilename());

			F32 fValue = 0.f;

			if (! GetFloatValue(strValue.data(), &fValue))
				throw Error("Constant attribute ConstantVal does not contain a valid number", pTag->GetLine(), pTag->GetFilename());

			char strTemp[256] = {0};
			if (strlen(m_GroupNameManager.GetCurrentGroupName()))
				sprintf(strTemp, "%s.%s", m_GroupNameManager.GetCurrentGroupName(), strName.data());
			else
				sprintf(strTemp, strName.data());

			AddConstant(strTemp, fValue);
		}

		for (Audio::Iterator<xml::Node*> i = pTag->GetChildren().first(); !i.isPastEnd(); i.advance())
		{
			xml::Node* pChild = i.retrieve();

			//	recurse
			Enumerate(pChild);

            if (dynamic_cast<xml::TagNode*>(pNode))
                EndNode((xml::TagNode*)pChild);
		}
	}
}


bool AudioDataFile::Save(char* strFilename, CommandLineApp* app, bool bWriteBankList)
{
	FILE* fOut = fopen(strFilename, "wb");

	if (! fOut)
		return false;

	U32 uFrameSkip = S32(m_uFrameSkip);

	//if (! app->IsBigEndian())
	//	uFrameSkip = app->EndianSwap32Bit(uFrameSkip);

	AudioDataFileHeader header;
	header.m_u32StringTableOffset = GetStringTableOffset();
	header.m_u32ObjectNameStringIndex = 0;	// object name is always placed as the 1st entry in the string table
	header.m_u32NumComponents = m_u32CurrComponentIndex + 1;
	header.m_u32VersionNumber = (DAT_FILE_VERSION_MAJOR << 16) + DAT_FILE_VERSION_MINOR;
	header.m_u32FrameSkip = uFrameSkip;
	strncpy(header.m_strTag, "JBOA", 4);

	app->WriteBytes(&header, sizeof(AudioDataFileHeader), fOut);

	for (U32 i = 0; i < m_apComponentData.Size(); i++)
	{
		// skip components we haven't implemented yet..
		if ((! m_apComponentData[i]->GetData()) || m_apComponentData[i]->GetType() == -1)
			continue;

		U32 u32CompType = m_apComponentData[i]->GetType();

		app->WriteBytes(&u32CompType, 4, fOut);

		app->WriteBytes(m_apComponentData[i]->GetData(), m_apComponentData[i]->GetDataSize(), fOut, false);

		for (ChildDataList* pChildList = m_apComponentData[i]->GetFirstDataList(); pChildList; pChildList = pChildList->GetNext())
		{
			for (ChildDataContainer* pCont = pChildList->GetFirstContainer(); pCont; pCont = pCont->GetNext())
			{
				if ((pChildList->GetId() == eDataTypeEffect && pCont->GetSize() == 4)
					|| pChildList->GetId() == eDataTypeScript)
					app->WriteBytes(pCont->GetData(), pCont->GetSize(), fOut, true);
				else
					app->WriteBytes(pCont->GetData(), pCont->GetSize(), fOut, false);
			}
		}
	}

	fwrite(m_strObjectName, strlen(m_strObjectName) + 1, 1, fOut);

	for (U32 i = 0; i < m_apComponentData.Size(); i++)
	{
		if (m_apComponentData[i]->GetName())
			fwrite(m_apComponentData[i]->GetName(), strlen(m_apComponentData[i]->GetName()) + 1, 1, fOut);
	}

	for (U32 i = 0; i < m_aNonComponentNameStrings.Size(); i++)
	{
		fwrite(m_aNonComponentNameStrings[i]->m_strName, strlen(m_aNonComponentNameStrings[i]->m_strName) + 1, 1, fOut);
	}

	fclose(fOut);

	if (bWriteBankList)
	{
		Btl::String string(strFilename, (S32)strlen(strFilename)-3); 
		string += "wbl";

		if (m_WaveBankList.WriteToDisk(string) == false)
		{
			system("color c");
			printf("\nCould not write wave bank list file '%s'\n", string.Chars());
			system("pause 0");
		}
	}

	return true;
}


//------------------------------------------------------------------------------
//	AddConstant
//------------------------------------------------------------------------------
void AudioDataFile::AddConstant(char* strName, F32 fValue)
{
	Constant* pConstant = new Constant;
	pConstant->Set(strName, fValue);

	m_aConstants.PushBack(pConstant);
}


//------------------------------------------------------------------------------
//	LookupConstant
//------------------------------------------------------------------------------
bool AudioDataFile::LookupConstant(char* strName, F32* pfValue)
{
	for (U32 i = 0; i < m_aConstants.Size(); i++)
	{
		if (m_aConstants[i]->IsCalled(strName))
		{
			*(pfValue) = m_aConstants[i]->GetValue();
			return true;
		}
	}

	return false;
}


//------------------------------------------------------------------------------
//  EndNode
//  changes group prefix
//------------------------------------------------------------------------------
void AudioDataFile::EndNode(xml::TagNode* pTag)
{
    if (strcmp(pTag->GetType(), "Group") == 0)
    {
        std::string strName;
        if (pTag->GetAttributes() && GetValueFromKey(pTag->GetAttributes(), "Name", strName))
			m_GroupNameManager.RemoveGroupLevel();
//            m_strCurrGroup.resize(m_strCurrGroup.length() - strName.length() - 1);
    }
}

//------------------------------------------------------------------------------
//  GetValueFromKey
//	(look up value and remove attribute from list)
//------------------------------------------------------------------------------
bool AudioDataFile::GetValueFromKey(const char* data, const char* pchKey, std::string& strValue)
{
	for (unsigned int i = 0; i < m_aStrictParseAttrNames.Size(); i++)
	{
		if (std::string(pchKey) == *(m_aStrictParseAttrNames[i]))	// match
		{
			// remove entry from list..
			m_aStrictParseAttrNames[i]->clear();
			delete m_aStrictParseAttrNames[i];
			m_aStrictParseAttrNames.Erase(i);

			break;
		}
	}

	return PeekValueFromKey(data, pchKey, strValue);
}


//------------------------------------------------------------------------------
//  PeekValueFromKey
//	(look up value without removing attribute from list)
//------------------------------------------------------------------------------
bool AudioDataFile::PeekValueFromKey(const char* pchData, const char* pchKey, std::string& strValue)
{
    char* pchCurr = (char*)pchData;
    strValue = "";
    char* pchCloseQuote;

	char* pchKeyStart = strstr(pchCurr, pchKey);

	//	ensure it's not a substring
	while (pchKeyStart)
	{
		if (pchKeyStart != pchData
			&& (pchKeyStart[-1] != ' ' && pchKeyStart[-1] != '\t' && pchKeyStart[-1] != '\n' && pchKeyStart[-1] != '\r'))
			pchKeyStart = strstr(&pchKeyStart[1], pchKey);
		else
			break;
	}

    //  if a match is found
    while (pchKeyStart)
    {
		//	make sure its the whole word
		char* pchKeyEnd = pchKeyStart + strlen(pchKey);

		//	skip whitespace
		while ((*pchKeyEnd == ' ') || (*pchKeyEnd == '\t') || (*pchKeyEnd == '\n') || (*pchKeyEnd == '\r'))
			pchKeyEnd++;

		//	next char must be '='
		char* pchEquals = pchKeyEnd;
		if (*pchEquals == '=')
        {
		    //	get value
		    char* pchValueStart = pchEquals + 1;;

		    //	skip whitespace
		    while ((*pchValueStart == ' ') || (*pchValueStart == '\t') || (*pchValueStart == '\n') || (*pchValueStart == '\r'))
			    pchValueStart++;

		    //	must be a quote (" or ')
		    char* pchOpenQuote = pchValueStart;
		    if ((*pchOpenQuote == '\'') || (*pchOpenQuote == '\"'))
            {
 		        //	get close quote
		        pchCloseQuote = strchr(pchOpenQuote + 1, *pchOpenQuote);

                if (pchCloseQuote)
                {
		            //	length of string to copy
		            int iStart = (int)(pchOpenQuote - pchData) + 1;
		            int iEnd = (int)(pchCloseQuote - pchData);

		            std::string strData(pchData);

		            strValue = strData.substr(iStart, iEnd - iStart);

                    return true;
                }
                else
                    pchCurr = pchOpenQuote;
            }
            else
                pchCurr = pchOpenQuote;
        }
        else
            pchCurr = pchEquals;

        pchKeyStart = strstr(pchCurr, pchKey);
    }

    return false;
}


//------------------------------------------------------------------------------
//  GetParentComponentData
//------------------------------------------------------------------------------
ComponentData* AudioDataFile::GetParentComponentData(xml::Node* pNode)
{
	std::string strParentName;
	PeekValueFromKey(pNode->GetParent()->GetAttributes(), "Name", strParentName);

	char strFullName[256] = {0};

	if (strlen(m_GroupNameManager.GetCurrentGroupName()))
		sprintf(strFullName, "%s.%s", m_GroupNameManager.GetCurrentGroupName(), strParentName.data());
	else
		sprintf(strFullName, "%s", strParentName.data());

	U32 u32CompIndex = GetStringIndex(strFullName);

	if (u32CompIndex != -1)
		return m_apComponentData[u32CompIndex];

	return 0;
}


//------------------------------------------------------------------------------
//  ConvertNodeToAudioData
//------------------------------------------------------------------------------
void AudioDataFile::ConvertNodeToAudioData(xml::Node* pNode)
{
    xml::TagNode* pTag = dynamic_cast<xml::TagNode*>(pNode);
    xml::TextNode* pText = dynamic_cast<xml::TextNode*>(pNode);
    xml::CommentNode* pComment = dynamic_cast<xml::CommentNode*>(pNode);

    for (U32 i = 0; i < m_aStrictParseAttrNames.Size(); i++)
	{
		m_aStrictParseAttrNames[i]->clear();
        delete m_aStrictParseAttrNames[i];
		m_aStrictParseAttrNames[i] = 0;
	}

    m_aStrictParseAttrNames.Clear();

    if ((pTag) && (strcmp(pTag->GetType(), "Group") == 0))
    {
        //  add group prefix
        std::string strGroup;
        if (pTag->GetAttributes() && GetValueFromKey(pTag->GetAttributes(), "Name", strGroup))
        {
			m_GroupNameManager.AddGroupLevel((char*)(strGroup.data()));
            //m_strCurrGroup += strGroup;
            //m_strCurrGroup += ".";
        }

        for (Audio::Iterator<xml::Node*> i = pTag->GetChildren().first(); !i.isPastEnd(); i.advance())
		{
			if (! m_Diagnostics.CanContinue())
				return;

			//	recurse
			ConvertNodeToAudioData(i.retrieve());

//jkc			if (pChildBlock)
//jkc				pParentBlock->AddChild(pChildBlock);
		}

        //  remove group prefix
        if (pTag->GetAttributes() && GetValueFromKey(pTag->GetAttributes(), "Name", strGroup))
			m_GroupNameManager.RemoveGroupLevel();
//            m_strCurrGroup.resize(m_strCurrGroup.length() - strGroup.length() - 1);
    }
    else
    {
	    if (pTag)
	    {
 		    const char* pchName = pTag->GetType();
		    const char* pchAttributes = pTag->GetAttributes();

			if (FoundTag(pTag))
			{
			    //	process attributes
			    if (pchAttributes)
			    {
					//	for what used to be called strict parsing..
					if (strcmp(pchName, "Constant") != 0)
						CreateValueListFromAttributeBlock(pchAttributes);

					// FoundTagAttributes creates the audio data structure.. jkc

				    FoundTagAttributes(pTag, pchName, pchAttributes);
			    }
				else if (strcmp(pchName, "Object") == 0)
					DoError("Object has no Name attribute", pTag->GetLine(), pTag->GetFilename(), true);

				if (! m_Diagnostics.CanContinue())
					return;

				if (pchAttributes)
				{
					if (m_aStrictParseAttrNames.Size())
					{
						char strError[1024] = {0};
						sprintf(strError, "found unrecognised attribute '%s'", m_aStrictParseAttrNames[0]->data());
						DoWarning(strError, pTag->GetLine(), pTag->GetFilename());
					}
				}

                for (Audio::Iterator<xml::Node*> i = pTag->GetChildren().first(); !i.isPastEnd(); i.advance())
			    {
					if (! m_Diagnostics.CanContinue())
						return;

				    //	recurse
				    ConvertNodeToAudioData(i.retrieve());
			    }
		    }

            EndNode(pTag);
	    }
	}
}


//------------------------------------------------------------------------------
//  FoundTag
//------------------------------------------------------------------------------
bool AudioDataFile::FoundTag(xml::TagNode* pTag)
{
    if (strcmp(pTag->GetType(), "Include") == 0
		|| strcmp(pTag->GetType(), "Messages") == 0
		|| strcmp(pTag->GetType(), "Parameters") == 0)
    {
        return false;
    }
    else if (strcmp(pTag->GetType(), "Group") == 0)
    {
        if ((strcmp(pTag->GetParent()->GetType(), "Object") == 0) ||
            (strcmp(pTag->GetParent()->GetType(), "Group") == 0))
        {
            std::string strGroup;

            //  add group name to component prefix
            if (pTag->GetAttributes() && GetValueFromKey(pTag->GetAttributes(), "Name", strGroup))
            {
				m_GroupNameManager.AddGroupLevel((char*)(strGroup.data()));
                //m_strCurrGroup += strGroup;
                //m_strCurrGroup += ".";
            }

//jkc            return XmlDataFile::FoundTag(pTag, pParentBlock);
            return false;	// ..jkc
        }
        else
        {
            //  wrong parent type
            DoError("Group tag has invalid parent", pTag->GetLine(), pTag->GetFilename());
            return false;
        }
    }

	return true;
}


//------------------------------------------------------------------------------
//  FoundTagAttributes
//------------------------------------------------------------------------------
void AudioDataFile::FoundTagAttributes(xml::TagNode* pParentNode, const char* pchType, const char* pchAttributes)
{
	m_pchType = pchType;
	m_pchAttributes = pchAttributes;
	m_pParentNode = pParentNode;

 	if (strcmp(pchType, "Group") == 0)
    {
        //  ignore group name tag
        return;
    }
 	else if (strcmp(pchType, "Object") == 0)
    {
#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		std::string strValue;
        if (GetValueFromKey(pchAttributes, "Name", strValue))
		{
			m_strObjectName = new char[strlen(strValue.data()) + 1];
            strcpy(m_strObjectName, strValue.data());
			m_strObjectName[strlen(strValue.data())] = 0;
		}
        else
            DoError("Object has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename(), true);

        //  FrameSkip (opt.)
        if (GetValueFromKey(pchAttributes, "FrameSkip", strValue))
		{
			if (! GetIntValue(strValue.data(), &m_uFrameSkip))
				Error("Object FrameSkip is not an integer.");
		}
#endif // AUDIO_INCLUDE_NAMES

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoOneShotSound
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoOneShotSound"))
    {
		Audio::MonoOneShotSound::Data* pInitData = new Audio::MonoOneShotSound::Data;
		Audio::MonoOneShotSound::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  Name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoOneShotSound has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

        //  InstancesNum (opt.)
		GetUnsignedIntAttribute("InstancesNum", &(pInitData->uInstances));

        //  wave
		GetBankAttribute(&(pInitData->strBank));

		GetNonComponentStringOffsetAttribute("Wave", &(pInitData->strWave));

        //  waveCom (req.)
		//if (! GetComponentAttribute("WaveCom", &(pInitData->sWaveCom), 1, Audio::Component::eMonoOneShotWaveform))
        //    DoError("MonoOneShotSound has no waveCom parameter", pParentNode->GetLine(), pParentNode->GetFilename());


        //	Pitch (opt.)
		GetComponentFloatOutputAttributes("Pitch", &(pInitData->optPitch));

        //	Frequency (opt.)
		GetComponentFloatOutputAttributes("Frequency", &(pInitData->optFrequency));

		//	Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain)); 

		//	Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel)); 

        //	FinishedScr (opt.)
#ifndef NEW_EVENT_CODE
		GetComponentAttribute("FinishedScr", &(pInitData->sFinishedScr));
#else
		LookupEvent("Finished", pInitData->evFinished);
#endif

		SetComponentData(pInitData, sizeof(Audio::MonoOneShotSound::Data), Audio::Component::eMonoOneShotSound);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoOneShotSoundTable
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoOneShotSoundTable"))
    {
		Audio::MonoOneShotSoundTable::Data* pInitData = new Audio::MonoOneShotSoundTable::Data;
		Audio::MonoOneShotSoundTable::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoOneShotSound has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

        //  instances (opt.)
		GetUnsignedIntAttribute("InstancesNum", &(pInitData->uInstances));

        //  bank
		GetBankAttribute(&(pInitData->strBank));

        //  index (opt.)
		GetComponentFloatOutputAttributes("Index", &(pInitData->optIndex));

        //	Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

        //	Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

        //	Pitch (opt.)
		GetComponentFloatOutputAttributes("Pitch", &(pInitData->optPitch));

        //	Frequency (opt.)
		GetComponentFloatOutputAttributes("Frequency", &(pInitData->optFrequency));

        //	FinishedScr (opt.)
#ifndef NEW_EVENT_CODE
		GetComponentAttribute("FinishedScr", &(pInitData->sFinishedScr));
#else
		LookupEvent("Finished", pInitData->evFinished);
#endif

		SetComponentData(pInitData, sizeof(Audio::MonoOneShotSoundTable::Data), Audio::Component::eMonoOneShotSoundTable);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoOneShotSoundTableEntry
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (strcmp(pchType, "MonoOneShotSoundTableEntry") == 0)
	{
        Audio::MonoOneShotSoundTable::InputWave inputWave;// = new Audio::MonoOneShotSoundTable::InputWave;
        inputWave.strWave = -1;

		// WaveCom com (req).
		//if (! GetComponentAttribute("WaveCom", &(inputWave.sWaveCom)))
		//	DoError("MonoOneShotSoundTable has no WaveCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        //  wave
		GetNonComponentStringOffsetAttribute("Wave", &(inputWave.strWave));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for MonoOneShotSoundTableEntry", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eMonoOneShotSoundTable:
				{
					Audio::MonoOneShotSoundTable::Data* pParentData = (Audio::MonoOneShotSoundTable::Data*)(pCompData->GetData());
					pParentData->uWaves++;
				}
				break;
			default:
				DoError("MonoOneShotSoundTableEntry has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeDefault, &inputWave, sizeof(Audio::MonoOneShotSoundTable::InputWave));

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoOneShotSoundTable
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("StereoOneShotSoundTable"))
    {
		Audio::StereoOneShotSoundTable::Data* pInitData = new Audio::StereoOneShotSoundTable::Data;
		Audio::StereoOneShotSoundTable::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoOneShotSound has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

        //  instances (opt.)
		GetUnsignedIntAttribute("InstancesNum", &(pInitData->uInstances));

        //  bank
		GetBankAttribute(&(pInitData->strBank));

        //  index (opt.)
		GetComponentFloatOutputAttributes("Index", &(pInitData->optIndex));

        //	Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

        //	Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

        //	Pitch (opt.)
		GetComponentFloatOutputAttributes("Pitch", &(pInitData->optPitch));

        //	Frequency (opt.)
		GetComponentFloatOutputAttributes("Frequency", &(pInitData->optFrequency));

        //	FinishedScr (opt.)
#ifndef NEW_EVENT_CODE
		GetComponentAttribute("FinishedScr", &(pInitData->sFinishedScr));
#else
		LookupEvent("Finished", pInitData->evFinished);
#endif

		SetComponentData(pInitData, sizeof(Audio::StereoOneShotSoundTable::Data), Audio::Component::eStereoOneShotSoundTable);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoOneShotSoundTableEntry
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (strcmp(pchType, "StereoOneShotSoundTableEntry") == 0)
	{
        Audio::StereoOneShotSoundTable::InputWave inputWave;// = new Audio::StereoOneShotSoundTable::InputWave;
        inputWave.strWave = -1;

		// WaveCom com (req).
		//if (! GetComponentAttribute("WaveCom", &(inputWave.sWaveCom)))
		//	DoError("StereoOneShotSoundTable has no WaveCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        //  wave
		GetNonComponentStringOffsetAttribute("Wave", &(inputWave.strWave));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for StereoOneShotSoundTableEntry", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eStereoOneShotSoundTable:
				{
					Audio::StereoOneShotSoundTable::Data* pParentData = (Audio::StereoOneShotSoundTable::Data*)(pCompData->GetData());
					pParentData->uWaves++;
				}
				break;
			default:
				DoError("StereoOneShotSoundTableEntry has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeDefault, &inputWave, sizeof(Audio::StereoOneShotSoundTable::InputWave));

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoLoopingSound
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoLoopingSound"))
    {
		Audio::MonoLoopingSound::Data* pInitData = new Audio::MonoLoopingSound::Data;
		Audio::MonoLoopingSound::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoLoopingSound has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

        //  wave
		GetBankAttribute(&(pInitData->strBank));

		GetNonComponentStringOffsetAttribute("Wave", &(pInitData->strWave));

        //  waveCom (req.)
		//if (! GetComponentAttribute("WaveCom", &(pInitData->sWaveCom), 1, Audio::Component::eMonoLoopingWaveform))
        //    DoError("MonoLoopingSound has no waveCom parameter", pParentNode->GetLine(), pParentNode->GetFilename());

		//	Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		//	Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		//	Frequency (opt.)
		GetComponentFloatOutputAttributes("Frequency", &(pInitData->optFrequency));

		//	Pitch (opt.)
		GetComponentFloatOutputAttributes("Pitch", &(pInitData->optPitch));

		//	LoopedScr (opt.)
#ifndef NEW_EVENT_CODE
		GetComponentAttribute("LoopedScr", &(pInitData->sLoopedScr));
#else
		LookupEvent("Looped", pInitData->evLooped);
#endif

		//	Offset (opt.)
		GetComponentFloatOutputAttributes("Offset", &(pInitData->optOffset));

		SetComponentData(pInitData, sizeof(Audio::MonoLoopingSound::Data), Audio::Component::eMonoLoopingSound);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoAudioOutput
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoAudioOutput"))
	{
		Audio::MonoAudioOutput::Data* pInitData = new Audio::MonoAudioOutput::Data;
		Audio::MonoAudioOutput::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoAudioOutput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// input com (req.)
		if (! GetComponentAttribute("InputCom", &(pInitData->sInputCom), 7, Audio::Component::eMonoAudioInput, Audio::Component::eMonoLoopingSound, Audio::Component::eMonoOneShotSound, Audio::Component::eMonoOneShotSoundTable, Audio::Component::eMonoToMonoMixer, Audio::Component::eStereoToMonoMixer, Audio::Component::eSurroundToMonoMixer))
			DoError("MonoAudioOutput has no inputCom parameter", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::MonoAudioOutput::Data), Audio::Component::eMonoAudioOutput);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoLowPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoLowPass"))
	{
		Audio::MonoLowPass::Data* pInitData = new Audio::MonoLowPass::Data;
		Audio::MonoLowPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoLowPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoLowPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoLowPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoLowPass::Data), Audio::Component::eMonoLowPass);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoHighPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoHighPass"))
	{
		Audio::MonoHighPass::Data* pInitData = new Audio::MonoHighPass::Data;
		Audio::MonoHighPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoHighPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoHighPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoHighPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoHighPass::Data), Audio::Component::eMonoHighPass);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoResonantLowPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoResonantLowPass"))
	{
		Audio::MonoResonantLowPass::Data* pInitData = new Audio::MonoResonantLowPass::Data;
		Audio::MonoResonantLowPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoResonantLowPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// Q (opt.)
		GetComponentFloatOutputAttributes("Q", &(pInitData->optQ));

		// gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// pitch shift (opt.)
		GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift (opt.)
		GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// clamp (opt.)
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoResonantLowPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoResonantLowPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoResonantLowPass::Data), Audio::Component::eMonoResonantLowPass);

		return;
	}

////////////////////////////////////////////////////////////////////////////////////////////
//
//	Mono24dBResonantLowPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("Mono24dBResonantLowPass"))
	{
		Audio::Mono24dBResonantLowPass::Data* pInitData = new Audio::Mono24dBResonantLowPass::Data;
		Audio::Mono24dBResonantLowPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Mono24dBResonantLowPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// Q (opt.)
		GetComponentFloatOutputAttributes("Q", &(pInitData->optQ));

		// gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// pitch shift (opt.)
		GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift (opt.)
		GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// clamp (opt.)
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for Mono24dBResonantLowPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("Mono24dBResonantLowPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::Mono24dBResonantLowPass::Data), Audio::Component::eMono24dBResonantLowPass);

		return;
	}

////////////////////////////////////////////////////////////////////////////////////////////
//
//	Surround24dBResonantLowPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("Surround24dBResonantLowPass"))
	{
		Audio::Surround24dBResonantLowPass::Data* pInitData = new Audio::Surround24dBResonantLowPass::Data;
		Audio::Surround24dBResonantLowPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Surround24dBResonantLowPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// Q (opt.)
		GetComponentFloatOutputAttributes("Q", &(pInitData->optQ));

		// gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// pitch shift (opt.)
		GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift (opt.)
		GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// clamp (opt.)
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for Surround24dBResonantLowPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("Surround24dBResonantLowPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::Surround24dBResonantLowPass::Data), Audio::Component::eSurround24dBResonantLowPass);

		return;
	}

////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoResonantLowPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("StereoResonantLowPass"))
	{
		Audio::StereoResonantLowPass::Data* pInitData = new Audio::StereoResonantLowPass::Data;
		Audio::StereoResonantLowPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoResonantLowPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// Q (opt.)
		GetComponentFloatOutputAttributes("Q", &(pInitData->optQ));

		// gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// pitch shift (opt.)
		GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift (opt.)
		GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// clamp (opt.)
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoResonantLowPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoResonantLowPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoResonantLowPass::Data), Audio::Component::eStereoResonantLowPass);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//	SurroundResonantLowPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("SurroundResonantLowPass"))
	{
		Audio::SurroundResonantLowPass::Data* pInitData = new Audio::SurroundResonantLowPass::Data;
		Audio::SurroundResonantLowPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundResonantLowPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// Q (opt.)
		GetComponentFloatOutputAttributes("Q", &(pInitData->optQ));

		// gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// pitch shift (opt.)
		GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift (opt.)
		GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// clamp (opt.)
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundResonantLowPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundResonantLowPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundResonantLowPass::Data), Audio::Component::eSurroundResonantLowPass);

		return;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundRMSLevel
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("SurroundRMSLevel"))
	{
		Audio::SurroundRMSLevel::Data* pInitData = new Audio::SurroundRMSLevel::Data;
		Audio::SurroundRMSLevel::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundRMSLevel has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));
		
		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundRMSLevel", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundRMSLevel has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundRMSLevel::Data), Audio::Component::eSurroundRMSLevel);

		return;
	}
//////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoLowShelf
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoLowShelf"))
	{
		Audio::MonoLowShelf::Data* pInitData = new Audio::MonoLowShelf::Data;
		Audio::MonoLowShelf::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoLowShelf has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// slope (opt.)
        GetComponentFloatOutputAttributes("Slope", &(pInitData->optSlope));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// clamp (opt.)
        GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoLowShelf", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoLowShelf has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoLowShelf::Data), Audio::Component::eMonoLowShelf);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoHighShelf
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoHighShelf"))
	{
		Audio::MonoHighShelf::Data* pInitData = new Audio::MonoHighShelf::Data;
		Audio::MonoHighShelf::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoHighShelf has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// slope component (opt.)
        GetComponentFloatOutputAttributes("Slope", &(pInitData->optSlope));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// clamp (opt.)
        GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoHighShelf", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoHighShelf has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoHighShelf::Data), Audio::Component::eMonoHighShelf);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoLipSync
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoLipSync"))
	{
		Audio::MonoLipSync::Data* pInitData = new Audio::MonoLipSync::Data;
		Audio::MonoLipSync::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoLipSync has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		GetComponentFloatOutputAttributes("BlendRMS", &(pInitData->optBlendRMS));

		GetComponentFloatOutputAttributes("BlendPitch", &(pInitData->optBlendPitch));

		GetComponentFloatOutputAttributes("SilenceThreshold", &(pInitData->optSilenceThreshold));

		GetComponentFloatOutputAttributes("ClosedMouthWidth", &(pInitData->optClosedMouthWidth));

		GetComponentFloatOutputAttributes("SibPostScale", &(pInitData->optSibPostScale));

		if (! pCompData)
			DoError("Can't get parent for MonoLipSync", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoLipSync has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoLipSync::Data), Audio::Component::eMonoLipSync);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoSpectralReverb
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoSpectralReverb"))
	{
		Audio::MonoSpectralReverb::Data* pInitData = new Audio::MonoSpectralReverb::Data;
		Audio::MonoSpectralReverb::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoSpectralReverb has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		GetComponentFloatOutputAttributes("Band1_DecayTime", &(pInitData->optBand1_DecayTime));

		GetComponentFloatOutputAttributes("Band2_DecayTime", &(pInitData->optBand2_DecayTime));

		GetComponentFloatOutputAttributes("Band3_DecayTime", &(pInitData->optBand3_DecayTime));

		GetComponentFloatOutputAttributes("Band4_DecayTime", &(pInitData->optBand4_DecayTime));

		GetComponentFloatOutputAttributes("Band5_DecayTime", &(pInitData->optBand5_DecayTime));

		GetComponentFloatOutputAttributes("Band6_DecayTime", &(pInitData->optBand6_DecayTime));

		GetComponentFloatOutputAttributes("Master_DecayTime", &(pInitData->optMaster_DecayTime));

		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		GetComponentFloatOutputAttributes("SmoothFilter", &(pInitData->optSmoothFilter));

		if (! pCompData)
			DoError("Can't get parent for MonoSpectralReverb", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoSpectralReverb has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoSpectralReverb::Data), Audio::Component::eMonoSpectralReverb);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoSpectralEnvelope
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoSpectralEnvelope"))
	{
		Audio::MonoSpectralEnvelope::Data* pInitData = new Audio::MonoSpectralEnvelope::Data;
		Audio::MonoSpectralEnvelope::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoSpectralEnvelope has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoSpectralEnvelope", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoSpectralEnvelope has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoSpectralEnvelope::Data), Audio::Component::eMonoSpectralEnvelope);

		return;
	}

////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoSpectralEnvelope
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("StereoSpectralEnvelope"))
	{
		Audio::StereoSpectralEnvelope::Data* pInitData = new Audio::StereoSpectralEnvelope::Data;
		Audio::StereoSpectralEnvelope::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoSpectralEnvelope has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoSpectralEnvelope", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoSpectralEnvelope has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoSpectralEnvelope::Data), Audio::Component::eStereoSpectralEnvelope);

		return;
	}

////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoRMSLevel
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoRMSLevel"))
	{
		Audio::MonoRMSLevel::Data* pInitData = new Audio::MonoRMSLevel::Data;
		Audio::MonoRMSLevel::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoRMSLevel has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));
		
		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoRMSLevel", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoRMSLevel has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoRMSLevel::Data), Audio::Component::eMonoRMSLevel);

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoOscilloscope
//
////////////////////////////////////////////////////////////////////////////////////////////
	if (GetComponent("MonoOscilloscope"))
	{
		Audio::MonoOscilloscope::Data* pInitData = new Audio::MonoOscilloscope::Data;
		Audio::MonoOscilloscope::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoOscilloscope has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));

		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));
		
		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoOscilloscope", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoOscilloscope has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoOscilloscope::Data), Audio::Component::eMonoOscilloscope);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoPeakLevel
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoPeakLevel"))
	{
		Audio::MonoPeakLevel::Data* pInitData = new Audio::MonoPeakLevel::Data;
		Audio::MonoPeakLevel::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoPeakLevel has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoPeakLevel", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoPeakLevel has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoPeakLevel::Data), Audio::Component::eMonoPeakLevel);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoDelay
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoDelay"))
	{
		Audio::MonoDelay::Data* pInitData = new Audio::MonoDelay::Data;
		Audio::MonoDelay::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoDelay has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetFloatAttribute("MaxLength", &(pInitData->fMaxLength));

		GetFloatAttribute("DownSample", &(pInitData->fDownSample));

		// length component (opt.)
        GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));

		// wet component (opt.)
        GetComponentFloatOutputAttributes("Wet", &(pInitData->optWet));

		// dry component (opt.)
        GetComponentFloatOutputAttributes("Dry", &(pInitData->optDry));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoDelay", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoDelay has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoDelay::Data), Audio::Component::eMonoDelay);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoDelay
//
////////////////////////////////////////////////////////////////////////////////////////////
//	else if (GetComponent("StereoDelay"))
//	{
//		Audio::StereoDelay::Data* pInitData = new Audio::StereoDelay::Data;
//		Audio::StereoDelay::DefaultData(pInitData);
//
//#ifdef AUDIO_INCLUDE_NAMES
//        //  name (req.)
//		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
//            DoError("StereoDelay has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
//#endif // AUDIO_INCLUDE_NAMES
//
//		// length component (opt.)
//        GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));
//
//		ComponentData* pCompData = GetParentComponentData(pParentNode);
//
//		if (! pCompData)
//			DoError("Can't get parent for StereoDelay", pParentNode->GetLine(), pParentNode->GetFilename());
//
//		if (! CheckMonoAndIncEffects(pCompData))
//			DoError("StereoDelay has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
//
//		// TODO: make enum for 1st param (id), eg. effect, input
//		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));
//
//		SetComponentData(pInitData, sizeof(Audio::StereoDelay::Data), Audio::Component::eStereoDelay);
//	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoFeedbackDelay
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoFeedbackDelay"))
	{
		Audio::MonoFeedbackDelay::Data* pInitData = new Audio::MonoFeedbackDelay::Data;
		Audio::MonoFeedbackDelay::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoFeedbackDelay has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		//	max length (opt.)
		GetFloatAttribute("MaxLengthVal", &(pInitData->fMaxLength));

		// length component (opt.)
        GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));

		// feedback component (opt.)
        GetComponentFloatOutputAttributes("Feedback", &(pInitData->optFeedback));

		// wet component (opt.)
        GetComponentFloatOutputAttributes("WetGain", &(pInitData->optWetGain));
        GetComponentFloatOutputAttributes("WetLevel", &(pInitData->optWetLevel));

		// dry component (opt.)
        GetComponentFloatOutputAttributes("DryGain", &(pInitData->optDryGain));
        GetComponentFloatOutputAttributes("DryLevel", &(pInitData->optDryLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoFeedbackDelay", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoFeedbackDelay has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoFeedbackDelay::Data), Audio::Component::eMonoFeedbackDelay);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoFeedbackDelay
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoFeedbackDelay"))
	{
		Audio::StereoFeedbackDelay::Data* pInitData = new Audio::StereoFeedbackDelay::Data;
		Audio::StereoFeedbackDelay::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoFeedbackDelay has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		//	max length (opt.)
		GetFloatAttribute("MaxLengthVal", &(pInitData->fMaxLength));

		// length component (opt.)
        GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));

		// feedback component (opt.)
        GetComponentFloatOutputAttributes("Feedback", &(pInitData->optFeedback));

		// wet component (opt.)
        GetComponentFloatOutputAttributes("Wet", &(pInitData->optWet));

		// dry component (opt.)
        GetComponentFloatOutputAttributes("Dry", &(pInitData->optDry));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoFeedbackDelay", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoFeedbackDelay has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoFeedbackDelay::Data), Audio::Component::eStereoFeedbackDelay);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundDelay
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundDelay"))
	{
		Audio::SurroundDelay::Data* pInitData = new Audio::SurroundDelay::Data;
		Audio::SurroundDelay::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundDelay has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// length component (opt.)
        GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundDelay", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundDelay has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundDelay::Data), Audio::Component::eSurroundDelay);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoGain
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoGain"))
	{
		Audio::MonoGain::Data* pInitData = new Audio::MonoGain::Data;
		Audio::MonoGain::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoGain has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// gain component (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoGain", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoGain has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoGain::Data), Audio::Component::eMonoGain);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoPhaser
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoPhaser"))
	{
		Audio::MonoPhaser::Data* pInitData = new Audio::MonoPhaser::Data;
		Audio::MonoPhaser::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoPhaser has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// dry component (opt.)
        GetComponentFloatOutputAttributes("Dry", &(pInitData->optDry));

		// Wet component (opt.)
        GetComponentFloatOutputAttributes("Wet", &(pInitData->optWet));

		// Angle component (opt.)
        GetComponentFloatOutputAttributes("Angle", &(pInitData->optAngle));

		// Speed component (opt.)
        GetComponentFloatOutputAttributes("Speed", &(pInitData->optSpeed));

		// Width component (opt.)
        GetComponentFloatOutputAttributes("Width", &(pInitData->optWidth));

		// Feedback component (opt.)
        GetComponentFloatOutputAttributes("Feedback", &(pInitData->optFeedback));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoPhaser", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoPhaser has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoPhaser::Data), Audio::Component::eMonoPhaser);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoFlanger
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoFlanger"))
	{
		Audio::MonoFlanger::Data* pInitData = new Audio::MonoFlanger::Data;
		Audio::MonoFlanger::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoFlanger has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// Delay component (opt.)
        GetComponentFloatOutputAttributes("Delay", &(pInitData->optDelay));

		// Depth component (opt.)
        GetComponentFloatOutputAttributes("Depth", &(pInitData->optDepth));

		// Rate component (opt.)
        GetComponentFloatOutputAttributes("Rate", &(pInitData->optRate));

		// Feedback component (opt.)
        GetComponentFloatOutputAttributes("Feedback", &(pInitData->optFeedback));

		// dry component (opt.)
        GetComponentFloatOutputAttributes("Dry", &(pInitData->optDry));

		// Wet component (opt.)
        GetComponentFloatOutputAttributes("Wet", &(pInitData->optWet));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoFlanger", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoFlanger has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoFlanger::Data), Audio::Component::eMonoFlanger);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundFlanger
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundFlanger"))
	{
		Audio::SurroundFlanger::Data* pInitData = new Audio::SurroundFlanger::Data;
		Audio::SurroundFlanger::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundFlanger has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// Delay component (opt.)
        GetComponentFloatOutputAttributes("Delay", &(pInitData->optDelay));

		// Depth component (opt.)
        GetComponentFloatOutputAttributes("Depth", &(pInitData->optDepth));

		// Rate component (opt.)
        GetComponentFloatOutputAttributes("Rate", &(pInitData->optRate));

		// Feedback component (opt.)
        GetComponentFloatOutputAttributes("Feedback", &(pInitData->optFeedback));

		// dry component (opt.)
        GetComponentFloatOutputAttributes("Dry", &(pInitData->optDry));

		// Wet component (opt.)
        GetComponentFloatOutputAttributes("Wet", &(pInitData->optWet));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundFlanger", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundFlanger has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundFlanger::Data), Audio::Component::eSurroundFlanger);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoPeakEQ
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoPeakEQ"))
	{
		Audio::MonoPeakEQ::Data* pInitData = new Audio::MonoPeakEQ::Data;
		Audio::MonoPeakEQ::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoPeakEQ has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Centre", &(pInitData->optCentre)); 
		GetComponentFloatOutputAttributes("Q", &(pInitData->optQ));
		GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));
		GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoPeakEQ", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoPeakEQ has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoPeakEQ::Data), Audio::Component::eMonoPeakEQ);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoSaturator
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoSaturator"))
	{
		Audio::MonoSaturator::Data* pInitData = new Audio::MonoSaturator::Data;
		Audio::MonoSaturator::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoSaturator has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Drive", &(pInitData->optDrive));
		GetComponentFloatOutputAttributes("Clip", &(pInitData->optClip));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoSaturator", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoSaturator has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoSaturator::Data), Audio::Component::eMonoSaturator);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoBitCrusher
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoBitCrusher"))
	{
		Audio::MonoBitCrusher::Data* pInitData = new Audio::MonoBitCrusher::Data;
		Audio::MonoBitCrusher::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoBitCrusher has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Depth", &(pInitData->optDepth));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoBitCrusher", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoBitCrusher has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoBitCrusher::Data), Audio::Component::eMonoBitCrusher);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundBitCrusher
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundBitCrusher"))
	{
		Audio::SurroundBitCrusher::Data* pInitData = new Audio::SurroundBitCrusher::Data;
		Audio::SurroundBitCrusher::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundBitCrusher has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Depth", &(pInitData->optDepth));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundBitCrusher", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundBitCrusher has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundBitCrusher::Data), Audio::Component::eSurroundBitCrusher);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoRingModulator
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoRingModulator"))
	{
		Audio::MonoRingModulator::Data* pInitData = new Audio::MonoRingModulator::Data;
		Audio::MonoRingModulator::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoRingModulator has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Rate", &(pInitData->optRate));
		GetComponentFloatOutputAttributes("Wet", &(pInitData->optWet));
		GetComponentFloatOutputAttributes("Dry", &(pInitData->optDry));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoRingModulator", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoRingModulator has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoRingModulator::Data), Audio::Component::eMonoRingModulator);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundRingModulator
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundRingModulator"))
	{
		Audio::SurroundRingModulator::Data* pInitData = new Audio::SurroundRingModulator::Data;
		Audio::SurroundRingModulator::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundRingModulator has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Rate", &(pInitData->optRate));
		GetComponentFloatOutputAttributes("Wet", &(pInitData->optWet));
		GetComponentFloatOutputAttributes("Dry", &(pInitData->optDry));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundRingModulator", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundRingModulator has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundRingModulator::Data), Audio::Component::eSurroundRingModulator);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoCompressor
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoCompressor"))
	{
		Audio::MonoCompressor::Data* pInitData = new Audio::MonoCompressor::Data;
		Audio::MonoCompressor::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoCompressor has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentAndOutputAttributes("SideChainRMS", &(pInitData->ctrlSideChainRMS));
		GetComponentFloatOutputAttributes("Attack", &(pInitData->optAttack));
		GetComponentFloatOutputAttributes("Release", &(pInitData->optRelease));
		GetComponentFloatOutputAttributes("ThresholdGain", &(pInitData->optThresholdGain));
		GetComponentFloatOutputAttributes("ThresholdLevel", &(pInitData->optThresholdLevel));
		GetComponentFloatOutputAttributes("Ratio", &(pInitData->optRatio));
		GetComponentFloatOutputAttributes("MakeUpGain", &(pInitData->optMakeUpGain));
		GetComponentFloatOutputAttributes("MakeUpGain", &(pInitData->optMakeUpGain));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoCompressor", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoCompressor has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoCompressor::Data), Audio::Component::eMonoCompressor);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoDistortion
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoDistortion"))
	{
		Audio::MonoDistortion::Data* pInitData = new Audio::MonoDistortion::Data;
		Audio::MonoDistortion::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoDistortion has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Drive", &(pInitData->optDrive));
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));
		GetComponentFloatOutputAttributes("Slope", &(pInitData->optSlope));
		GetComponentFloatOutputAttributes("CutGain", &(pInitData->optCutGain));
		GetComponentFloatOutputAttributes("CutLevel", &(pInitData->optCutLevel));
		GetComponentFloatOutputAttributes("BoostGain", &(pInitData->optBoostGain));
		GetComponentFloatOutputAttributes("BoostLevel", &(pInitData->optBoostLevel));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoDistortion", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckMonoAndIncEffects(pCompData))
			DoError("MonoDistortion has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::MonoDistortion::Data), Audio::Component::eMonoDistortion);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoDistortion
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoDistortion"))
	{
		Audio::StereoDistortion::Data* pInitData = new Audio::StereoDistortion::Data;
		Audio::StereoDistortion::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoDistortion has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Drive", &(pInitData->optDrive));
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));
		GetComponentFloatOutputAttributes("Slope", &(pInitData->optSlope));
		GetComponentFloatOutputAttributes("CutGain", &(pInitData->optCutGain));
		GetComponentFloatOutputAttributes("CutLevel", &(pInitData->optCutLevel));
		GetComponentFloatOutputAttributes("BoostGain", &(pInitData->optBoostGain));
		GetComponentFloatOutputAttributes("BoostLevel", &(pInitData->optBoostLevel));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoDistortion", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoDistortion has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoDistortion::Data), Audio::Component::eStereoDistortion);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoSaturator
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoSaturator"))
	{
		Audio::StereoSaturator::Data* pInitData = new Audio::StereoSaturator::Data;
		Audio::StereoSaturator::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoSaturator has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Drive", &(pInitData->optDrive));
		GetComponentFloatOutputAttributes("Clip", &(pInitData->optClip));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoSaturator", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoSaturator has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoSaturator::Data), Audio::Component::eStereoSaturator);
	}

	////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoPitchShift
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoPitchShift"))
	{
		Audio::StereoPitchShift::Data* pInitData = new Audio::StereoPitchShift::Data;
		Audio::StereoPitchShift::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoPitchShift has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// pitch shift component (opt.)
       GetComponentOrFloatAttributes("PitchShift", &(pInitData->optPitchShift.ctrl.sCom), &(pInitData->optPitchShift.fVal));

	   // sequenceLengt component (opt.)
	   GetComponentOrFloatAttributes("SequenceLength", &(pInitData->optSequenceLength.ctrl.sCom), &(pInitData->optSequenceLength.fVal));
	
	     // seek window component (opt.)
	   GetComponentOrFloatAttributes("SeekWindowLength", &(pInitData->optSeekWindowLength.ctrl.sCom), &(pInitData->optSeekWindowLength.fVal));
		
	  // sequenceLengt component (opt.)
	   GetComponentOrFloatAttributes("OverlapLength", &(pInitData->optOverlapLength.ctrl.sCom), &(pInitData->optOverlapLength.fVal));
		
	   
	   //GetFloatAttribute("PitchShift", &(pInitData->fMaxLength));

		////	max length (opt.)
		//GetFloatAttribute("MaxLengthVal", &(pInitData->fMaxLength));

		//// length component (opt.)
  //      GetComponentOrFloatAttributes("Length", &(pInitData->optLength.ctrl.sCom), &(pInitData->optLength.fVal));

		//// length out (opt.)
  //      GetUnsignedIntAttribute("LengthOut", &(pInitData->optLength.ctrl.uOut));

		//// feedback component (opt.)
  //      GetComponentOrFloatAttributes("Feedback", &(pInitData->optFeedback.ctrl.sCom), &(pInitData->optFeedback.fVal));

		//// feedback out (opt.)
  //      GetUnsignedIntAttribute("FeedbackOut", &(pInitData->optFeedback.ctrl.uOut));

		//// wet component (opt.)
  //      GetComponentOrFloatAttributes("Wet", &(pInitData->optWet.ctrl.sCom), &(pInitData->optWet.fVal));

		//// wet out (opt.)
  //      GetUnsignedIntAttribute("WetOut", &(pInitData->optWet.ctrl.uOut));

		//// dry component (opt.)
  //      GetComponentOrFloatAttributes("Dry", &(pInitData->optDry.ctrl.sCom), &(pInitData->optDry.fVal));

		//// dry out (opt.)
  //      GetUnsignedIntAttribute("DryOut", &(pInitData->optDry.ctrl.uOut));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoPitchShift", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoPitchShift has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoPitchShift::Data), Audio::Component::eStereoPitchShift);
	}

////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoCompressor
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoCompressor"))
	{
		Audio::StereoCompressor::Data* pInitData = new Audio::StereoCompressor::Data;
		Audio::StereoCompressor::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoCompressor has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentAndOutputAttributes("SideChainRMS", &(pInitData->ctrlSideChainRMS));
		GetComponentFloatOutputAttributes("Attack", &(pInitData->optAttack));
		GetComponentFloatOutputAttributes("Release", &(pInitData->optRelease));
		GetComponentFloatOutputAttributes("ThresholdGain", &(pInitData->optThresholdGain));
		GetComponentFloatOutputAttributes("ThresholdLevel", &(pInitData->optThresholdLevel));
		GetComponentFloatOutputAttributes("Ratio", &(pInitData->optRatio));
		GetComponentFloatOutputAttributes("MakeUpGain", &(pInitData->optMakeUpGain));
		GetComponentFloatOutputAttributes("MakeUpLevel", &(pInitData->optMakeUpLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoCompressor", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoCompressor has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoCompressor::Data), Audio::Component::eStereoCompressor);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoLowPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoLowPass"))
	{
		Audio::StereoLowPass::Data* pInitData = new Audio::StereoLowPass::Data;
		Audio::StereoLowPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoLowPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoLowPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoLowPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoLowPass::Data), Audio::Component::eStereoLowPass);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoHighPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoHighPass"))
	{
		Audio::StereoHighPass::Data* pInitData = new Audio::StereoHighPass::Data;
		Audio::StereoHighPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoHighPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoHighPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoHighPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoHighPass::Data), Audio::Component::eStereoHighPass);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundHighPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundHighPass"))
	{
		Audio::SurroundHighPass::Data* pInitData = new Audio::SurroundHighPass::Data;
		Audio::SurroundHighPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundHighPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundHighPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundHighPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundHighPass::Data), Audio::Component::eSurroundHighPass);
	}

	////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoHighShelf
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoHighShelf"))
	{
		Audio::StereoHighShelf::Data* pInitData = new Audio::StereoHighShelf::Data;
		Audio::StereoHighShelf::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoHighShelf has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// slope component (opt.)
        GetComponentFloatOutputAttributes("Slope", &(pInitData->optSlope));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// clamp (opt.)
        GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoHighShelf", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoHighShelf has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoHighShelf::Data), Audio::Component::eStereoHighShelf);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoPeakEQ
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoPeakEQ"))
	{
		Audio::StereoPeakEQ::Data* pInitData = new Audio::StereoPeakEQ::Data;
		Audio::StereoPeakEQ::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //	name (req.)              
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoPeakEQ has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Centre", &(pInitData->optCentre)); 
		GetComponentFloatOutputAttributes("Q", &(pInitData->optQ));
		GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));
		GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoPeakEQ", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoPeakEQ has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoPeakEQ::Data), Audio::Component::eStereoPeakEQ);
	}

	////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoRMSLevel
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoRMSLevel"))
	{
		Audio::StereoRMSLevel::Data* pInitData = new Audio::StereoRMSLevel::Data;
		Audio::StereoRMSLevel::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoRMSLevel has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));
		
		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoRMSLevel", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckStereoAndIncEffects(pCompData))
			DoError("StereoRMSLevel has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::StereoRMSLevel::Data), Audio::Component::eStereoRMSLevel);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundLowPass
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundLowPass"))
	{
		Audio::SurroundLowPass::Data* pInitData = new Audio::SurroundLowPass::Data;
		Audio::SurroundLowPass::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundLowPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundLowPass", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundLowPass has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundLowPass::Data), Audio::Component::eSurroundLowPass);
	}

////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundPeakEQ
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundPeakEQ"))
	{
		Audio::SurroundPeakEQ::Data* pInitData = new Audio::SurroundPeakEQ::Data;
		Audio::SurroundPeakEQ::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundPeakEQ has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Centre", &(pInitData->optCentre)); 
		GetComponentFloatOutputAttributes("Q", &(pInitData->optQ));
		GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));
		GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundPeakEQ", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundPeakEQ has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundPeakEQ::Data), Audio::Component::eSurroundPeakEQ);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundHighShelf
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundHighShelf"))
	{
		Audio::SurroundHighShelf::Data* pInitData = new Audio::SurroundHighShelf::Data;
		Audio::SurroundHighShelf::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundHighShelf has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// slope component (opt.)
		GetComponentFloatOutputAttributes("Slope", &(pInitData->optSlope));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// clamp (opt.)
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundHighShelf", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundHighShelf has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundHighShelf::Data), Audio::Component::eSurroundHighShelf);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundLowShelf
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundLowShelf"))
	{
		Audio::SurroundLowShelf::Data* pInitData = new Audio::SurroundLowShelf::Data;
		Audio::SurroundLowShelf::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundLowShelf has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		// pitch shift component (opt.)
        GetComponentFloatOutputAttributes("PitchShift", &(pInitData->optPitchShift));

		// freq shift component (opt.)
        GetComponentFloatOutputAttributes("FreqShift", &(pInitData->optFreqShift));

		// slope component (opt.)
		GetComponentFloatOutputAttributes("Slope", &(pInitData->optSlope));

		// gain (opt.)
        GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		// level (opt.)
        GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		// clamp (opt.)
		GetComponentFloatOutputAttributes("Clamp", &(pInitData->optClamp));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundLowShelf", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundLowShelf has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundLowShelf::Data), Audio::Component::eSurroundLowShelf);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundSaturator
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundSaturator"))
	{
		Audio::SurroundSaturator::Data* pInitData = new Audio::SurroundSaturator::Data;
		Audio::SurroundSaturator::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundSaturator has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentFloatOutputAttributes("Drive", &(pInitData->optDrive));
		GetComponentFloatOutputAttributes("Clip", &(pInitData->optClip));
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundSaturator", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundSaturator has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundSaturator::Data), Audio::Component::eSurroundSaturator);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundCompressor
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundCompressor"))
	{
		Audio::SurroundCompressor::Data* pInitData = new Audio::SurroundCompressor::Data;
		Audio::SurroundCompressor::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundCompressor has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		GetComponentAndOutputAttributes("SideChainRMS", &(pInitData->ctrlSideChainRMS));
		GetComponentFloatOutputAttributes("Attack", &(pInitData->optAttack));
		GetComponentFloatOutputAttributes("Release", &(pInitData->optRelease));
		GetComponentFloatOutputAttributes("ThresholdGain", &(pInitData->optThresholdGain));
		GetComponentFloatOutputAttributes("ThresholdLevel", &(pInitData->optThresholdLevel));
		GetComponentFloatOutputAttributes("Ratio", &(pInitData->optRatio));
		GetComponentFloatOutputAttributes("MakeUpGain", &(pInitData->optMakeUpGain));
		GetComponentFloatOutputAttributes("MakeUpLevel", &(pInitData->optMakeUpLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for SurroundCompressor", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! CheckSurroundAndIncEffects(pCompData))
			DoError("SurroundCompressor has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		SetComponentData(pInitData, sizeof(Audio::SurroundCompressor::Data), Audio::Component::eSurroundCompressor);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoToMonoMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("MonoToMonoMixer"))
    {
		Audio::MonoToMonoMixer::Data* pInitData = new Audio::MonoToMonoMixer::Data;
		Audio::MonoToMonoMixer::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoToMonoMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		SetComponentData(pInitData, sizeof(Audio::MonoToMonoMixer::Data), Audio::Component::eMonoToMonoMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoToMonoMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "MonoToMonoMixerInput") == 0)
	{
		Audio::MonoToMonoMixer::InputGain inputGain;// = new Audio::MonoToMonoMixer::InputGain;
		inputGain.optGain.fVal = 1.f;
		inputGain.optGain.ctrl.sCom = -1;
		inputGain.sInputCom = -1;
		inputGain.optGain.ctrl.uOut = 0;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom), 11,
								Audio::Component::eMonoAudioInput,
								Audio::Component::eMonoLoopingSound,
								Audio::Component::eMonoOneShotSound,
								Audio::Component::eMonoOneShotSoundTable,
								Audio::Component::eMonoSpectralToMonoMixer,
								Audio::Component::eMonoStreamingSound,
								Audio::Component::eMonoToMonoMixer,
								Audio::Component::eStereoToMonoMixer,
								Audio::Component::eSurroundToMonoMixer,
								Audio::Component::eStereoSpectralToMonoMixer,
								Audio::Component::eSurroundSpectralToMonoMixer))
			DoError("MonoToMonoMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(inputGain.optGain));

		GetComponentFloatOutputAttributes("Level", &(inputGain.optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for MonoToMonoMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eMonoToMonoMixer:
				{
					Audio::MonoToMonoMixer::Data* pParentData = (Audio::MonoToMonoMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("MonoToMonoMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::MonoToMonoMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoToStereoMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("MonoToStereoMixer"))
    {
		Audio::MonoToStereoMixer::Data* pInitData = new Audio::MonoToStereoMixer::Data;
		Audio::MonoToStereoMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoToStereoMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		SetComponentData(pInitData, sizeof(Audio::MonoToStereoMixer::Data), Audio::Component::eMonoToStereoMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoToStereoMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "MonoToStereoMixerInput") == 0)
	{
		Audio::MonoToStereoMixer::InputGain inputGain;// = new Audio::MonoToStereoMixer::InputGain;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom), 11,
								Audio::Component::eMonoAudioInput,
								Audio::Component::eMonoLoopingSound,
								Audio::Component::eMonoOneShotSound,
								Audio::Component::eMonoOneShotSoundTable,
								Audio::Component::eMonoSpectralToMonoMixer,
								Audio::Component::eMonoStreamingSound,
								Audio::Component::eMonoToMonoMixer,
								Audio::Component::eStereoToMonoMixer,
								Audio::Component::eSurroundToMonoMixer,
								Audio::Component::eStereoSpectralToMonoMixer,
								Audio::Component::eSurroundSpectralToMonoMixer))
			DoError("MonoToStereoMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("LeftGain", &(inputGain.optLeftGain));
		GetComponentFloatOutputAttributes("LeftLevel", &(inputGain.optLeftLevel));
		GetComponentFloatOutputAttributes("RightGain", &(inputGain.optRightGain));
		GetComponentFloatOutputAttributes("RightLevel", &(inputGain.optRightLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for MonoToStereoMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eMonoToStereoMixer:
				{
					Audio::MonoToStereoMixer::Data* pParentData = (Audio::MonoToStereoMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("MonoToStereoMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::MonoToStereoMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoToSurroundMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("MonoToSurroundMixer"))
    {
		Audio::MonoToSurroundMixer::Data* pInitData = new Audio::MonoToSurroundMixer::Data;
		Audio::MonoToSurroundMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoToSurroundMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        std::string strValue;

		//	default flags
		Audio::MixerComponent::Flags bfFlags;
		bfFlags.nSurroundMode = (U32)Audio::MixerComponent::ePairwise;
		bfFlags.nDomain = Audio::MixerComponent::eTime;
		bfFlags.nAlwaysOn = Audio::MixerComponent::eTrue;
		bfFlags.nPad = 0;

		//	SurroundMode
        if (GetValueFromKey(m_pchAttributes, "SurroundMode", strValue))
		{
			Audio::MixerComponent::SurroundMode eSurroundMode = Audio::MixerComponent::GetSurroundMode(strValue.c_str());
			if (eSurroundMode != Audio::MixerComponent::eUndefinedSurroundMode)
				bfFlags.nSurroundMode = (U32)eSurroundMode;
		}

		//	Domain
        if (GetValueFromKey(m_pchAttributes, "Domain", strValue))
		{
			Audio::MixerComponent::Domain eDomain = Audio::MixerComponent::GetDomain(strValue.c_str());
			if (eDomain != Audio::MixerComponent::eUndefinedDomain)
				bfFlags.nDomain = (U32)eDomain;
		}

		//	AlwaysOn
        if (GetValueFromKey(m_pchAttributes, "AlwaysOn", strValue))
		{
			Audio::MixerComponent::AlwaysOn eAlwaysOn = Audio::MixerComponent::GetAlwaysOn(strValue.c_str());
			if (eAlwaysOn != Audio::MixerComponent::eUndefinedAlwaysOn)
				bfFlags.nDomain = (U32)eAlwaysOn;
		}

		//	endian swap flags
		pInitData->bfFlags.uFlags = CommandLineApp::GetApp()->DoEndian(*(U32*)&(bfFlags));

		//  gain
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		//  level
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));
		//  azimuth
		GetComponentFloatOutputAttributes("Azimuth", &(pInitData->optAzimuth));
		//  elevation
		GetComponentFloatOutputAttributes("Elevation", &(pInitData->optElevation));
		//  Width
		GetComponentFloatOutputAttributes("Width", &(pInitData->optWidth));
		//  Distance
		GetComponentFloatOutputAttributes("Distance", &(pInitData->optDistance));

		SetComponentData(pInitData, sizeof(Audio::MonoToSurroundMixer::Data), Audio::Component::eMonoToSurroundMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoToSurroundMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "MonoToSurroundMixerInput") == 0)
	{
		Audio::MonoToSurroundMixer::InputGain inputGain;// = new Audio::MonoToSurroundMixer::InputGain;
		inputGain.optGain.fVal = 1.f;
		inputGain.optGain.ctrl.sCom = -1;
		inputGain.sInputCom = -1;
		inputGain.optGain.ctrl.uOut = 0;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom), 11,
								Audio::Component::eMonoAudioInput,
								Audio::Component::eMonoLoopingSound,
								Audio::Component::eMonoOneShotSound,
								Audio::Component::eMonoOneShotSoundTable,
								Audio::Component::eMonoSpectralToMonoMixer,
								Audio::Component::eMonoStreamingSound,
								Audio::Component::eMonoToMonoMixer,
								Audio::Component::eStereoToMonoMixer,
								Audio::Component::eSurroundToMonoMixer,
								Audio::Component::eStereoSpectralToMonoMixer,
								Audio::Component::eSurroundSpectralToMonoMixer))
			DoError("MonoToSurroundMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(inputGain.optGain));

		// Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(inputGain.optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoToSurroundMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());

		switch (pCompData->GetType())
		{
			case Component::eMonoToSurroundMixer:
				{
					Audio::MonoToSurroundMixer::Data* pParentData = (Audio::MonoToSurroundMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("MonoToSurroundMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::MonoToSurroundMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoToSurroundLFEMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "MonoToSurroundLFEMixerInput") == 0)
	{
		Audio::MonoToSurroundLFEMixer::InputGain inputGain;// = new Audio::MonoToSurroundMixer::InputGain;
		inputGain.optGain.fVal = 1.f;
		inputGain.optGain.ctrl.sCom = -1;
		inputGain.sInputCom = -1;
		inputGain.optGain.ctrl.uOut = 0;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom), 11,
								Audio::Component::eMonoAudioInput,
								Audio::Component::eMonoLoopingSound,
								Audio::Component::eMonoOneShotSound,
								Audio::Component::eMonoOneShotSoundTable,
								Audio::Component::eMonoSpectralToMonoMixer,
								Audio::Component::eMonoStreamingSound,
								Audio::Component::eMonoToMonoMixer,
								Audio::Component::eStereoToMonoMixer,
								Audio::Component::eSurroundToMonoMixer,
								Audio::Component::eStereoSpectralToMonoMixer,
								Audio::Component::eSurroundSpectralToMonoMixer))
			DoError("MonoToSurroundLFEMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(inputGain.optGain));

		// Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(inputGain.optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for MonoToSurroundMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());

		switch (pCompData->GetType())
		{
			case Component::eMonoToSurroundLFEMixer:
				{
					Audio::MonoToSurroundLFEMixer::Data* pParentData = (Audio::MonoToSurroundLFEMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("MonoToSurroundLFEMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::MonoToSurroundLFEMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoToSurroundLFEMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("MonoToSurroundLFEMixer"))
    {
		Audio::MonoToSurroundLFEMixer::Data* pInitData = new Audio::MonoToSurroundLFEMixer::Data;
		Audio::MonoToSurroundLFEMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoToSurroundMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	TODO: get flags from xml
		pInitData->bfFlags.nSurroundMode = Audio::MixerComponent::ePairwise;
		pInitData->bfFlags.nDomain = Audio::MixerComponent::eTime;
		pInitData->bfFlags.nAlwaysOn = 0;
		pInitData->bfFlags.nPad = 0;

		//  gain
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));
		//  Level
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));
		//  azimuth
		GetComponentFloatOutputAttributes("Azimuth", &(pInitData->optAzimuth));
		//  elevation
		GetComponentFloatOutputAttributes("Elevation", &(pInitData->optElevation));
		//  Width
		GetComponentFloatOutputAttributes("Width", &(pInitData->optWidth));
		//  Distance
		GetComponentFloatOutputAttributes("Distance", &(pInitData->optDistance));
		//  lfe gain
		GetComponentFloatOutputAttributes("LFEGain", &(pInitData->optLFEGain));
		//  lfe level
		GetComponentFloatOutputAttributes("LFELevel", &(pInitData->optLFELevel));

		SetComponentData(pInitData, sizeof(Audio::MonoToSurroundLFEMixer::Data), Audio::Component::eMonoToSurroundLFEMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoOneShotSound
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoOneShotSound"))
    {
		Audio::StereoOneShotSound::Data* pInitData = new Audio::StereoOneShotSound::Data;
		Audio::StereoOneShotSound::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  Name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoOneShotSound has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

        //  InstancesNum (opt.)
		GetUnsignedIntAttribute("InstancesNum", &(pInitData->uInstances));

        //  wave
		GetBankAttribute(&(pInitData->strBank));

		GetNonComponentStringOffsetAttribute("Wave", &(pInitData->strWave));

        //  Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

        //  Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

        //  Pitch (opt.)
		GetComponentFloatOutputAttributes("Pitch", &(pInitData->optPitch));

        //  Frequency (opt.)
		GetComponentFloatOutputAttributes("Frequency", &(pInitData->optFrequency));

        //  FinishedScr (opt.)
#ifndef NEW_EVENT_CODE
		GetComponentAttribute("FinishedScr", &(pInitData->sFinishedScr));
#else
		LookupEvent("Finished", pInitData->evFinished);
#endif

		SetComponentData(pInitData, sizeof(Audio::StereoOneShotSound::Data), Audio::Component::eStereoOneShotSound);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoLoopingSound
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("StereoLoopingSound"))
    {
		Audio::StereoLoopingSound::Data* pInitData = new Audio::StereoLoopingSound::Data;
		Audio::StereoLoopingSound::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoLoopingSound has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

        //  wave
		GetBankAttribute(&(pInitData->strBank));

		GetNonComponentStringOffsetAttribute("Wave", &(pInitData->strWave));

		//	Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		//	Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		//	Pitch (opt.)
		GetComponentFloatOutputAttributes("Pitch", &(pInitData->optPitch));

		//	Frequency (opt.)
		GetComponentFloatOutputAttributes("Frequency", &(pInitData->optFrequency));

		//	LoopedScr (opt.)
#ifndef NEW_EVENT_CODE
		GetComponentAttribute("LoopedScr", &(pInitData->sLoopedScr));
#else
		LookupEvent("Looped", pInitData->evLooped);
#endif

		//	Offset (opt.)
		GetComponentFloatOutputAttributes("Offset", &(pInitData->optOffset));

		SetComponentData(pInitData, sizeof(Audio::StereoLoopingSound::Data), Audio::Component::eStereoLoopingSound);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoAudioInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoAudioInput"))
	{
		Audio::StereoAudioInput::Data* pInitData = new Audio::StereoAudioInput::Data;
		Audio::StereoAudioInput::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoAudioInput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::StereoAudioInput::Data), Audio::Component::eStereoAudioInput);		
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoAudioOutput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StereoAudioOutput"))
	{
		Audio::StereoAudioOutput::Data* pInitData = new Audio::StereoAudioOutput::Data;
		Audio::StereoAudioOutput::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoAudioOutput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// input com (req.)
		if (! GetComponentAttribute("InputCom", &(pInitData->sInputCom), 7, Audio::Component::eStereoAudioInput, Audio::Component::eStereoLoopingSound, Audio::Component::eStereoOneShotSound, Audio::Component::eStereoOneShotSoundTable, Audio::Component::eMonoToStereoMixer, Audio::Component::eStereoToStereoMixer, Audio::Component::eSurroundToStereoMixer))
			DoError("StereoAudioOutput has no inputCom parameter", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::StereoAudioOutput::Data), Audio::Component::eStereoAudioOutput);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoToMonoMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("StereoToMonoMixer"))
    {
		Audio::StereoToMonoMixer::Data* pInitData = new Audio::StereoToMonoMixer::Data;
		Audio::StereoToMonoMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoToMonoMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		SetComponentData(pInitData, sizeof(Audio::StereoToMonoMixer::Data), Audio::Component::eStereoToMonoMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoToMonoMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "StereoToMonoMixerInput") == 0)
	{
		Audio::StereoToMonoMixer::InputGain inputGain;// = new Audio::StereoToMonoMixer::InputGain;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom)))
			DoError("StereoToMonoMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Gain (opt.)
		GetComponentFloatOutputAttributes("LeftGain", &(inputGain.optLeftGain));

		GetComponentFloatOutputAttributes("LeftLevel", &(inputGain.optLeftLevel));

		GetComponentFloatOutputAttributes("RightGain", &(inputGain.optRightGain));

		GetComponentFloatOutputAttributes("RightLevel", &(inputGain.optRightLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for StereoToMonoMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eStereoToMonoMixer:
				{
					Audio::StereoToMonoMixer::Data* pParentData = (Audio::StereoToMonoMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("StereoToMonoMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::StereoToMonoMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoToStereoMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("StereoToStereoMixer"))
    {
		Audio::StereoToStereoMixer::Data* pInitData = new Audio::StereoToStereoMixer::Data;
		Audio::StereoToStereoMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoToStereoMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		SetComponentData(pInitData, sizeof(Audio::StereoToStereoMixer::Data), Audio::Component::eStereoToStereoMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoToStereoMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "StereoToStereoMixerInput") == 0)
	{
		Audio::StereoToStereoMixer::InputGain inputGain;// = new Audio::StereoToStereoMixer::InputGain;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom)))
			DoError("StereoToStereoMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("LeftToLeftGain", &(inputGain.optLeftToLeftGain));
		GetComponentFloatOutputAttributes("LeftToLeftLevel", &(inputGain.optLeftToLeftLevel));
		GetComponentFloatOutputAttributes("LeftToRightGain", &(inputGain.optLeftToRightGain));
		GetComponentFloatOutputAttributes("LeftToRightLevel", &(inputGain.optLeftToRightLevel));
		GetComponentFloatOutputAttributes("RightToLeftGain", &(inputGain.optRightToLeftGain));
		GetComponentFloatOutputAttributes("RightToLeftLevel", &(inputGain.optRightToLeftLevel));
		GetComponentFloatOutputAttributes("RightToRightGain", &(inputGain.optRightToRightGain));
		GetComponentFloatOutputAttributes("RightToRightLevel", &(inputGain.optRightToRightLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for StereoToStereoMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eStereoToStereoMixer:
				{
					Audio::StereoToStereoMixer::Data* pParentData = (Audio::StereoToStereoMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("StereoToStereoMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::StereoToStereoMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoToSurroundMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("StereoToSurroundMixer"))
    {
		Audio::StereoToSurroundMixer::Data* pInitData = new Audio::StereoToSurroundMixer::Data;
		Audio::StereoToSurroundMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoToSurroundMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        std::string strValue;

		//	default flags
		Audio::MixerComponent::Flags bfFlags;
		bfFlags.nSurroundMode = (U32)Audio::MixerComponent::ePairwise;
		bfFlags.nDomain = Audio::MixerComponent::eTime;
		bfFlags.nAlwaysOn = Audio::MixerComponent::eTrue;
		bfFlags.nPad = 0;

		//	SurroundMode
        if (GetValueFromKey(m_pchAttributes, "SurroundMode", strValue))
		{
			Audio::MixerComponent::SurroundMode eSurroundMode = Audio::MixerComponent::GetSurroundMode(strValue.c_str());
			if (eSurroundMode != Audio::MixerComponent::eUndefinedSurroundMode)
				bfFlags.nSurroundMode = (U32)eSurroundMode;
		}

		//	Domain
        if (GetValueFromKey(m_pchAttributes, "Domain", strValue))
		{
			Audio::MixerComponent::Domain eDomain = Audio::MixerComponent::GetDomain(strValue.c_str());
			if (eDomain != Audio::MixerComponent::eUndefinedDomain)
				bfFlags.nDomain = (U32)eDomain;
		}

		//	AlwaysOn
        if (GetValueFromKey(m_pchAttributes, "AlwaysOn", strValue))
		{
			Audio::MixerComponent::AlwaysOn eAlwaysOn = Audio::MixerComponent::GetAlwaysOn(strValue.c_str());
			if (eAlwaysOn != Audio::MixerComponent::eUndefinedAlwaysOn)
				bfFlags.nDomain = (U32)eAlwaysOn;
		}

		//	endian swap flags
		pInitData->bfFlags.uFlags = CommandLineApp::GetApp()->DoEndian(*(U32*)&(bfFlags));

		//  gain
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		//  Level
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		//  left azimuth
		GetComponentFloatOutputAttributes("LeftAzimuth", &(pInitData->optLeftAzimuth));
		//  left elevation
		GetComponentFloatOutputAttributes("LeftElevation", &(pInitData->optLeftElevation));
		//  left Width
		GetComponentFloatOutputAttributes("LeftWidth", &(pInitData->optLeftWidth));
		//  left Distance
		GetComponentFloatOutputAttributes("LeftDistance", &(pInitData->optLeftDistance));

		//  right azimuth
		GetComponentFloatOutputAttributes("RightAzimuth", &(pInitData->optRightAzimuth));
		//  right elevation
		GetComponentFloatOutputAttributes("RightElevation", &(pInitData->optRightElevation));
		//  right Width
		GetComponentFloatOutputAttributes("RightWidth", &(pInitData->optRightWidth));
		//  right Distance
		GetComponentFloatOutputAttributes("RightDistance", &(pInitData->optRightDistance));

		SetComponentData(pInitData, sizeof(Audio::StereoToSurroundMixer::Data), Audio::Component::eStereoToSurroundMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoToSurroundMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "StereoToSurroundMixerInput") == 0)
	{
		Audio::StereoToSurroundMixer::InputGain inputGain;// = new Audio::StereoToSurroundMixer::InputGain;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom)))
			DoError("StereoToSurroundMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Gain (opt.)
		GetComponentFloatOutputAttributes("LeftGain", &(inputGain.optLeftGain));

		// Level (opt.)
		GetComponentFloatOutputAttributes("LeftLevel", &(inputGain.optLeftLevel));

		// Gain (opt.)
		GetComponentFloatOutputAttributes("RightGain", &(inputGain.optRightGain));

		// Level (opt.)
		GetComponentFloatOutputAttributes("RightLevel", &(inputGain.optRightLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoToSurroundMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());

		switch (pCompData->GetType())
		{
			case Component::eStereoToSurroundMixer:
				{
					Audio::StereoToSurroundMixer::Data* pParentData = (Audio::StereoToSurroundMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("StereoToSurroundMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::StereoToSurroundMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoToQuadMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("StereoToQuadMixer"))
    {
		Audio::StereoToQuadMixer::Data* pInitData = new Audio::StereoToQuadMixer::Data;
		Audio::StereoToQuadMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StereoToQuadMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//  gain
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		//  level
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		SetComponentData(pInitData, sizeof(Audio::StereoToQuadMixer::Data), Audio::Component::eStereoToQuadMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StereoToQuadMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "StereoToQuadMixerInput") == 0)
	{
		Audio::StereoToQuadMixer::InputGain inputGain;// = new Audio::StereoToQuadMixer::InputGain;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom)))
			DoError("StereoToQuadMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("LeftToFrontLeftGain", &(inputGain.optLeftToFrontLeftGain));
		GetComponentFloatOutputAttributes("LeftToFrontLeftLevel", &(inputGain.optLeftToFrontLeftLevel));
		GetComponentFloatOutputAttributes("LeftToFrontRightGain", &(inputGain.optLeftToFrontRightGain));
		GetComponentFloatOutputAttributes("LeftToFrontRightLevel", &(inputGain.optLeftToFrontRightLevel));

		GetComponentFloatOutputAttributes("RightToFrontLeftGain", &(inputGain.optRightToFrontLeftGain));
		GetComponentFloatOutputAttributes("RightToFrontLeftLevel", &(inputGain.optRightToFrontLeftLevel));
		GetComponentFloatOutputAttributes("RightToFrontRightGain", &(inputGain.optRightToFrontRightGain));
		GetComponentFloatOutputAttributes("RightToFrontRightLevel", &(inputGain.optRightToFrontRightLevel));

		GetComponentFloatOutputAttributes("LeftToRearLeftGain", &(inputGain.optLeftToRearLeftGain));
		GetComponentFloatOutputAttributes("LeftToRearLeftLevel", &(inputGain.optLeftToRearLeftLevel));
		GetComponentFloatOutputAttributes("LeftToRearRightGain", &(inputGain.optLeftToRearRightGain));
		GetComponentFloatOutputAttributes("LeftToRearRightLevel", &(inputGain.optLeftToRearRightLevel));

		GetComponentFloatOutputAttributes("RightToRearLeftGain", &(inputGain.optRightToRearLeftGain));
		GetComponentFloatOutputAttributes("RightToRearLeftLevel", &(inputGain.optRightToRearLeftLevel));
		GetComponentFloatOutputAttributes("RightToRearRightGain", &(inputGain.optRightToRearRightGain));
		GetComponentFloatOutputAttributes("RightToRearRightLevel", &(inputGain.optRightToRearRightLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for StereoToQuadMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());

		switch (pCompData->GetType())
		{
			case Component::eStereoToQuadMixer:
				{
					Audio::StereoToQuadMixer::Data* pParentData = (Audio::StereoToQuadMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("StereoToQuadMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::StereoToQuadMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	QuadToSurroundMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("QuadToSurroundMixer"))
    {
		Audio::QuadToSurroundMixer::Data* pInitData = new Audio::QuadToSurroundMixer::Data;
		Audio::QuadToSurroundMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("QuadToSurroundMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        std::string strValue;

		//	default flags
		Audio::MixerComponent::Flags bfFlags;
		bfFlags.nSurroundMode = (U32)Audio::MixerComponent::ePairwise;
		bfFlags.nDomain = Audio::MixerComponent::eTime;
		bfFlags.nAlwaysOn = Audio::MixerComponent::eTrue;
		bfFlags.nPad = 0;

		//	SurroundMode
        if (GetValueFromKey(m_pchAttributes, "SurroundMode", strValue))
		{
			Audio::MixerComponent::SurroundMode eSurroundMode = Audio::MixerComponent::GetSurroundMode(strValue.c_str());
			if (eSurroundMode != Audio::MixerComponent::eUndefinedSurroundMode)
				bfFlags.nSurroundMode = (U32)eSurroundMode;
		}

		//	Domain
        if (GetValueFromKey(m_pchAttributes, "Domain", strValue))
		{
			Audio::MixerComponent::Domain eDomain = Audio::MixerComponent::GetDomain(strValue.c_str());
			if (eDomain != Audio::MixerComponent::eUndefinedDomain)
				bfFlags.nDomain = (U32)eDomain;
		}

		//	AlwaysOn
        if (GetValueFromKey(m_pchAttributes, "AlwaysOn", strValue))
		{
			Audio::MixerComponent::AlwaysOn eAlwaysOn = Audio::MixerComponent::GetAlwaysOn(strValue.c_str());
			if (eAlwaysOn != Audio::MixerComponent::eUndefinedAlwaysOn)
				bfFlags.nDomain = (U32)eAlwaysOn;
		}

		//	endian swap flags
		pInitData->bfFlags.uFlags = CommandLineApp::GetApp()->DoEndian(*(U32*)&(bfFlags));

		//  gain
		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		//  level
		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		//  front left azimuth
		GetComponentFloatOutputAttributes("FrontLeftAzimuth", &(pInitData->optFrontLeftAzimuth));
		//  front left elevation
		GetComponentFloatOutputAttributes("FrontLeftElevation", &(pInitData->optFrontLeftElevation));
		//  front left Width
		GetComponentFloatOutputAttributes("FrontLeftWidth", &(pInitData->optFrontLeftWidth));
		//  front left Distance
		GetComponentFloatOutputAttributes("FrontLeftDistance", &(pInitData->optFrontLeftDistance));

		//  front right azimuth
		GetComponentFloatOutputAttributes("FrontRightAzimuth", &(pInitData->optFrontRightAzimuth));
		//  front right elevation
		GetComponentFloatOutputAttributes("FrontRightElevation", &(pInitData->optFrontRightElevation));
		//  front right Width
		GetComponentFloatOutputAttributes("FrontRightWidth", &(pInitData->optFrontRightWidth));
		//  front right Distance
		GetComponentFloatOutputAttributes("FrontRightDistance", &(pInitData->optFrontRightDistance));

		//  rear left azimuth
		GetComponentFloatOutputAttributes("RearLeftAzimuth", &(pInitData->optRearLeftAzimuth));
		//  rear left elevation
		GetComponentFloatOutputAttributes("RearLeftElevation", &(pInitData->optRearLeftElevation));
		//  rear left Width
		GetComponentFloatOutputAttributes("RearLeftWidth", &(pInitData->optRearLeftWidth));
		//  rear left Distance
		GetComponentFloatOutputAttributes("RearLeftDistance", &(pInitData->optRearLeftDistance));

		//  rear right azimuth
		GetComponentFloatOutputAttributes("RearRightAzimuth", &(pInitData->optRearRightAzimuth));
		//  rear right elevation
		GetComponentFloatOutputAttributes("RearRightElevation", &(pInitData->optRearRightElevation));
		//  rear right Width
		GetComponentFloatOutputAttributes("RearRightWidth", &(pInitData->optRearRightWidth));
		//  rear right Distance
		GetComponentFloatOutputAttributes("RearRightDistance", &(pInitData->optRearRightDistance));

		SetComponentData(pInitData, sizeof(Audio::QuadToSurroundMixer::Data), Audio::Component::eQuadToSurroundMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	QuadToSurroundMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "QuadToSurroundMixerInput") == 0)
	{
		Audio::QuadToSurroundMixer::InputGain inputGain;// = new Audio::QuadToSurroundMixer::InputGain;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom)))
			DoError("QuadToSurroundMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("FrontLeftGain", &(inputGain.optFrontLeftGain));
		GetComponentFloatOutputAttributes("FrontLeftLevel", &(inputGain.optFrontLeftLevel));
		GetComponentFloatOutputAttributes("FrontRightGain", &(inputGain.optFrontRightGain));
		GetComponentFloatOutputAttributes("FrontRightLevel", &(inputGain.optFrontRightLevel));
		GetComponentFloatOutputAttributes("RearLeftGain", &(inputGain.optRearLeftGain));
		GetComponentFloatOutputAttributes("RearLeftLevel", &(inputGain.optRearLeftLevel));
		GetComponentFloatOutputAttributes("RearRightGain", &(inputGain.optRearRightGain));
		GetComponentFloatOutputAttributes("RearRightLevel", &(inputGain.optRearRightLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
			DoError("Can't get parent for QuadToSurroundMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());

		switch (pCompData->GetType())
		{
			case Component::eQuadToSurroundMixer:
				{
					Audio::QuadToSurroundMixer::Data* pParentData = (Audio::QuadToSurroundMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("QuadToSurroundMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		// TODO: make enum for 1st param (id), eg. effect, input
		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::QuadToSurroundMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundAudioInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundAudioInput"))
	{
		Audio::SurroundAudioInput::Data* pInitData = new Audio::SurroundAudioInput::Data;
		Audio::SurroundAudioInput::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundAudioInput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::SurroundAudioInput::Data), Audio::Component::eSurroundAudioInput);		
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundAudioOutput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundAudioOutput"))
	{
		Audio::SurroundAudioOutput::Data* pInitData = new Audio::SurroundAudioOutput::Data;
		Audio::SurroundAudioOutput::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundAudioOutput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// input com (req.)
		if (! GetComponentAttribute("InputCom", &(pInitData->sInputCom), 5, Audio::Component::eSurroundAudioInput, Audio::Component::eMonoToSurroundMixer, Audio::Component::eStereoToSurroundMixer, Audio::Component::eQuadToSurroundMixer, Audio::Component::eSurroundToSurroundMixer))
			DoError("SurroundAudioOutput has no inputCom parameter", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::SurroundAudioOutput::Data), Audio::Component::eSurroundAudioOutput);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundLFEAudioInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundLFEAudioInput"))
	{
		Audio::SurroundLFEAudioInput::Data* pInitData = new Audio::SurroundLFEAudioInput::Data;
		Audio::SurroundLFEAudioInput::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundLFEAudioInput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::SurroundLFEAudioInput::Data), Audio::Component::eSurroundLFEAudioInput);		
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundLFEAudioOutput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("SurroundLFEAudioOutput"))
	{
		Audio::SurroundLFEAudioOutput::Data* pInitData = new Audio::SurroundLFEAudioOutput::Data;
		Audio::SurroundLFEAudioOutput::DefaultData(pInitData);

#ifdef AUDIO_INCLUDE_NAMES
        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundLFEAudioOutput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif // AUDIO_INCLUDE_NAMES

		// input com (req.)
		if (! GetComponentAttribute("InputCom", &(pInitData->sInputCom), 4, Audio::Component::eSurroundLFEAudioInput, Audio::Component::eMonoToSurroundLFEMixer, Audio::Component::eSurroundToSurroundLFEMixer, Audio::Component::eSurroundLFEToSurroundLFEMixer))
			DoError("SurroundLFEAudioOutput has no inputCom parameter", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::SurroundLFEAudioOutput::Data), Audio::Component::eSurroundLFEAudioOutput);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundToSurroundMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("SurroundToSurroundMixer"))
    {
		Audio::SurroundToSurroundMixer::Data* pInitData = new Audio::SurroundToSurroundMixer::Data;
		Audio::SurroundToSurroundMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundToSurroundMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		SetComponentData(pInitData, sizeof(Audio::SurroundToSurroundMixer::Data), Audio::Component::eSurroundToSurroundMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundToSurroundMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "SurroundToSurroundMixerInput") == 0)
	{
		Audio::SurroundToSurroundMixer::InputGain inputGain;// = new Audio::SurroundToSurroundMixer::InputGain;
		inputGain.optGain.fVal = 1.f;
		inputGain.optGain.ctrl.sCom = -1;
		inputGain.sInputCom = -1;
		inputGain.optGain.ctrl.uOut = 0;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom)))
			DoError("SurroundToSurroundMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(inputGain.optGain));

		// Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(inputGain.optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for SurroundToSurroundMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eSurroundToSurroundMixer:
				{
					Audio::SurroundToSurroundMixer::Data* pParentData = (Audio::SurroundToSurroundMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("SurroundToSurroundMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::SurroundToSurroundMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundToSurroundLFEMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("SurroundToSurroundLFEMixer"))
    {
		Audio::SurroundToSurroundLFEMixer::Data* pInitData = new Audio::SurroundToSurroundLFEMixer::Data;
		Audio::SurroundToSurroundLFEMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundToSurroundLFEMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		GetComponentFloatOutputAttributes("LFEGain", &(pInitData->optLFEGain));

		GetComponentFloatOutputAttributes("LFELevel", &(pInitData->optLFELevel));

		SetComponentData(pInitData, sizeof(Audio::SurroundToSurroundLFEMixer::Data), Audio::Component::eSurroundToSurroundLFEMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundToSurroundLFEMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "SurroundToSurroundLFEMixerInput") == 0)
	{
		Audio::SurroundToSurroundLFEMixer::InputGain inputGain;// = new Audio::SurroundToSurroundMixer::InputGain;
		inputGain.optGain.fVal = 1.f;
		inputGain.optGain.ctrl.sCom = -1;
		inputGain.sInputCom = -1;
		inputGain.optGain.ctrl.uOut = 0;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom)))
			DoError("SurroundToSurroundLFEMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(inputGain.optGain));

		// Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(inputGain.optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for SurroundToSurroundLFEMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eSurroundToSurroundLFEMixer:
				{
					Audio::SurroundToSurroundLFEMixer::Data* pParentData = (Audio::SurroundToSurroundLFEMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("SurroundToSurroundLFEMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::SurroundToSurroundLFEMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundLFEToSurroundLFEMixer
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("SurroundLFEToSurroundLFEMixer"))
    {
		Audio::SurroundLFEToSurroundLFEMixer::Data* pInitData = new Audio::SurroundLFEToSurroundLFEMixer::Data;
		Audio::SurroundLFEToSurroundLFEMixer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("SurroundLFEToSurroundLFEMixer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("Gain", &(pInitData->optGain));

		GetComponentFloatOutputAttributes("Level", &(pInitData->optLevel));

		SetComponentData(pInitData, sizeof(Audio::SurroundLFEToSurroundLFEMixer::Data), Audio::Component::eSurroundLFEToSurroundLFEMixer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SurroundLFEToSurroundLFEMixerInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "SurroundLFEToSurroundLFEMixerInput") == 0)
	{
		Audio::SurroundLFEToSurroundLFEMixer::InputGain inputGain;// = new Audio::SurroundToSurroundMixer::InputGain;
		inputGain.optGain.fVal = 1.f;
		inputGain.optGain.ctrl.sCom = -1;
		inputGain.sInputCom = -1;
		inputGain.optGain.ctrl.uOut = 0;

		//Input com (req).
		if (! GetComponentAttribute("InputCom", &(inputGain.sInputCom)))
			DoError("SurroundLFEToSurroundLFEMixerInput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Gain (opt.)
		GetComponentFloatOutputAttributes("Gain", &(inputGain.optGain));

		// Level (opt.)
		GetComponentFloatOutputAttributes("Level", &(inputGain.optLevel));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for SurroundLFEToSurroundLFEMixerInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eSurroundLFEToSurroundLFEMixer:
				{
					Audio::SurroundLFEToSurroundLFEMixer::Data* pParentData = (Audio::SurroundLFEToSurroundLFEMixer::Data*)(pCompData->GetData());
					pParentData->uInputs++;
				}
				break;
			default:
				DoError("SurroundLFEToSurroundLFEMixerInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeInput, &inputGain, sizeof(Audio::SurroundLFEToSurroundLFEMixer::InputGain));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Source
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("Source"))
	{
		Audio::Source::Data* pInitData = new Audio::Source::Data;
		Audio::Source::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Source has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("Size", &(pInitData->optSize));

		GetComponentAttribute("SourcePositionVec", &(pInitData->sSourcePositionVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("SourcePositionXVal", &(pInitData->fSourcePositionXVal));
		GetFloatAttribute("SourcePositionYVal", &(pInitData->fSourcePositionYVal));
		GetFloatAttribute("SourcePositionZVal", &(pInitData->fSourcePositionZVal));
		GetComponentAttribute("SourceDirectionVec", &(pInitData->sSourceDirectionVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("SourceDirectionXVal", &(pInitData->fSourceDirectionXVal));
		GetFloatAttribute("SourceDirectionYVal", &(pInitData->fSourceDirectionYVal));
		GetFloatAttribute("SourceDirectionZVal", &(pInitData->fSourceDirectionZVal));
		GetComponentAttribute("SourceVelocityVec", &(pInitData->sSourceVelocityVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("SourceVelocityXVal", &(pInitData->fSourceVelocityXVal));
		GetFloatAttribute("SourceVelocityYVal", &(pInitData->fSourceVelocityYVal));
		GetFloatAttribute("SourceVelocityZVal", &(pInitData->fSourceVelocityZVal));
			//  object
		GetComponentAttribute("ObjectPositionVec", &(pInitData->sObjectPositionVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("ObjectPositionXVal", &(pInitData->fObjectPositionXVal));
		GetFloatAttribute("ObjectPositionYVal", &(pInitData->fObjectPositionYVal));
		GetFloatAttribute("ObjectPositionZVal", &(pInitData->fObjectPositionZVal));
		GetComponentAttribute("ObjectDirectionVec", &(pInitData->sObjectDirectionVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("ObjectDirectionXVal", &(pInitData->fObjectDirectionXVal));
		GetFloatAttribute("ObjectDirectionYVal", &(pInitData->fObjectDirectionYVal));
		GetFloatAttribute("ObjectDirectionZVal", &(pInitData->fObjectDirectionZVal));
		GetComponentAttribute("ObjectUpVec", &(pInitData->sObjectUpVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("ObjectUpXVal", &(pInitData->fObjectUpXVal));
		GetFloatAttribute("ObjectUpYVal", &(pInitData->fObjectUpYVal));
		GetFloatAttribute("ObjectUpXZVal", &(pInitData->fObjectUpZVal));
		GetComponentAttribute("ObjectVelocityVec", &(pInitData->sObjectVelocityVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("ObjectVelocityXVal", &(pInitData->fObjectVelocityXVal));
		GetFloatAttribute("ObjectVelocityYVal", &(pInitData->fObjectVelocityYVal));
		GetFloatAttribute("ObjectVelocityZVal", &(pInitData->fObjectVelocityZVal));
			//  listener
		GetComponentAttribute("ListenerPositionVec", &(pInitData->sListenerPositionVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("ListenerPositionXVal", &(pInitData->fListenerPositionXVal));
		GetFloatAttribute("ListenerPositionYVal", &(pInitData->fListenerPositionYVal));
		GetFloatAttribute("ListenerPositionZVal", &(pInitData->fListenerPositionZVal));
		GetComponentAttribute("ListenerDirectionVec", &(pInitData->sListenerDirectionVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("ListenerDirectionXVal", &(pInitData->fListenerDirectionXVal));
		GetFloatAttribute("ListenerDirectionYVal", &(pInitData->fListenerDirectionYVal));
		GetFloatAttribute("ListenerDirectionZVal", &(pInitData->fListenerDirectionZVal));
		GetComponentAttribute("ListenerUpVec", &(pInitData->sListenerUpVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("ListenerUpXVal", &(pInitData->fListenerUpXVal));
		GetFloatAttribute("ListenerUpYVal", &(pInitData->fListenerUpYVal));
		GetFloatAttribute("ListenerUpZVal", &(pInitData->fListenerUpZVal));
		GetComponentAttribute("ListenerVelocityVec", &(pInitData->sListenerVelocityVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);
		GetFloatAttribute("ListenerVelocityXVal", &(pInitData->fListenerVelocityXVal));
		GetFloatAttribute("ListenerVelocityYVal", &(pInitData->fListenerVelocityYVal));
		GetFloatAttribute("ListenerVelocityZVal", &(pInitData->fListenerVelocityZVal));

		SetComponentData(pInitData, sizeof(Audio::Source::Data), Audio::Component::eSource);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Rolloff
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("Rolloff"))
	{
		Audio::Rolloff::Data* pInitData = new Audio::Rolloff::Data;
		Audio::Rolloff::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Rolloff has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	distance com (req.)
// TODO: needs new method of specifying outputs implementing
		if (! GetComponentAndOutputAttributes("Distance", &(pInitData->ctrlDistance)))
			DoError("Rolloff has no DistanceCom", pParentNode->GetLine(), pParentNode->GetFilename());

//  rolloff factor
		GetComponentFloatOutputAttributes("RolloffFactor", &(pInitData->optRolloffFactor));
//  max level
		GetComponentFloatOutputAttributes("MaxLevel", &(pInitData->optMaxLevel));
//  ref level
		GetComponentFloatOutputAttributes("RefLevel", &(pInitData->optRefLevel));
//  ref distance
		GetComponentFloatOutputAttributes("RefDistance", &(pInitData->optRefDistance));
//  max distance
		GetComponentFloatOutputAttributes("MaxDistance", &(pInitData->optMaxDistance));

		SetComponentData(pInitData, sizeof(Audio::Rolloff::Data), Audio::Component::eRolloff);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	DopplerShift
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("DopplerShift"))
	{
		Audio::DopplerShift::Data* pInitData = new Audio::DopplerShift::Data;
		Audio::DopplerShift::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("DopplerShift has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

	//////////	RelativeVelocity (opt.)
	//////	GetComponentOrFloatAttributes("RelativeVelocity", &(pInitData->optRelativeVelocity.ctrl.sCom), &(pInitData->optRelativeVelocity.fVal));

	////////	//	RelativeVelocity out (opt.)
	//////	GetUnsignedIntAttribute("RelativeVelocityOut", &(pInitData->optRelativeVelocity.ctrl.uOut));

	//////	
				// Input A Vec (req.)
		if (! GetComponentAttribute("SourceVelocity", &(pInitData->sInputSourceVelocityVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eDopplerShift, Audio::Component::eVector))
			DoError("DopplerShift has no SourceVelocity attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Input B Vec (req.)
		if (! GetComponentAttribute("ListenerVelocity", &(pInitData->sInputListenerVelocityVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eDopplerShift, Audio::Component::eVector))
			DoError("DopplerShift has no ListenerVelocity attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Input Align Vec (req.)
		if (! GetComponentAttribute("SourcePosition", &(pInitData->sInputSourcePositionVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eDopplerShift, Audio::Component::eVector))
			DoError("DopplerShift has no SourcePosition attribute", pParentNode->GetLine(), pParentNode->GetFilename());

			// Input Align Vec (req.)
		if (! GetComponentAttribute("ListenerPosition", &(pInitData->sInputListenerPositionVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eDopplerShift, Audio::Component::eVector))
			DoError("DopplerShift has no ListenerPosition attribute", pParentNode->GetLine(), pParentNode->GetFilename());
		
		
		
		//	reset (opt.)
		GetComponentFloatOutputAttributes("SpeedOfSound", &(pInitData->optSpeedOfSound));

		SetComponentData(pInitData, sizeof(Audio::DopplerShift::Data), Audio::Component::eDopplerShift);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Cone
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("Cone"))
	{
		Audio::Cone::Data* pInitData = new Audio::Cone::Data;
		Audio::Cone::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Cone has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// incident angle com (req.)
		if (! GetComponentAndOutputAttributes("IncidentAngle", &(pInitData->ctrlIncidentAngle)))
			DoError("Cone has no IncidentAngleCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("InnerAngle", &(pInitData->optInnerAngle));

		GetComponentFloatOutputAttributes("InnerLevel", &(pInitData->optInnerLevel));

		GetComponentFloatOutputAttributes("OuterAngle", &(pInitData->optOuterAngle));

		GetComponentFloatOutputAttributes("OuterLevel", &(pInitData->optOuterLevel));

		GetComponentFloatOutputAttributes("Curve", &(pInitData->optCurve));

		SetComponentData(pInitData, sizeof(Audio::Cone::Data), Audio::Component::eCone);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	VectorIn
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("VectorIn"))
	{
		Audio::VectorIn::Data* pInitData = new Audio::VectorIn::Data;
		Audio::VectorIn::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Vector has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetFloatAttribute("XVal", &(pInitData->fX));
		GetFloatAttribute("YVal", &(pInitData->fY));
		GetFloatAttribute("ZVal", &(pInitData->fZ));

		SetComponentData(pInitData, sizeof(Audio::VectorIn::Data), Audio::Component::eVectorIn);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
//
//	VectorOut
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("VectorOut"))
	{
		Audio::VectorOut::Data* pInitData = new Audio::VectorOut::Data;
		Audio::VectorOut::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("VectorOut has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetFloatAttribute("XVal", &(pInitData->fInputXVal));
		GetFloatAttribute("YVal", &(pInitData->fInputYVal));
		GetFloatAttribute("ZVal", &(pInitData->fInputZVal));

		SetComponentData(pInitData, sizeof(Audio::VectorOut::Data), Audio::Component::eVectorOut);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Vector
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("Vector"))
	{
		Audio::Vector::Data* pInitData = new Audio::Vector::Data;
		Audio::Vector::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Vector has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// X (opt.)
		bool bXInput = GetComponentFloatOutputAttributes("X", &(pInitData->optX));

		// Y (opt.)
		bool bYInput = GetComponentFloatOutputAttributes("Y", &(pInitData->optY));

		// Z (opt.)
		bool bZInput = GetComponentFloatOutputAttributes("Z", &(pInitData->optZ));

		// Input Vec (opt.)
		bool bVecInput = GetComponentAttribute("InputVec", &(pInitData->sInputVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector);

		// we shouldn't have a vector input if x, y, or z have also been specified
		if (bVecInput && (bXInput || bYInput || bZInput))
			DoError("Vector has both vector input and X, Y or Z element input", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::Vector::Data), Audio::Component::eVector);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	VectorNearestPointOnLine
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("VectorNearestPointOnLine"))
	{
		Audio::VectorNearestPointOnLine::Data* pInitData = new Audio::VectorNearestPointOnLine::Data;
		Audio::VectorNearestPointOnLine::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("VectorNearestPointOnLine has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Input A Vec (req.)
		if (! GetComponentAttribute("InputAVec", &(pInitData->sInputAVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector))
			DoError("VectorNearestPointOnLine has no InputA attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Input B Vec (req.)
		if (! GetComponentAttribute("InputBVec", &(pInitData->sInputBVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector))
			DoError("VectorNearestPointOnLine has no InputB attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Input Align Vec (req.)
		if (! GetComponentAttribute("InputAlignVec", &(pInitData->sInputAlignVec), 4, Audio::Component::eVectorIn, Audio::Component::eCoordinateInput, Audio::Component::eVectorNearestPointOnLine, Audio::Component::eVector))
			DoError("VectorNearestPointOnLine has no InputAlign attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::VectorNearestPointOnLine::Data), Audio::Component::eVectorNearestPointOnLine);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	PiecewiseLinearFunction
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("PiecewiseLinearFunction"))
	{
		Audio::PiecewiseLinearFunction::Data* pInitData = new Audio::PiecewiseLinearFunction::Data;
		Audio::PiecewiseLinearFunction::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("PiecewiseLinearFunction has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
			DoError("PiecewiseLinearFunction has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentFloatOutputAttributes("Curve", &(pInitData->optCurve));

		SetComponentData(pInitData, sizeof(Audio::PiecewiseLinearFunction::Data), Audio::Component::ePiecewiseLinearFunction);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	DataPoint
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "DataPoint") == 0)
	{
		Audio::PiecewiseLinearFunction::Point point;// = new Audio::PiecewiseLinearFunction::Point;

		point.fXVal = 0.f;
		point.optY.fVal = 0.f;
		point.optY.ctrl.sCom = -1;
		point.optY.ctrl.uOut = 0;

		//	X Val (req).
		if (! GetFloatAttribute("XVal", &(point.fXVal)))
			DoError("PLF DataPoint has no X value", pParentNode->GetLine(), pParentNode->GetFilename());

		//	Y (opt).
		GetComponentFloatOutputAttributes("Y", &(point.optY));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for PLF DataPoint", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::ePiecewiseLinearFunction:
				{
					Audio::PiecewiseLinearFunction::Data* pParentData = (Audio::PiecewiseLinearFunction::Data*)(pCompData->GetData());
					pParentData->uNumPoints++;
				}
				break;
			default: 
				DoError("PLF DataPoint has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeDefault, &point, sizeof(Audio::PiecewiseLinearFunction::Point));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StateMachine
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("StateMachine"))
	{
		Audio::StateMachine::Data* pInitData = new Audio::StateMachine::Data;
		Audio::StateMachine::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("StateMachine has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// input com (req.)
		if (! GetIntAttribute("InitialState", &(pInitData->sInitialState)))
			DoError("StateMachine has no InitialState attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetComponentAndOutputAttributes("NextState", &(pInitData->ctrlNextState));

		SetComponentData(pInitData, sizeof(Audio::StateMachine::Data), Audio::Component::eStateMachine);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StateItem
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "State") == 0)
	{
		Audio::StateMachine::StateItem state;// = new Audio::PiecewiseLinearFunction::Point;

		state.evEnter.sCom = -1;
		state.evEnter.uMsg = 0;
		state.evExit.sCom = -1;
		state.evExit.uMsg = 0;

		LookupEvent("Enter", state.evEnter);
		LookupEvent("Exit", state.evExit);

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for StateMachine StateItem", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eStateMachine:
				{
					Audio::StateMachine::Data* pParentData = (Audio::StateMachine::Data*)(pCompData->GetData());
					pParentData->uNumStates++;
				}
				break;
			default:
				DoError("StateMachine DataPoint has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeDefault, &state, sizeof(Audio::StateMachine::StateItem));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StateMachine
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("Sequence"))
	{
		Audio::Sequence::Data* pInitData = new Audio::Sequence::Data;
		Audio::Sequence::DefaultData(pInitData);

		//	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Sequence has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
            DoError("Sequence has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::Sequence::Data), Audio::Component::eSequence);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	StateItem
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (strcmp(pchType, "SequenceElement") == 0)
	{
		Audio::Sequence::Element state;

		if (! GetFloatAttribute("Time", &(state.fTime)))
			DoError("Sequence Element has no Time value", pParentNode->GetLine(), pParentNode->GetFilename());

		LookupEvent("Event", state.evEvent);

		GetComponentFloatOutputAttributes("Value", &(state.optValue));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for Sequence Element", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eSequence:
				{
					Audio::Sequence::Data* pParentData = (Audio::Sequence::Data*)(pCompData->GetData());
					pParentData->uNumElements++;
				}
				break;
			default:
				DoError("Sequence Element has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeDefault, &state, sizeof(Audio::Sequence::Element));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	GateSwitch
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("GateSwitch"))
    {
		Audio::GateSwitch::Data* pInitData = new Audio::GateSwitch::Data;
		Audio::GateSwitch::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("GateSwitch has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
			DoError("GateSwitch has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());
//  threshold
		GetComponentFloatOutputAttributes("Threshold", &(pInitData->optThreshold));
//  hystresis
		GetComponentFloatOutputAttributes("Hystresis", &(pInitData->optHystresis));
//  low level
		GetComponentFloatOutputAttributes("LowLevel", &(pInitData->optLowLevel));
//  high level
		GetComponentFloatOutputAttributes("HighLevel", &(pInitData->optHighLevel));
//  rising script
#ifndef NEW_EVENT_CODE
		GetComponentAttribute("RisingScr", &(pInitData->sRisingScr));
#else
		LookupEvent("Rising", pInitData->evRising);
#endif
//  falling script
#ifndef NEW_EVENT_CODE
		GetComponentAttribute("FallingScr", &(pInitData->sFallingScr));
#else
		LookupEvent("Falling", pInitData->evFalling);
#endif

		SetComponentData(pInitData, sizeof(Audio::GateSwitch::Data), Audio::Component::eGateSwitch);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	LowPass
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("LowPass"))
    {
		Audio::LowPass::Data* pInitData = new Audio::LowPass::Data;
		Audio::LowPass::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("LowPass has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        //	input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
            DoError("LowPass has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	cutoff (opt.)
		GetComponentFloatOutputAttributes("Cutoff", &(pInitData->optCutoff));

		SetComponentData(pInitData, sizeof(Audio::LowPass::Data), Audio::Component::eLowPass);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Gradient
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Gradient"))
    {
		Audio::Gradient::Data* pInitData = new Audio::Gradient::Data;
		Audio::Gradient::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Gradient has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        //	input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
            DoError("Gradient has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::Gradient::Data), Audio::Component::eGradient);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	ParameterIn
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("ParameterIn"))
    {
		Audio::ParameterIn::Data* pInitData = new Audio::ParameterIn::Data;
		Audio::ParameterIn::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("ParameterIn has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	default (opt.)
		GetFloatAttribute("DefaultVal", &(pInitData->fDefault));

		SetComponentData(pInitData, sizeof(Audio::ParameterIn::Data), Audio::Component::eParameterIn);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	ParameterOut
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("ParameterOut"))
    {
		Audio::ParameterOut::Data* pInitData = new Audio::ParameterOut::Data;
		Audio::ParameterOut::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("ParameterOut has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	input (req.)
		if (! GetComponentFloatOutputAttributes("Input", &(pInitData->optInput)))
			DoError("ParameterOut has no input attributes", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::ParameterOut::Data), Audio::Component::eParameterOut);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Variable
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Variable"))
    {
		Audio::Variable::Data* pInitData = new Audio::Variable::Data;
		Audio::Variable::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Variable has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	default (opt.)
		GetFloatAttribute("DefaultVal", &(pInitData->fDefault));

		SetComponentData(pInitData, sizeof(Audio::Variable::Data), Audio::Component::eVariable);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	ControlInput
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("ControlInput"))
    {
		Audio::ControlInput::Data* pInitData = new Audio::ControlInput::Data;
		Audio::ControlInput::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("ControlInput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::ControlInput::Data), Audio::Component::eControlInput);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	ControlOutput
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("ControlOutput"))
    {
		Audio::ControlOutput::Data* pInitData = new Audio::ControlOutput::Data;
		Audio::ControlOutput::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("ControlOutput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// input com (req.)
		//if (! GetComponentAttribute("InputCom", &(pInitData->sInputCom)))
		//	DoError("ControlOutput has no inputCom parameter", pParentNode->GetLine(), pParentNode->GetFilename());
		//	input (opt.)
		if (! GetComponentFloatOutputAttributes("Input", &(pInitData->optInput)))
			DoError("ControlOutput has no input attributes", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::ControlOutput::Data), Audio::Component::eControlOutput);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Product
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Product"))
    {
		Audio::Product::Data* pInitData = new Audio::Product::Data;
		Audio::Product::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Product has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::Product::Data), Audio::Component::eProduct);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProductInput
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (strcmp(pchType, "ProductInput") == 0)
    {
		Audio::Product::Input input;// = new Audio::Product::Input;

		input.optInput.ctrl.sCom = -1;
		input.optInput.ctrl.uOut = 0;
		input.optInput.fVal = 0.f;

		// input (req.)
		if (! GetComponentFloatOutputAttributes("Input", &(input.optInput)))
			DoError("ProductInput has no Input attributes", pParentNode->GetLine(), pParentNode->GetFilename());

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for Product ProductInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eProduct:
				{
					Audio::Product::Data* pParentData = (Audio::Product::Data*)(pCompData->GetData());
					pParentData->uNumInputs++;
				}
				break;
			default:
				DoError("Product ProductInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeDefault, &input, sizeof(Audio::Product::Input));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Sum
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Sum"))
    {
		Audio::Sum::Data* pInitData = new Audio::Sum::Data;
		Audio::Sum::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Sum has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::Sum::Data), Audio::Component::eSum);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SumInput
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (strcmp(pchType, "SumInput") == 0)
    {
		Audio::Sum::Input input;// = new Audio::Sum::Input;

		input.optInput.ctrl.sCom = -1;
		input.optInput.ctrl.uOut = 0;
		input.optInput.fVal = 0.f;

		// input com (opt.)
		GetComponentFloatOutputAttributes("Input", &(input.optInput));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for Sum SumInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eSum:
				{
					Audio::Sum::Data* pParentData = (Audio::Sum::Data*)(pCompData->GetData());
					pParentData->uNumInputs++;
				}
				break;
			default:
				DoError("Sum SumInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeDefault, &input, sizeof(Audio::Sum::Input));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Exponent
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Exponent"))
    {
		Audio::Exponent::Data* pInitData = new Audio::Exponent::Data;
		Audio::Exponent::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Exponent has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
			DoError("Exponent has no Input attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	power (opt.)
		GetComponentFloatOutputAttributes("Power", &(pInitData->optPower));

		SetComponentData(pInitData, sizeof(Audio::Exponent::Data), Audio::Component::eExponent);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Fraction
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Fraction"))
    {
		Audio::Fraction::Data* pInitData = new Audio::Fraction::Data;
		Audio::Fraction::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
			DoError("Fraction has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
			DoError("Fraction has no Input Com attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::Fraction::Data), Audio::Component::eFraction);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Random
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Random"))
    {
		Audio::Random::Data* pInitData = new Audio::Random::Data;
		Audio::Random::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Random has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	min
		GetComponentFloatOutputAttributes("Min", &(pInitData->optMin));
		//	max
		GetComponentFloatOutputAttributes("Max", &(pInitData->optMax));

		SetComponentData(pInitData, sizeof(Audio::Random::Data), Audio::Component::eRandom);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Max
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Max"))
    {
		Audio::Max::Data* pInitData = new Audio::Max::Data;
		Audio::Max::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Max has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::Max::Data), Audio::Component::eMax);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MaxInput
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (strcmp(pchType, "MaxInput") == 0)
    {
		Audio::Max::Input input;

		input.optInput.ctrl.sCom = -1;
		input.optInput.ctrl.uOut = 0;
		input.optInput.fVal = 0.f;

		// input (req.)
		if (! GetComponentFloatOutputAttributes("Input", &(input.optInput)))
			DoError("MaxInput has no Input attributes", pParentNode->GetLine(), pParentNode->GetFilename());

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for Max MaxInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eMax:
				{
					Audio::Max::Data* pParentData = (Audio::Max::Data*)(pCompData->GetData());
					pParentData->uNumInputs++;
				}
				break;
			default:
				DoError("Max MaxInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeDefault, &input, sizeof(Audio::Max::Input));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Min
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Min"))
    {
		Audio::Min::Data* pInitData = new Audio::Min::Data;
		Audio::Min::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Min has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::Min::Data), Audio::Component::eMin);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MinInput
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (strcmp(pchType, "MinInput") == 0)
    {
		Audio::Min::Input input;

		input.optInput.ctrl.sCom = -1;
		input.optInput.ctrl.uOut = 0;
		input.optInput.fVal = 0.f;

		// input (req.)
		if (! GetComponentFloatOutputAttributes("Input", &(input.optInput)))
			DoError("MinInput has no Input attributes", pParentNode->GetLine(), pParentNode->GetFilename());

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for Min MinInput", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eMin:
				{
					Audio::Min::Data* pParentData = (Audio::Min::Data*)(pCompData->GetData());
					pParentData->uNumInputs++;
				}
				break;
			default:
				DoError("Min MinInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeDefault, &input, sizeof(Audio::Min::Input));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Envelope
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Envelope"))
    {
		Audio::Envelope::Data* pInitData = new Audio::Envelope::Data;
		Audio::Envelope::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Envelope has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
			DoError("Envelope has no Input attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// attack (opt.)
		GetComponentFloatOutputAttributes("Attack", &(pInitData->optAttack));

		// hold (opt.)
		GetComponentFloatOutputAttributes("Hold", &(pInitData->optHold));

		// release (opt.)
		GetComponentFloatOutputAttributes("Release", &(pInitData->optRelease));

		// delay (opt.)
		GetComponentFloatOutputAttributes("Delay", &(pInitData->optDelay));

		LookupEvent("AttackEnd", pInitData->evAttackEnd);
		LookupEvent("HoldEnd", pInitData->evHoldEnd);
		LookupEvent("ReleaseEnd", pInitData->evReleaseEnd);

		SetComponentData(pInitData, sizeof(Audio::Envelope::Data), Audio::Component::eEnvelope);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Spring
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Spring"))
    {
		Audio::Spring::Data* pInitData = new Audio::Spring::Data;
		Audio::Spring::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Spring has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
			DoError("Spring has no Input attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Frequency (opt.)
		GetComponentFloatOutputAttributes("Frequency", &(pInitData->optFrequency));

		// Damping (opt.)
		GetComponentFloatOutputAttributes("Damping", &(pInitData->optDamping));

		SetComponentData(pInitData, sizeof(Audio::Spring::Data), Audio::Component::eSpring);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Selector
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Selector"))
    {
		Audio::Selector::Data* pInitData = new Audio::Selector::Data;
		Audio::Selector::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Selector has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//	reset (opt.)
		GetComponentFloatOutputAttributes("Reset", &(pInitData->optReset));

		SetComponentData(pInitData, sizeof(Audio::Selector::Data), Audio::Component::eSelector);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SelectorElement
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (strcmp(pchType, "SelectorElement") == 0)
    {
		Audio::Selector::Element element;//

		Audio::Selector::DefaultDataElement(&element);

		// element (opt.)
		GetComponentFloatOutputAttributes("Element", &(element.optElement));

		// scale (opt.)
		GetComponentFloatOutputAttributes("Scale", &(element.optScale));

		// weight (opt.)
		GetComponentFloatOutputAttributes("Weight", &(element.optWeight));

		// Offset (opt.)
		GetComponentFloatOutputAttributes("Offset", &(element.optOffset));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for Selector SelectorElement", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eSelector:
				{
					Audio::Selector::Data* pParentData = (Audio::Selector::Data*)(pCompData->GetData());
					pParentData->uNumElements++;
				}
				break;
			default:
				DoError("Selector SelectorElement has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeDefault, &element, sizeof(Audio::Selector::Element));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Quantize
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Quantize"))
    {
		Audio::Quantize::Data* pInitData = new Audio::Quantize::Data;
		Audio::Quantize::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
			DoError("Quantize has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
			DoError("Quantize has no Input Com attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        GetComponentFloatOutputAttributes("Step", &(pInitData->optStep));

        GetComponentFloatOutputAttributes("Offset", &(pInitData->optOffset));

		SetComponentData(pInitData, sizeof(Audio::Quantize::Data), Audio::Component::eQuantize);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Sine
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("Sine"))
    {
		Audio::Sine::Data* pInitData = new Audio::Sine::Data;
		Audio::Sine::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
			DoError("Sine has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
			DoError("Sine has no Input Com attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::Sine::Data), Audio::Component::eSine);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	SampleAndHold
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (GetComponent("SampleAndHold"))
    {
		Audio::SampleAndHold::Data* pInitData = new Audio::SampleAndHold::Data;
		Audio::SampleAndHold::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
			DoError("SampleAndHold has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// control com (opt.)
		GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput));

		SetComponentData(pInitData, sizeof(Audio::SampleAndHold::Data), Audio::Component::eSampleAndHold);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Array
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("Array"))
	{
		Audio::Array::Data* pInitData = new Audio::Array::Data;
		Audio::Array::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Array has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        //  input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
            DoError("Array has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        GetComponentFloatOutputAttributes("Offset", &(pInitData->optOffset));

		SetComponentData(pInitData, sizeof(Audio::Array::Data), Audio::Component::eArray);		
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	ArrayElement
//
////////////////////////////////////////////////////////////////////////////////////////////
    else if (strcmp(pchType, "ArrayElement") == 0)
    {
		Audio::Array::Element element;

		element.optElement.fVal = 0.f;
		element.optElement.ctrl.sCom = -1;
		element.optElement.ctrl.uOut = 0;

		GetComponentFloatOutputAttributes("Element", &(element.optElement));

		ComponentData* pCompData = GetParentComponentData(pParentNode);

		if (! pCompData)
		{
			DoError("Can't get parent for ArrayElement", pParentNode->GetLine(), pParentNode->GetFilename());
			return;
		}

		switch (pCompData->GetType())
		{
			case Component::eArray:
				{
					Audio::Array::Data* pParentData = (Audio::Array::Data*)(pCompData->GetData());
					pParentData->uNumElements++;
				}
				break;
			default:
				DoError("ArrayElement has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
		}

		AssociateDataWithComponent(pCompData, eDataTypeDefault, &element, sizeof(Audio::Array::Element));
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MonoAudioInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MonoAudioInput"))
	{
		Audio::MonoAudioInput::Data* pInitData = new Audio::MonoAudioInput::Data;
		Audio::MonoAudioInput::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MonoAudioInput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		//[ciano] - ready to be added when needed
		//ComponentData* pCompData = GetParentComponentData(pParentNode);

		//if (! pCompData)
		//	DoError("Can't get parent for MonoAudioInput", pParentNode->GetLine(), pParentNode->GetFilename());

		//if (! CheckMonoAndIncEffects(pCompData))
		//	DoError("MonoAudioInput has invalid parent", pParentNode->GetLine(), pParentNode->GetFilename());
	
		//		// TODO: make enum for 1st param (id), eg. effect, input
		//AssociateDataWithComponent(pCompData, eDataTypeEffect, &m_u32CurrComponentIndex, sizeof(U32));

		//if (! CheckMonoAndIncEffects(pInitData))
		//	DoError("MonoAudioInput has invalid effects");
		//U32 u32OrigCompIndex = m_u32CurrComponentIndex;
	//	pInitData->uEffects++;

		//IncrementDataSize(sizeof(U32));
		//IncrementDataSize(pInitData->uEffects* sizeof(U32), u32OrigCompIndex);
		


		SetComponentData(pInitData, sizeof(Audio::MonoAudioInput::Data), Audio::Component::eMonoAudioInput);		
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Script
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("Script"))
    {
		Audio::Script::Data* pInitData = new Audio::Script::Data;
		Audio::Script::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Script has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		U32 u32OrigCompIndex = m_u32CurrComponentIndex;
		SetComponentData(pInitData, sizeof(Audio::Script::Data), Audio::Component::eScript);

        //  get script code
		for (Audio::Iterator<xml::Node*> i = pParentNode->GetChildren().first(); ! i.isPastEnd(); i.advance())
		{
		    xml::TextNode* pText = dynamic_cast<xml::TextNode*>(i.retrieve());
			if (pText)
			{
				Audio::ScriptBuilder scriptBuilder(m_apComponentData, m_aConstants);
				scriptBuilder.SetErrorCallback(ReceiveScriptError);

				if (scriptBuilder.Make(m_strWorkString.data(), pText->GetText(), m_apComponentData[u32OrigCompIndex]))
				{
					pInitData->uLength = scriptBuilder.GetCodeLength();
					m_apComponentData[u32OrigCompIndex]->AddChildData(eDataTypeScript, scriptBuilder.GetCode(), pInitData->uLength * sizeof(U32));

					if (! scriptBuilder.ContainsAnExit())
						DoWarning("Script does not contain an EXIT", pParentNode->GetLine(), pParentNode->GetFilename());

					IncrementDataSize(pInitData->uLength * sizeof(U32), u32OrigCompIndex);
				}

				break;
			}
		}
    }
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Timer
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("Timer"))
	{
		Audio::Timer::Data* pInitData = new Audio::Timer::Data;
		Audio::Timer::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Timer has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// start at com (opt.)
		GetComponentFloatOutputAttributes("Startat", &(pInitData->optStartAt));

		GetComponentFloatOutputAttributes("Increment", &(pInitData->optIncrement));

		GetComponentFloatOutputAttributes("Alarm", &(pInitData->optAlarm));

#ifndef NEW_EVENT_CODE
		GetComponentAttribute("AlarmScr", &(pInitData->sAlarmScr));
#else
		LookupEvent("Alarm", pInitData->evAlarm);
#endif

		SetComponentData(pInitData, sizeof(Audio::Timer::Data), Audio::Component::eTimer);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Timer
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("MessageDelay"))
	{
		Audio::MessageDelay::Data* pInitData = new Audio::MessageDelay::Data;
		Audio::MessageDelay::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("MessageDelay has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// start at com (opt.)
		GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));

		LookupEvent("Delayed", pInitData->evDelayed);

		SetComponentData(pInitData, sizeof(Audio::MessageDelay::Data), Audio::Component::eMessageDelay);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Timer
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("ImpactFilter"))
	{
		Audio::ImpactFilter::Data* pInitData = new Audio::ImpactFilter::Data;
		Audio::ImpactFilter::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("ImpactFilter has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        //  input com (req.)
		if (! GetComponentAndOutputAttributes("Force", &(pInitData->ctrlForce)))
            DoError("ImpactFilter has no ForceCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Delay com (opt.)
		GetComponentFloatOutputAttributes("Delay", &(pInitData->optDelay));

		// HalfLife com (opt.)
		GetComponentFloatOutputAttributes("HalfLife", &(pInitData->optHalfLife));

		LookupEvent("Impact", pInitData->evImpact);

		SetComponentData(pInitData, sizeof(Audio::ImpactFilter::Data), Audio::Component::eImpactFilter);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	Metronome
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("Metronome"))
	{
		Audio::Metronome::Data* pInitData = new Audio::Metronome::Data;
		Audio::Metronome::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("Metronome has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// Tempo (opt.)
		GetComponentFloatOutputAttributes("Tempo", &(pInitData->optTempo));

		// BeatsPerBar (opt.)
		GetComponentFloatOutputAttributes("BeatsPerBar", &(pInitData->optBeatsPerBar));

		// Length (opt.)
		GetComponentFloatOutputAttributes("Length", &(pInitData->optLength));

#ifndef NEW_EVENT_CODE
		GetComponentAttribute("BeatScr", &(pInitData->sBeatScr));
#else
		LookupEvent("Beat", pInitData->evBeat);
#endif

#ifndef NEW_EVENT_CODE
		GetComponentAttribute("BarScr", &(pInitData->sBarScr));
#else
		LookupEvent("Bar", pInitData->evBar);
#endif

#ifndef NEW_EVENT_CODE
		GetComponentAttribute("FinishedScr", &(pInitData->sFinishedScr));
#else
		LookupEvent("Finished", pInitData->evFinished);
#endif

		SetComponentData(pInitData, sizeof(Audio::Metronome::Data), Audio::Component::eMetronome);
	}
////////////////////////////////////////////////////////////////////////////////////////////
//
//	CoordinateInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("CoordinateInput"))
    {
		Audio::CoordinateInput::Data* pInitData = new Audio::CoordinateInput::Data;
		Audio::CoordinateInput::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("CoordinateInput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::CoordinateInput::Data), Audio::Component::eCoordinateInput);
    }
////////////////////////////////////////////////////////////////////////////////////////////
//
//	CoordinateOutput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("CoordinateOutput"))
    {
		Audio::CoordinateOutput::Data* pInitData = new Audio::CoordinateOutput::Data;
		Audio::CoordinateOutput::DefaultData(pInitData);

        //	name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("CoordinateOutput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

        //	input com (req.)
		if (! GetComponentAttribute("InputCom", &(pInitData->sInputCom)))
            DoError("CoordinateOutput has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::CoordinateOutput::Data), Audio::Component::eCoordinateOutput);
    }
////////////////////////////////////////////////////////////////////////////////////////////
//
//	ActionIn
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("ActionIn"))
    {
		Audio::ActionIn::Data* pInitData = new Audio::ActionIn::Data;
		Audio::ActionIn::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("ActionIn has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// script (req.)
#ifndef NEW_EVENT_CODE
		if (! GetComponentAttribute("ActionScr", &(pInitData->sEventScr), 2, Audio::Component::eScript, Audio::Component::eActionOut))
			DoError("ActionIn has no ActionScr attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#else
		if (! LookupEvent("Action", pInitData->evTrigger))
			DoError("ActionIn has no ActionScr attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif

		SetComponentData(pInitData, sizeof(Audio::ActionIn::Data), Audio::Component::eActionIn);
    }
////////////////////////////////////////////////////////////////////////////////////////////
//
//	ActionOut
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("ActionOut"))
    {
		Audio::ActionOut::Data* pInitData = new Audio::ActionOut::Data;
		Audio::ActionOut::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("ActionOut has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::ActionOut::Data), Audio::Component::eActionOut);
    }
////////////////////////////////////////////////////////////////////////////////////////////
//
//	EventInput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("EventInput"))
    {
		Audio::EventInput::Data* pInitData = new Audio::EventInput::Data;
		Audio::EventInput::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("EventInput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// script (req.)
#ifndef NEW_EVENT_CODE
		if (! GetComponentAttribute("EventScr", &(pInitData->sOutputScr), 1, Audio::Component::eScript))
			DoError("EventInput has no EventScr attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#else
		if (! LookupEvent("Event", pInitData->evOutput))
			DoError("EventInput has no EventScr attribute", pParentNode->GetLine(), pParentNode->GetFilename());
#endif

		SetComponentData(pInitData, sizeof(Audio::EventInput::Data), Audio::Component::eEventInput);
    }
////////////////////////////////////////////////////////////////////////////////////////////
//
//	EventOutput
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("EventOutput"))
    {
		Audio::EventOutput::Data* pInitData = new Audio::EventOutput::Data;
		Audio::EventOutput::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("EventOutput has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		SetComponentData(pInitData, sizeof(Audio::EventOutput::Data), Audio::Component::eEventOutput);
    }
////////////////////////////////////////////////////////////////////////////////////////////
//
//	DebugPrint
//
////////////////////////////////////////////////////////////////////////////////////////////
	else if (GetComponent("DebugPrint"))
    {
		Audio::DebugPrint::Data* pInitData = new Audio::DebugPrint::Data;
		Audio::DebugPrint::DefaultData(pInitData);

        //  name (req.)
		if (! GetStringOffsetAttribute("Name", &(pInitData->strName)))
            DoError("DebugPrint has no Name attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		// input com (req.)
		if (! GetComponentAndOutputAttributes("Input", &(pInitData->ctrlInput)))
			DoError("DebugPrint has no InputCom attribute", pParentNode->GetLine(), pParentNode->GetFilename());

		GetNonComponentStringOffsetAttribute("Text", &(pInitData->strText));

		SetComponentData(pInitData, sizeof(Audio::DebugPrint::Data), Audio::Component::eDebugPrint);
    }
// default..
	else if (strcmp(pchType, "Constant") != 0)
	{
		DoError("Found unknown component", pParentNode->GetLine(), pParentNode->GetFilename(), true);
		// add dummy component
//		SetComponentData(0, 0, -1);
	}
}


//------------------------------------------------------------------------------
//  GetStringOffset
//------------------------------------------------------------------------------
U32 AudioDataFile::GetStringOffset(U32 u32StringIndex)
{
	return m_apComponentData[u32StringIndex]->GetOffset() + strlen(m_strObjectName) + 1;
}


//------------------------------------------------------------------------------
//  GetStringIndex
//------------------------------------------------------------------------------
U32 AudioDataFile::GetStringIndex(const char* string)
{
	for (U32 i = 0; i < m_apComponentData.Size(); i++)
	{
		if (m_apComponentData[i]->GetName())
		{
			if (strcmp(m_apComponentData[i]->GetName(), string) == 0)
				return i;
		}
	}

	return -1;
}


//------------------------------------------------------------------------------
//  GetCurrentStringTableSize
//------------------------------------------------------------------------------
U32 AudioDataFile::GetCurrentStringTableSize()
{
	return m_u32CurrCompNameStringTableSize;
//	return (m_apComponentData.Size()) ? m_apComponentData[m_apComponentData.Size() - 1]->GetOffset() + strlen(m_apComponentData[m_apComponentData.Size() - 1]->GetName()) + 1 : 0;
}


//------------------------------------------------------------------------------
//  SetComponentData
//------------------------------------------------------------------------------
void AudioDataFile::SetComponentData(void* pData, U32 u32Size, U32 u32Type)
{
	m_apComponentData[m_u32CurrComponentIndex]->SetData(pData, u32Size);

//	Assert(u32Type == m_apComponentData[m_u32CurrComponentIndex]->GetType());
//	m_apComponentData[m_u32CurrComponentIndex]->SetType(u32Type);

	m_u32ComponentDataSize += u32Size + 4;
//	m_u32CurrComponentIndex++;
}


//------------------------------------------------------------------------------
//  AssociateDataWithComponent
//------------------------------------------------------------------------------
void AudioDataFile::AssociateDataWithComponent(ComponentData* pCompData, U32 u32Id, void* pData, U32 u32DataSize)
{
	pCompData->AddChildData(u32Id, pData, u32DataSize);
	IncrementDataSize(u32DataSize);
}


//------------------------------------------------------------------------------
//  AllocateComponent
//------------------------------------------------------------------------------
void AudioDataFile::AllocateComponent(const char* strName, U32 eType, const char* strFile, U32 u32LineNum, NamedGroupLevel* pNamedGroup)
{
	ComponentData* pCompData = 0;

	if (strName)
	{
		pCompData = new ComponentData(strName);
		pCompData->SetNameOffset(GetCurrentStringTableSize());
		m_u32CurrCompNameStringTableSize += U32(strlen(strName) + 1);
		pCompData->SetNamedGroup(pNamedGroup);
	}
	else
		pCompData = new ComponentData();

	pCompData->SetType(eType);
	pCompData->SetXmlFile(strFile);
	pCompData->SetLineNumber(u32LineNum);
	m_apComponentData.PushBack(pCompData);
}


//------------------------------------------------------------------------------
//  AddNonComponentString
//------------------------------------------------------------------------------
U32 AudioDataFile::AddNonComponentString(const char* strName)
{
	U32 u32CurrValue = GetNonComponentStringOffset(strName);

	if (u32CurrValue != -1)	// ..string has already been added
		return u32CurrValue;

	char* strToAdd = new char[strlen(strName) + 1];
	sprintf(strToAdd, strName);

	NonComponentNameStringData* pNonComponentNameStringData = new NonComponentNameStringData;

	pNonComponentNameStringData->m_strName = strToAdd;
	pNonComponentNameStringData->m_u32Offset = (m_aNonComponentNameStrings.Size()) ? m_aNonComponentNameStrings[m_aNonComponentNameStrings.Size() - 1]->m_u32Offset + strlen(m_aNonComponentNameStrings[m_aNonComponentNameStrings.Size() - 1]->m_strName) + 1 : 0;

	m_aNonComponentNameStrings.PushBack(pNonComponentNameStringData);

	return pNonComponentNameStringData->m_u32Offset + GetCurrentStringTableSize() + strlen(m_strObjectName) + 1;
}


//------------------------------------------------------------------------------
//  GetNonComponentStringOffset
//------------------------------------------------------------------------------
U32 AudioDataFile::GetNonComponentStringOffset(const char* strName)
{
	for (U32 i = 0; i < m_aNonComponentNameStrings.Size(); i++)
	{
		if (strcmp(m_aNonComponentNameStrings[i]->m_strName, strName) == 0)
		{
			return m_aNonComponentNameStrings[i]->m_u32Offset + GetCurrentStringTableSize() + strlen(m_strObjectName) + 1;
		}
	}

	return -1;
}


//------------------------------------------------------------------------------
//  GetFloatValue (1st overload)
//------------------------------------------------------------------------------
bool AudioDataFile::GetFloatValue(const char* pchValue, const F32* pfRetValue)
{
	Assert(pfRetValue);

	// validate what we're dealing with here is actually a float
	for (U32 i = 0; i < strlen(pchValue); i++)
	{
		if ((pchValue[i] < '0' || pchValue[i] > '9') && pchValue[i] != '.' && pchValue[i] != '-')
			return false;
	}

	sscanf(pchValue, "%f", pfRetValue);
	return true;
}

//------------------------------------------------------------------------------
//  GetFloatValue (2nd overload)
//------------------------------------------------------------------------------
bool AudioDataFile::GetFloatValue(const char* pchValue, const AudioFloat* pfRetValue)
{
	Assert(pfRetValue);

	F32 f32RetValue = (AudioFloat)(*(pfRetValue));

	bool bSuccess = GetFloatValue(pchValue, &f32RetValue);

	(*((AudioFloat*)pfRetValue)) = f32RetValue;

	return bSuccess;
}

//------------------------------------------------------------------------------
//  GetIntValue (1st overload)
//------------------------------------------------------------------------------
bool AudioDataFile::GetIntValue(const char* pchValue, AudioS32Convertor* ps32RetValue)
{
	Assert(ps32RetValue);

	// validate what we're dealing with here is actually an int
	for (U32 i = 0; i < strlen(pchValue); i++)
	{
		if (pchValue[i] < '0' || pchValue[i] > '9')
			return false;
	}

	S32 s32Value = 0;
	sscanf(pchValue, "%d", &s32Value);
	*(ps32RetValue) = s32Value;
	return true;
}

//------------------------------------------------------------------------------
//  GetIntValue (2nd overload)
//------------------------------------------------------------------------------
bool AudioDataFile::GetIntValue(const char* pchValue, AudioU16Convertor* pu16RetValue)
{
	Assert(pu16RetValue);

	// validate what we're dealing with here is actually an int
	for (U32 i = 0; i < strlen(pchValue); i++)
	{
		if (pchValue[i] < '0' || pchValue[i] > '9')
			return false;
	}

	U32 u32Value = 0;
	sscanf(pchValue, "%d", &u32Value);

	U16 u16Value = (U16)u32Value;

	*(pu16RetValue) = u16Value;
	return true;
}

//------------------------------------------------------------------------------
//  GetComponent
//------------------------------------------------------------------------------
bool AudioDataFile::GetComponent(const char* strName)
{
	if (! m_pchType)
		return false;

	if (! strlen(m_pchType))
		return false;

	if (strcmp(m_pchType, strName) != 0)
		return false;

	m_u32CurrComponentIndex++;

	m_GroupNameManager.SetGroupLevel(m_apComponentData[m_u32CurrComponentIndex]->m_pNamedGroupLevel);

	return true;
}

//------------------------------------------------------------------------------
//  GetFloatAttribute
//------------------------------------------------------------------------------
bool AudioDataFile::GetFloatAttribute(const char* strName, AudioFloat* pFloat)
{
	m_strWorkString.clear();

	if (GetValueFromKey(m_pchAttributes, strName, m_strWorkString))
	{
		if (! GetFloatValue(m_strWorkString.data(), pFloat))
		{
			char strError[1024] = {0};
			sprintf(strError, "%s does not contain valid float", strName);
			DoError(strError, m_pParentNode->GetLine(), m_pParentNode->GetFilename());

			return false;
		}
		else
			return true;
	}
	else
		return false;
}

//------------------------------------------------------------------------------
//  GetIntAttribute
//------------------------------------------------------------------------------
bool AudioDataFile::GetIntAttribute(const char* strName, AudioS32Convertor* pInt)
{
	m_strWorkString.clear();

	if (GetValueFromKey(m_pchAttributes, strName, m_strWorkString))
	{
		if (! GetIntValue(m_strWorkString.data(), pInt))
		{
			char strError[1024] = {0};
			sprintf(strError, "%s does not contain valid integer", strName);
			DoError(strError, m_pParentNode->GetLine(), m_pParentNode->GetFilename());

			return false;
		}
		else
			return true;
	}
	else
		return false;
}

//------------------------------------------------------------------------------
//  GetUnsignedIntAttribute
//------------------------------------------------------------------------------
bool AudioDataFile::GetUnsignedIntAttribute(const char* strName, AudioU16Convertor* pInt)
{
	m_strWorkString.clear();

	if (GetValueFromKey(m_pchAttributes, strName, m_strWorkString))
	{
		if (! GetIntValue(m_strWorkString.data(), pInt))
		{
			char strError[1024] = {0};
			sprintf(strError, "%s does not contain valid integer", strName);
			DoError(strError, m_pParentNode->GetLine(), m_pParentNode->GetFilename());

			return false;
		}
		else
			return true;
	}
	else
		return false;
}

//------------------------------------------------------------------------------
//  GetStringOffsetAttribute
//------------------------------------------------------------------------------
bool AudioDataFile::GetStringOffsetAttribute(const char* strName, StrOffset* pInt)
{
	m_strWorkString.clear();

	if (GetValueFromKey(m_pchAttributes, strName, m_strWorkString))
	{
		// first char should be a letter
		if ((m_strWorkString.data()[0] < 'A' || m_strWorkString.data()[0] > 'Z')
			&& (m_strWorkString.data()[0] < 'a' || m_strWorkString.data()[0] > 'z'))
		{
			DoError("string does not begin with a letter", m_pParentNode->GetLine(), m_pParentNode->GetFilename());
			return false;
		}

		*(pInt) = GetStringOffset(m_u32CurrComponentIndex);
		return true;
	}
	else
		return false;
}

//------------------------------------------------------------------------------
//  GetNonComponentStringOffsetAttribute
//------------------------------------------------------------------------------
bool AudioDataFile::GetNonComponentStringOffsetAttribute(const char* strName, StrOffset* pInt)
{
	m_strWorkString.clear();

	if (GetValueFromKey(m_pchAttributes, strName, m_strWorkString))
	{
		*(pInt) = AddNonComponentString(m_strWorkString.data());
		return true;
	}
	else
		return false;
}

//------------------------------------------------------------------------------
//  GetBankAttribute
//------------------------------------------------------------------------------
bool AudioDataFile::GetBankAttribute(StrOffset* pInt)
{
	if (GetNonComponentStringOffsetAttribute("Bank", pInt))
	{
		m_WaveBankList.Add(Btl::String(m_strWorkString.data()) + Btl::String(".baf"));
		return true;
	}
	return false;
}
//------------------------------------------------------------------------------
//	MapComponentNameToIndex
//------------------------------------------------------------------------------
bool AudioDataFile::MapComponentNameToIndex(U32* pu32ComponentIndex, const char* strComponentString, const char* strComponentName)
{
	*(pu32ComponentIndex) = GetStringIndex(strComponentString);
	if (*(pu32ComponentIndex) == -1)
	{
		char ErrorMsg[1024] = {0};
		sprintf(ErrorMsg, "unrecognised component specified for %s", strComponentName);
		DoError(ErrorMsg, m_pParentNode->GetLine(), m_pParentNode->GetFilename());
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
//	GetCompAttrShared
//------------------------------------------------------------------------------
bool AudioDataFile::GetCompAttrShared(const char* strName, U32* pu32ComponentIndex, NamedGroupLevel* pNamedGroup, char* strOutputString)
{
	m_strWorkString.clear();

	if (strOutputString)
		strOutputString[0] = 0;

	if (! GetValueFromKey(m_pchAttributes, strName, m_strWorkString))
		return false;

	char* strCompName = (char*)(m_strWorkString.data());

	ComponentStringBreakdown breakdown;

	bool bOk = AnalyzeComponentNameCommon(m_strWorkString, pNamedGroup, breakdown);

	if (! bOk)
	{
		char strError[256] = {0};
		sprintf(strError, "Error processing output string in script for component %s", breakdown.strNameIncludingGroup);
	    DoError(strError, m_pParentNode->GetLine(), m_pParentNode->GetFilename());
		return false;		
	}

	if (strlen(breakdown.strOutputString) && strOutputString)
		sprintf(strOutputString, breakdown.strOutputString);

	return MapComponentNameToIndex(pu32ComponentIndex, breakdown.strNameIncludingGroup, strCompName);
}

//------------------------------------------------------------------------------
//	ValidateComponentType
//------------------------------------------------------------------------------
bool AudioDataFile::ValidateComponentType(U32 u32ComponentIndex, U32 u32NumTypesToValidateAgainst, va_list* pvl)
{
	bool bFoundValidType = false;

	for (U32 i = 0; i < u32NumTypesToValidateAgainst; i++)
	{
		U32 eType = va_arg(*(pvl), unsigned int);
		if (eType == m_apComponentData[u32ComponentIndex]->GetType())
			bFoundValidType = true;
	}

	return bFoundValidType;
}

//------------------------------------------------------------------------------
//	ResolveOutputFromString
//------------------------------------------------------------------------------
U16 AudioDataFile::ResolveOutputFromString(Com sCom, char* strOutput)
{
	if (! strOutput || ! strlen(strOutput))
		return 0xffff;

	char* strTemp = strOutput;

	bool bAllNumeric = true;

	for (; strTemp[0] != '0' && bAllNumeric; strTemp = &(strTemp[1]))
	{
		if (strTemp[0] < '0' || strTemp[0] > '9')
			bAllNumeric = false;
	}

	if (bAllNumeric)
		return atoi(strOutput);

	U32 u32Type = m_apComponentData[(S16)sCom]->GetType();

	switch (u32Type)
	{
		case Component::eArray:
			return (U16)(Array::GetControl(strOutput));

		case Component::eCone:
			return (U16)(Cone::GetControl(strOutput));

		case Component::eControlInput:
			return (U16)(ControlInput::GetControl(strOutput));

		case Component::eCoordinateInput:
			return (U16)(CoordinateInput::GetControl(strOutput));

		case Component::eDopplerShift:
			return (U16)(DopplerShift::GetControl(strOutput));

		case Component::eEnvelope:
			return (U16)(Envelope::GetControl(strOutput));

		case Component::eExponent:
			return (U16)(Exponent::GetControl(strOutput));

		case Component::eFraction:
			return (U16)(Fraction::GetControl(strOutput));

		case Component::eGateSwitch:
			return (U16)(GateSwitch::GetControl(strOutput));

		case Component::eGradient:
			return (U16)(Gradient::GetControl(strOutput));

		case Component::eLowPass:
			return (U16)(LowPass::GetControl(strOutput));

		case Component::eMax:
			return (U16)(Max::GetControl(strOutput));

		case Component::eMetronome:
			return (U16)(Metronome::GetControl(strOutput));

		case Component::eMin:
			return (U16)(Min::GetControl(strOutput));

		case Component::eMonoCompressor:
			return (U16)(MonoCompressor::GetControl(strOutput));

		case Component::eStereoCompressor:
			return (U16)(StereoCompressor::GetControl(strOutput));

		case Component::eMonoLipSync:
			return (U16)(MonoLipSync::GetControl(strOutput));

		case Component::eMonoLoopingSound:
			return (U16)(MonoLoopingSound::GetControl(strOutput));

		case Component::eMonoOneShotSound:
			return (U16)(MonoOneShotSound::GetControl(strOutput));

		case Component::eMonoOneShotSoundTable:
			return (U16)(MonoOneShotSoundTable::GetControl(strOutput));

		case Component::eParameterIn:
			return (U16)(ParameterIn::GetControl(strOutput));

		case Component::ePiecewiseLinearFunction:
			return (U16)(PiecewiseLinearFunction::GetControl(strOutput));

		case Component::eProduct:
			return (U16)(Product::GetControl(strOutput));

		case Component::eQuantize:
			return (U16)(Quantize::GetControl(strOutput));

		case Component::eRandom:
			return (U16)(Random::GetControl(strOutput));

		case Component::eRolloff:
			return (U16)(Rolloff::GetControl(strOutput));

		case Component::eSampleAndHold:
			return (U16)(SampleAndHold::GetControl(strOutput));

		case Component::eSelector:
			return (U16)(Selector::GetControl(strOutput));

		case Component::eSource:
			return (U16)(Source::GetControl(strOutput));

		case Component::eStereoLoopingSound:
			return (U16)(StereoLoopingSound::GetControl(strOutput));

		case Component::eStereoOneShotSound:
			return (U16)(StereoOneShotSound::GetControl(strOutput));

		case Component::eStereoOneShotSoundTable:
			return (U16)(StereoOneShotSoundTable::GetControl(strOutput));

		case Component::eSum:
			return (U16)(Sum::GetControl(strOutput));

		case Component::eTimer:
			return (U16)(Timer::GetControl(strOutput));

		case Component::eVariable:
			return (U16)(Variable::GetControl(strOutput));

		case Component::eVector:
			return (U16)(Vector::GetControl(strOutput));

		case Component::eVectorIn:
			return (U16)(VectorIn::GetControl(strOutput));

		case Component::eVectorOut:
			return (U16)(VectorOut::GetControl(strOutput));

		case Component::eVectorNearestPointOnLine:
			return (U16)(VectorNearestPointOnLine::GetControl(strOutput));

		case Component::eMonoRMSLevel:
			return (U16)(MonoRMSLevel::GetControl(strOutput));

		case Component::eMonoOscilloscope:
			return (U16)(MonoOscilloscope::GetControl(strOutput));

		case Component::eStereoRMSLevel:
			return (U16)(StereoRMSLevel::GetControl(strOutput));

		case Component::eImpactFilter:
			return (U16)(ImpactFilter::GetControl(strOutput));

		case Component::eMonoSpectralEnvelope:
			return (U16)(MonoSpectralEnvelope::GetControl(strOutput));

		case Component::eStereoSpectralEnvelope:
			return (U16)(StereoSpectralEnvelope::GetControl(strOutput));

		case Component::eMonoToMonoMixer:
		case Component::eMonoToStereoMixer:
		case Component::eMonoToSurroundMixer:
		case Component::eMonoToSurroundLFEMixer:
		case Component::eStereoToMonoMixer:
		case Component::eStereoToStereoMixer:
		case Component::eStereoToQuadMixer:
		case Component::eStereoToSurroundMixer:
		case Component::eQuadToSurroundMixer:
		case Component::eSurroundToSurroundMixer:
		case Component::eSurroundToSurroundLFEMixer:
			return (U16)(MixerComponent::GetControl(strOutput));
	}

	return 0xffff;
}

//------------------------------------------------------------------------------
//	GetComponentAttribute
//------------------------------------------------------------------------------
bool AudioDataFile::GetComponentAttribute(const char* strName, Com* pCom, va_list* pvl, U32 u32NumTypesToValidateAgainst)
{
	U32 u32ComponentIndex;
	if (! GetCompAttrShared(strName, &u32ComponentIndex, m_GroupNameManager.GetCurrentGroupLevel(), m_strComponentOutputString))
		return false;

	if (u32NumTypesToValidateAgainst)
	{
		bool bFoundValidType = ValidateComponentType(u32ComponentIndex, u32NumTypesToValidateAgainst, pvl);

		if (! bFoundValidType)
		{
			char ErrorMsg[1024] = {0};
			sprintf(ErrorMsg, "Invalid component type used as input for %s", strName);
			DoWarning(ErrorMsg, m_pParentNode->GetLine(), m_pParentNode->GetFilename());
		}
	}

	// check if the retrieved component index is the same as the component we're currently setting up.
	// if it is, the component is feeding back into itself. Imagine that!!
	if (u32ComponentIndex == m_u32CurrComponentIndex)
		DoWarning("Component is feeding back into itself.", m_pParentNode->GetLine(), m_pParentNode->GetFilename());

	*(pCom) = u32ComponentIndex;
	return true;
}

//------------------------------------------------------------------------------
//	GetComponentAttribute (overloaded)
//------------------------------------------------------------------------------
bool AudioDataFile::GetComponentAttribute(const char* strName, Com* pCom, U32 u32NumTypesToValidateAgainst, U32 eFirstType, ...)
{
	U32 u32ComponentIndex;
	if (! GetCompAttrShared(strName, &u32ComponentIndex, m_GroupNameManager.GetCurrentGroupLevel(), m_strComponentOutputString))
		return false;

	if (u32NumTypesToValidateAgainst)
	{
		bool bFoundValidType = false;
		va_list vl;
		va_start(vl, u32NumTypesToValidateAgainst);

		bFoundValidType = ValidateComponentType(u32ComponentIndex, u32NumTypesToValidateAgainst, &vl);

		va_end(vl);

		if (! bFoundValidType)
		{
			char ErrorMsg[1024] = {0};
			sprintf(ErrorMsg, "Invalid component type used as input for %s", strName);
			DoWarning(ErrorMsg, m_pParentNode->GetLine(), m_pParentNode->GetFilename());
		}
	}

	// check if the retrieved component index is the same as the component we're currently setting up.
	// if it is, the component is feeding back into itself. Imagine that!!
	if (u32ComponentIndex == m_u32CurrComponentIndex)
		DoWarning("Component is feeding back into itself.", m_pParentNode->GetLine(), m_pParentNode->GetFilename());

	*(pCom) = u32ComponentIndex;
	return true;
}

//------------------------------------------------------------------------------
//	GetComponentOrFloatAttributes
//------------------------------------------------------------------------------
bool AudioDataFile::GetComponentOrFloatAttributes(const char* strName, Com* pCom, AudioFloat* pFloat, U32 u32NumTypesToValidateAgainst, U32 eFirstType, ...)
{
	if (strlen(strName) >= 61)
		DoError("String too long", m_pParentNode->GetLine(), m_pParentNode->GetFilename());

	char strCompName[64] = {0};
	char strValName[64] = {0};

	sprintf(strCompName, "%sCom", strName);
	sprintf(strValName, "%sVal", strName);

	m_strWorkString.clear();
	if (GetValueFromKey(m_pchAttributes, strCompName, m_strWorkString))
	{
		ComponentStringBreakdown breakdown;

		bool bOk = AnalyzeComponentNameCommon(m_strWorkString.data(), m_GroupNameManager.GetCurrentGroupLevel(), breakdown);

		if (! bOk)
		{
			char strError[256] = {0};
			sprintf(strError, "Error processing output string in script for component %s", breakdown.strNameIncludingGroup);
			DoError(strError, m_pParentNode->GetLine(), m_pParentNode->GetFilename());
			return false;		
		}

		// check list of constants
		for (U32 i = 0; i < m_aConstants.Size(); i++)
		{
			if (m_aConstants[i]->IsCalled((char*)breakdown.strNameIncludingGroup))
			{
				*(pFloat) = m_aConstants[i]->GetValue();
				return true;
			}
		}
	}

	va_list vl;
	va_start(vl, u32NumTypesToValidateAgainst);
	bool bSuccess = GetComponentAttribute(strCompName, pCom, &vl, u32NumTypesToValidateAgainst);
	va_end(vl);

	if (! bSuccess)
		bSuccess = GetFloatAttribute(strValName, pFloat);

	return bSuccess;
}

//------------------------------------------------------------------------------
//	GetComponentAndOutputAttributes
//------------------------------------------------------------------------------
bool AudioDataFile::GetComponentAndOutputAttributes(const char* strName, Audio::Component::Control* pControl)
{
	char strCompName[64] = {0};

	sprintf(strCompName, "%sCom", strName);

	bool bSuccess = GetComponentAttribute(strCompName, &(pControl->sCom));

	if (! bSuccess)
		return false;

	bool bOutputString = false;

	if (strlen(m_strComponentOutputString))
	{
		bOutputString = true;
		U16 u16Out = ResolveOutputFromString(pControl->sCom, m_strComponentOutputString);

		if (u16Out != 0xffff)
			pControl->uOut = u16Out;
		else
		{
			char strError[256] = {0};
			sprintf(strError, "Output string %s doesn't make sense");
			DoWarning(strError,  m_pParentNode->GetLine(), m_pParentNode->GetFilename());
			return true;
		}
	}

	if (strlen(strName) >= 61)
		DoError("String too long", m_pParentNode->GetLine(), m_pParentNode->GetFilename());

	char strOutputName[64] = {0};

	sprintf(strOutputName, "%sOut", strName);

	bool bOutAttribute = GetUnsignedIntAttribute(strOutputName, &(pControl->uOut));

	if (bOutputString && bOutAttribute)
		DoWarning("Both output attribute and output reference in component name have been specified", m_pParentNode->GetLine(), m_pParentNode->GetFilename());

	return true;
}

//------------------------------------------------------------------------------
//	GetComponentFloatOutputAttributes
//------------------------------------------------------------------------------
bool AudioDataFile::GetComponentFloatOutputAttributes(const char* strName, Audio::Component::Option* pOption)
{
	bool bSuccess = GetComponentOrFloatAttributes(strName, &(pOption->ctrl.sCom), &(pOption->fVal));

	if (! bSuccess)
		return false;

	bool bOutputString = false;

	if (strlen(m_strComponentOutputString))
	{
		bOutputString = true;
		U16 u16Out = ResolveOutputFromString(pOption->ctrl.sCom, m_strComponentOutputString);

		if (u16Out != 0xffff)
			pOption->ctrl.uOut = u16Out;
		else
		{
			char strError[256] = {0};
			sprintf(strError, "Output string %s doesn't make sense", m_strComponentOutputString ? m_strComponentOutputString : "(null)");
			DoWarning(strError,  m_pParentNode->GetLine(), m_pParentNode->GetFilename());
			return true;
		}
	}

	if (strlen(strName) >= 61)
		DoError("String too long", m_pParentNode->GetLine(), m_pParentNode->GetFilename());

	char strOutputName[64] = {0};

	sprintf(strOutputName, "%sOut", strName);

	bool bOutAttribute = GetUnsignedIntAttribute(strOutputName, &(pOption->ctrl.uOut));

	if (bOutputString && bOutAttribute)
		DoWarning("Both output attribute and output reference in component name have been specified", m_pParentNode->GetLine(), m_pParentNode->GetFilename());

	return true;
}

//------------------------------------------------------------------------------
//	LookupEvent
//------------------------------------------------------------------------------
#ifdef NEW_EVENT_CODE
bool AudioDataFile::LookupEvent(const char* strName, Audio::Component::Event& ev)
{
	char strCompNameToLookup[256] = {0};
	m_strWorkString.clear();
	sprintf(strCompNameToLookup, "%sScr", strName);

	if (! GetValueFromKey(m_pchAttributes, strCompNameToLookup, m_strWorkString))
		return false;

	char* strCompAttribute = (char*)(m_strWorkString.data());

	S32 s32EventStringIndex = -1;

	for (S32 i = 0; i < strlen(strCompAttribute) && s32EventStringIndex == -1; i++)
	{
		if (strCompAttribute[i] == '#')
			s32EventStringIndex = i;
	}

	if (s32EventStringIndex == -1)
	{
		return GetComponentAttribute(strCompNameToLookup, &(ev.sCom));
	}

	char* strEventName = (char*)(&(strCompAttribute[s32EventStringIndex + 1]));

	if (! strlen(strEventName))
	{
		DoWarning("Error in event lookup", m_pParentNode->GetLine(), m_pParentNode->GetFilename());
		return false;
	}

	char strComponentStringWithEventStripped[256] = {0};
	strcpy(strComponentStringWithEventStripped, strCompAttribute);
	strComponentStringWithEventStripped[s32EventStringIndex] = 0;

	U32 u32ComponentIndex;
	MapComponentNameToIndex(&u32ComponentIndex, strComponentStringWithEventStripped, strCompNameToLookup);

	ev.sCom = u32ComponentIndex;

	U32 u32Type = m_apComponentData[ev.sCom]->GetType();

	switch (u32Type)
	{
		case Component::eMonoCompressor:
			ev.uMsg = MonoCompressor::GetMessage(strEventName);
			break;

		case Component::eStereoCompressor:
			ev.uMsg = StereoCompressor::GetMessage(strEventName);
			break;

		case Component::eMonoConvolution:
			ev.uMsg = MonoConvolution::GetMessage(strEventName);
			break;

		case Component::eMonoDelay:
			ev.uMsg = MonoDelay::GetMessage(strEventName);
			break;

		case Component::eMonoRingModulator:
			ev.uMsg = MonoRingModulator::GetMessage(strEventName);
			break;

		case Component::eSurroundRingModulator:
			ev.uMsg = SurroundRingModulator::GetMessage(strEventName);
			break;

		case Component::eMonoGain:
			ev.uMsg = MonoGain::GetMessage(strEventName);
			break;

		case Component::eMonoHighPass:
			ev.uMsg = MonoHighPass::GetMessage(strEventName);
			break;

		case Component::eSurroundHighPass:
			ev.uMsg = MonoHighPass::GetMessage(strEventName);
			break;

		case Component::eMonoLipSync:
			ev.uMsg = MonoLipSync::GetMessage(strEventName);
			break;

		case Component::eMonoLoopingSound:
			ev.uMsg = MonoLoopingSound::GetMessage(strEventName);
			break;

		case Component::eMonoLowPass:
			ev.uMsg = MonoLowPass::GetMessage(strEventName);
			break;

		case Component::eMonoSpectralReverb:
			ev.uMsg = MonoSpectralReverb::GetMessage(strEventName);
			break;

		case Component::eMonoSpectralEnvelope:
			ev.uMsg = MonoSpectralEnvelope::GetMessage(strEventName);
			break;

		case Component::eStereoSpectralEnvelope:
			ev.uMsg = StereoSpectralEnvelope::GetMessage(strEventName);
			break;

		case Component::eMonoOneShotSound:
			ev.uMsg = MonoOneShotSound::GetMessage(strEventName);
			break;

		case Component::eMonoOneShotSoundTable:
			ev.uMsg = MonoOneShotSoundTable::GetMessage(strEventName);
			break;

		case Component::eMonoPeakEQ:
			ev.uMsg = MonoPeakEQ::GetMessage(strEventName);
			break;

		case Component::eMonoPhaser:
			ev.uMsg = MonoPhaser::GetMessage(strEventName);
			break;

		case Component::eMonoFlanger:
			ev.uMsg = MonoFlanger::GetMessage(strEventName);
			break;

		case Component::eSurroundFlanger:
			ev.uMsg = SurroundFlanger::GetMessage(strEventName);
			break;

		case Component::eMonoResonantLowPass:
			ev.uMsg = MonoResonantLowPass::GetMessage(strEventName);
			break;

		case Component::eSurround24dBResonantLowPass:
			ev.uMsg = Surround24dBResonantLowPass::GetMessage(strEventName);
			break;

		case Component::eMonoSaturator:
			ev.uMsg = MonoSaturator::GetMessage(strEventName);
			break;

		case Component::eRandom:
			ev.uMsg = Random::GetMessage(strEventName);
			break;

		case Component::eSampleAndHold:
			ev.uMsg = SampleAndHold::GetMessage(strEventName);
			break;

		case Component::eScript:
			ev.uMsg = Script::GetMessage(strEventName);
			break;

		case Component::eSelector:
			ev.uMsg = Selector::GetMessage(strEventName);
			break;

		case Component::eStereoLoopingSound:
			ev.uMsg = StereoLoopingSound::GetMessage(strEventName);
			break;

		case Component::eStereoLowPass:
			ev.uMsg = StereoLowPass::GetMessage(strEventName);
			break;

		case Component::eStereoOneShotSound:
			ev.uMsg = StereoOneShotSound::GetMessage(strEventName);
			break;

		case Component::eStereoOneShotSoundTable:
			ev.uMsg = StereoOneShotSoundTable::GetMessage(strEventName);
			break;

		case Component::eStereoPitchShift:
			ev.uMsg = StereoPitchShift::GetMessage(strEventName);
			break;

		case Component::eStereoResonantLowPass:
			ev.uMsg = StereoResonantLowPass::GetMessage(strEventName);
			break;

		case Component::eSurroundCompressor:
			ev.uMsg = SurroundCompressor::GetMessage(strEventName);
			break;

		case Component::eSurroundDelay:
			ev.uMsg = SurroundDelay::GetMessage(strEventName);
			break;

		case Component::eSurroundLowShelf:
			ev.uMsg = SurroundLowShelf::GetMessage(strEventName);
			break;

		case Component::eSurroundPeakEQ:
			ev.uMsg = SurroundPeakEQ::GetMessage(strEventName);
			break;

		case Component::eSurroundResonantLowPass:
			ev.uMsg = SurroundResonantLowPass::GetMessage(strEventName);
			break;

		case Component::eTimer:
			ev.uMsg = Timer::GetMessage(strEventName);
			break;

		case Component::eMonoRMSLevel:
			ev.uMsg = MonoRMSLevel::GetMessage(strEventName);
			break;

		case Component::eStereoRMSLevel:
			ev.uMsg = StereoRMSLevel::GetMessage(strEventName);
			break;

		case Component::eMessageDelay:
			ev.uMsg = MessageDelay::GetMessage(strEventName);
			break;

		case Component::eImpactFilter:
			ev.uMsg = ImpactFilter::GetMessage(strEventName);
			break;

		default:
			DoError("Invalid Event specified", m_pParentNode->GetLine(), m_pParentNode->GetFilename());
			return false;
	}

	return true;
}
#endif	//	#ifndef NEW_EVENT_CODE

//------------------------------------------------------------------------------
//	CheckSurroundAndIncEffects
//------------------------------------------------------------------------------
bool AudioDataFile::CheckSurroundAndIncEffects(ComponentData* pCompData)
{
	switch (pCompData->GetType())
	{
		case Component::eMonoToSurroundMixer:
			{
				Audio::MonoToSurroundMixer::Data* pParentData = (Audio::MonoToSurroundMixer::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		case Component::eStereoToSurroundMixer:
			{
				Audio::StereoToSurroundMixer::Data* pParentData = (Audio::StereoToSurroundMixer::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		case Component::eSurroundToSurroundMixer:
			{
				Audio::SurroundToSurroundMixer::Data* pParentData = (Audio::SurroundToSurroundMixer::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		default:
			break;
	}

	return false;
}

//------------------------------------------------------------------------------
//	CheckStereoAndIncEffects
//------------------------------------------------------------------------------
bool AudioDataFile::CheckStereoAndIncEffects(ComponentData* pCompData)
{
	switch (pCompData->GetType())
	{
		case Component::eStereoOneShotSound:
			{
				Audio::StereoOneShotSound::Data* pParentData = (Audio::StereoOneShotSound::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		case Component::eStereoOneShotSoundTable:
			{
				Audio::StereoOneShotSoundTable::Data* pParentData = (Audio::StereoOneShotSoundTable::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		case Component::eStereoLoopingSound:
			{
				Audio::StereoOneShotSound::Data* pParentData = (Audio::StereoOneShotSound::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		case Component::eMonoToStereoMixer:
			{
				Audio::MonoToStereoMixer::Data* pParentData = (Audio::MonoToStereoMixer::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		case Component::eStereoToStereoMixer:
			{
				Audio::StereoToStereoMixer::Data* pParentData = (Audio::StereoToStereoMixer::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		default:
			break;
	}

	return false;
}

//------------------------------------------------------------------------------
//	CheckMonoAndIncEffects
//------------------------------------------------------------------------------
bool AudioDataFile::CheckMonoAndIncEffects(ComponentData* pCompData)
{
	switch (pCompData->GetType())
	{
		case Component::eMonoOneShotSound:
			{
				Audio::MonoOneShotSound::Data* pParentData = (Audio::MonoOneShotSound::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		case Component::eMonoLoopingSound:
			{
				Audio::MonoLoopingSound::Data* pParentData = (Audio::MonoLoopingSound::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		case Component::eMonoToMonoMixer:
			{
				Audio::MonoToMonoMixer::Data* pParentData = (Audio::MonoToMonoMixer::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		//case Component::eMonoSpectralToMonoMixer:
		//	{
		//		Audio::MonoSpectralToMonoMixer::Data* pParentData = (Audio::MonoSpectralToMonoMixer::Data*)(pCompData->GetData());
		//		pParentData->uEffects++;
		//		return true;
		//	}
		//	break;
		case Component::eStereoToMonoMixer:
			{
				Audio::StereoToMonoMixer::Data* pParentData = (Audio::StereoToMonoMixer::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		case Component::eMonoOneShotSoundTable:
			{
				Audio::MonoOneShotSoundTable::Data* pParentData = (Audio::MonoOneShotSoundTable::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				return true;
			}
			break;
		//case Component::eStereoSpectralToMonoMixer:
		//	{
		//		Audio::StereoSpectralToMonoMixer::Data* pParentData = (Audio::StereoSpectralToMonoMixer::Data*)(pCompData->GetData());
		//		pParentData->uEffects++;
		//		return true;
		//	}
		//	break;
		//case Component::eSurroundToMonoMixer:
		//	{
		//		Audio::SurroundToMonoMixer::Data* pParentData = (Audio::SurroundToMonoMixer::Data*)(pCompData->GetData());
		//		pParentData->uEffects++;
		//		return true;
		//	}
		//	break;
		//case Component::eSurroundSpectralToMonoMixer:
		//	{
		//		Audio::SurroundSpectralToMonoMixer::Data* pParentData = (Audio::SurroundSpectralToMonoMixer::Data*)(pCompData->GetData());
		//		pParentData->uEffects++;
		//		return true;
		//	}
		//	break;
			//[ciano] - ready to be added when needed
	/*	case Component::eMonoAudioInput:
			{
				Audio::MonoAudioInput::Data* pParentData = (Audio::MonoAudioInput::Data*)(pCompData->GetData());
				pParentData->uEffects++;
				printf("Effect: %s\n\n",pCompData->GetName());
				return true;
			}
			break;*/
		default:
			break;
	}

	return false;
}

//------------------------------------------------------------------------------
//	DoError
//------------------------------------------------------------------------------
bool AudioDataFile::DoError(const char* pchWarning, int iLine, const char* pchFile, bool bFatal)
{
	ErrorData errorData;
	errorData.m_strErrorString = (char*)pchWarning;
	errorData.m_u32LineNumber = iLine;
	errorData.m_strFilename =  (char*)pchFile;
	if (bFatal)
		errorData.m_eErrorLevel = eFatalError;

	return m_Diagnostics.ReceiveError(&errorData);
}

//------------------------------------------------------------------------------
//  DoWarning
//------------------------------------------------------------------------------
bool AudioDataFile::DoWarning(const char* pchWarning, int iLine, const char* pchFile)
{
	ErrorData errorData;
	errorData.m_strErrorString = (char*)pchWarning;
	errorData.m_u32LineNumber = iLine;
	errorData.m_strFilename =  (char*)pchFile;
	errorData.m_eErrorLevel = eWarning;

	return m_Diagnostics.ReceiveError(&errorData);
}

//------------------------------------------------------------------------------
//	RegisterAttribute
//------------------------------------------------------------------------------
void AudioDataFile::RegisterAttribute(const char* sName, const char* sValue)
{
	if (stricmp(sName, "Group") == 0 || stricmp(sName, "Fullname") == 0)
		return;

	std::string* sAttrName = new std::string(sName);
	m_aStrictParseAttrNames.PushBack(sAttrName);
}

//------------------------------------------------------------------------------
//	CreateValueListFromAttributeBlock
//------------------------------------------------------------------------------
void AudioDataFile::CreateValueListFromAttributeBlock(const char* pchData)
{
    char* pchCurr = (char*)pchData;
	bool bInQuotes = false;
	bool bReadingSubString = false;
	bool bReadingName = false;
	char sName[64];
	char sValue[128];
	sName[0] = 0;
	sValue[0] = 0;
	
	for (; pchCurr[0] != 0; pchCurr++)
	{
		if ((*pchCurr == ' ') || (*pchCurr == '\t') || (*pchCurr == '\n') || (*pchCurr == '\r'))
			continue;

		if (*pchCurr == '"' || *pchCurr == '\'')
		{
			Assert (! bReadingName);

			if (bInQuotes)
			{
				bInQuotes = false;
				bReadingSubString = false;
				RegisterAttribute(sName, sValue);
				sName[0] = 0;
				sValue[0] = 0;
				continue;
			}
			else
			{
				bInQuotes = true;
				sValue[0] = 0;
				continue;
			}
		}

		if (bInQuotes)
		{
			if (*pchCurr == '#')
				bReadingSubString = true;

			if (! bReadingSubString)
			{
				Assert (strlen(sValue) < 128);

				sValue[strlen(sValue) + 1] = 0;
				sValue[strlen(sValue)] = *pchCurr;
			}

			continue;
		}

		if (*pchCurr == '=')
		{
			Assert (bReadingName);
			bReadingName = false;
			continue;
		}
		else
			bReadingName = true;

		if (bReadingName)
		{

			Assert (strlen(sName) < 64);

			sName[strlen(sName) + 1] = 0;
			sName[strlen(sName)] = *pchCurr;
		}
	}
}
