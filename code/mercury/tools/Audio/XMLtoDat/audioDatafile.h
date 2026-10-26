#pragma once

#include "WaveBankList.h"

// forwards decls
class ComponentData;
class CommandLineApp;


enum DataTypes
{
	eDataTypeDefault = 0,
	eDataTypeInput = 0,
	eDataTypeEffect = 1,
	eDataTypeScript = 2
};


class ChildDataContainer
{

public:
	ChildDataContainer() { m_pNext = 0; m_pPrev = 0; m_pChild = 0; m_u32Size = 0; }
	~ChildDataContainer() { if (m_pNext) delete m_pNext; if (m_pChild) delete [] m_pChild; }

	void								SetData(void* pData, U32 u32Size);
	void								SetNext(ChildDataContainer* pNext) { m_pNext = pNext; }
	void								SetPrev(ChildDataContainer* pPrev) { m_pPrev = pPrev; }
	ChildDataContainer*					GetNext() { return m_pNext; }
	ChildDataContainer*					GetPrev() { return m_pPrev; }
	void*								GetData() { return m_pChild; }
	U32									GetSize() { return m_u32Size; }

private:
	ChildDataContainer*					m_pNext;
	ChildDataContainer*					m_pPrev;
	void*								m_pChild;
	U32									m_u32Size;
};


class ChildDataList
{

public:
	ChildDataList() { m_pNextList = 0; m_pFirstContainer = 0; m_pLastContainer = 0; m_u32NumChildren = 0; }
	~ChildDataList() { if (m_pNextList) delete m_pNextList; if (m_pFirstContainer) delete m_pFirstContainer; }

	U32									GetId() { return m_u32ListId; }
	ChildDataList*						GetNext() { return m_pNextList; }
	void								AddChild(void* pData, U32 u32Size);
	void								SetId(U32 u32Id) { m_u32ListId = u32Id; }
	void								SetNext(ChildDataList* pNext) { m_pNextList = pNext; }
	ChildDataContainer*					GetFirstContainer() { return m_pFirstContainer; }
	U32									GetNumChildren() { return m_u32NumChildren; }

private:
	ChildDataList*						m_pNextList;
	U32									m_u32ListId;
	ChildDataContainer*					m_pFirstContainer;
	ChildDataContainer*				    m_pLastContainer;
	U32									m_u32NumChildren;
};


class ComponentData
{

public:
	ComponentData();
	ComponentData(const char* strName);
	~ComponentData();

	char*								GetName() { return m_strName; }
	U32									GetOffset() { return m_u32Offset; }
	void								SetType(U32 u32Type) { m_u32ComponentType = u32Type; }
	void								SetNameOffset(U32 u32Offset) { m_u32Offset = u32Offset; }
	void								SetData(void* pData, U32 u32Size) { m_pComponentData = pData; m_u32DataSize = u32Size; }
	void*								GetData() { return m_pComponentData; }
	U32									GetType() { return m_u32ComponentType; }
	U32									GetDataSize() { return m_u32DataSize; }
	bool								SetName(const char* strName);
	void								AddChildData(U32 u32Id, void* pData, U32 u32DataSize);
	U32									GetNumChildren();
	ChildDataList*						GetFirstDataList() { return m_pFirstChildList; }
	bool								SetXmlFile(const char* strFile);
	void								SetLineNumber(U32 u32LineNumber) { m_u32XmlLineNumber = u32LineNumber; }
	char*								GetXmlFile() { return m_strXmlFile; }
	U32									GetLineNumber() { return m_u32XmlLineNumber; }
	void								SetNamedGroup(NamedGroupLevel* pNamedGroup) { m_pNamedGroupLevel = pNamedGroup; }
	NamedGroupLevel*					GetGroupLevel() { return m_pNamedGroupLevel; }

private:
	ChildDataList*						GetDataList(U32 u32Id);

	void*								m_pComponentData;
	char*								m_strName;
	U32									m_u32ComponentType;
	U32									m_u32Offset;
	U32									m_u32DataSize;
	ChildDataList*						m_pFirstChildList;
	U32									m_u32XmlLineNumber;
	char*								m_strXmlFile;
public:	// temp
	NamedGroupLevel*					m_pNamedGroupLevel;
};


class NonComponentNameStringData
{

public:
	NonComponentNameStringData() { m_strName = 0; m_u32Offset = 0; }
	~NonComponentNameStringData() { if (m_strName) delete [] m_strName; }

	char*								m_strName;
	U32									m_u32Offset;
};


struct AudioDataFileHeader
{

	char								m_strTag[4];
	U32									m_u32VersionNumber;
	U32									m_u32StringTableOffset;
	U32									m_u32ObjectNameStringIndex;
	U32									m_u32NumComponents;
	U32									m_u32FrameSkip;
};


class AudioDataFile
{

public:
	AudioDataFile();
	~AudioDataFile();

	void								Insert(xml::TagNode* pTag);
	bool								LoadXml(char* strFilename, char* strError = 0);
	bool								Save(char* strFilename, CommandLineApp* app, bool bWriteBankList);
	void								Release();
	void								Enumerate(xml::Node* pNode);
	void								EndNode(xml::TagNode* pTag);
	void								ConvertNodeToAudioData(xml::Node* pNode);
	bool								FoundTag(xml::TagNode* pTag);
	void								FoundTagAttributes(xml::TagNode* pParentNode, const char* pchType, const char* pchAttributes);
	bool								GetFloatValue(const char* pchValue, const F32* pfRetValue);
	bool								GetFloatValue(const char* pchValue, const Audio::AudioFloat* pfRetValue);
	bool								GetIntValue(const char* pchValue, Audio::AudioS32Convertor* ps32RetValue);
	bool								GetIntValue(const char* pchValue, Audio::AudioU16Convertor* pu16RetValue);
	bool								CheckMonoAndIncEffects(ComponentData* pCompData);
	bool								CheckStereoAndIncEffects(ComponentData* pCompData);
	bool								CheckSurroundAndIncEffects(ComponentData* pCompData);
    bool								DoError(const char* pchWarning, int iLine, const char* pchFile, bool bFatal = false);
    bool								DoWarning(const char* pchWarning, int iLine, const char* pchFile);
	void								RegisterAttribute(const char* sName, const char* sValue);
	void								CreateValueListFromAttributeBlock(const char* pchData);
	static Diagnostics&					GetDiagnostics() { return m_Diagnostics; }
	void								SetFrameSkip(U32 u32Skip) { m_uFrameSkip = u32Skip; }

protected:
	bool								GetComponent(const char* strName);
	bool								GetFloatAttribute(const char* strName, Audio::AudioFloat* pFloat);
	bool								GetIntAttribute(const char* strName, Audio::AudioS32Convertor* pInt);
	bool								GetUnsignedIntAttribute(const char* strName, Audio::AudioU16Convertor* pInt);
	bool								GetStringOffsetAttribute(const char* strName, Audio::StrOffset* pInt);
	bool								GetNonComponentStringOffsetAttribute(const char* strName, Audio::StrOffset* pInt);
	bool								GetBankAttribute(Audio::StrOffset* pInt);
	bool								ValidateComponentType(U32 u32ComponentIndex, U32 u32NumTypesToValidateAgainst, va_list* pvl);
	bool								MapComponentNameToIndex(U32* pu32ComponentIndex, const char* strComponentString, const char* strComponentName);
	bool								GetCompAttrShared(const char* strName, U32* pu32ComponentIndex, NamedGroupLevel* pNamedGroup, char* strOutputString = 0);
	bool								GetComponentAttribute(const char* strName, Audio::Com* pCom, U32 u32NumTypesToValidateAgainst = 0, U32 eFirstType = 0, ...);
	bool								GetComponentAttribute(const char* strName, Audio::Com* pCom, va_list* pvl, U32 u32NumTypesToValidateAgainst = 0);
	bool								GetComponentOrFloatAttributes(const char* strName, Audio::Com* pCom, Audio::AudioFloat* pFloat, U32 u32NumTypesToValidateAgainst = 0, U32 eFirstType = 0, ...);
	bool								GetComponentAndOutputAttributes(const char* strName, Audio::Component::Control* pControl);
	bool								GetComponentFloatOutputAttributes(const char* strName, Audio::Component::Option* pOption);
#ifdef NEW_EVENT_CODE
	bool								LookupEvent(const char* strName, Audio::Component::Event& ev);
#endif
	bool								GetValueFromKey(const char* data, const char* pchKey, std::string& strValue);
	void								AddString(char* strString);
	bool								PeekValueFromKey(const char* pchData, const char* pchKey, std::string& strValue);
	U32									GetStringOffset(U32 u32StringIndex);
	U32									GetStringIndex(const char* string);
	U32									GetCurrentStringTableSize();
	ComponentData*						GetParentComponentData(xml::Node* pNode);
	U32									GetComponentDataSize() { return m_u32ComponentDataSize; }
	void								SetComponentData(void* pData, U32 u32Size, U32 u32Type);
	void								AssociateDataWithComponent(ComponentData* pCompData, U32 u32Id, void* pData, U32 u32DataSize);
	void								AllocateComponent(const char* strName, U32 eType, const char* strFile, U32 u32LineNum, NamedGroupLevel* pNamedGroup = 0);
	U32									AddNonComponentString(const char* strName);
	void								IncrementDataSize(U32 u32Inc, U32 u32CompIndex = -1) { m_u32ComponentDataSize += u32Inc; }
	void								AddConstant(char* strName, F32 fValue);
	bool								LookupConstant(char* strName, F32* pfValue);
	U32									GetNonComponentStringOffset(const char* strName);
	U32									GetStringTableOffset() { return m_u32ComponentDataSize + sizeof(AudioDataFileHeader); }
	U16									ResolveOutputFromString(Audio::Com sCom, char* strOutput);

	Audio::Core::Array<ComponentData*>	m_apComponentData;
//	std::string							m_strCurrGroup;
	Audio::Core::Array<std::string*>	m_aStrictParseAttrNames;
	Audio::Core::Array<NonComponentNameStringData*>	m_aNonComponentNameStrings;
	Audio::Core::Array<Constant*>		m_aConstants;
	GroupNameManager					m_GroupNameManager;
	char*								m_strObjectName;
	U32									m_u32CurrComponentIndex;
	U32									m_u32ComponentDataSize;
	U32									m_u32CurrCompNameStringTableSize;
	const char*							m_pchType;
	const char*							m_pchAttributes;
	xml::TagNode*						m_pParentNode;
	std::string							m_strWorkString;
	static Diagnostics					m_Diagnostics;
	char								m_strComponentOutputString[32];
	Audio::AudioS32Convertor			m_uFrameSkip;
	WaveBankList						m_WaveBankList;
};