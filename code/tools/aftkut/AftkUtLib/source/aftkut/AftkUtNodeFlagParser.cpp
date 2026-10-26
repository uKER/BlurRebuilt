#include "stdafx.h"
#include <core/utility/StringProc.h>
#include <core/containers/String.h>
using namespace AftkUt;

// The new string format -actorString3 (which all previous formats are being converted into) 
// is <param type = "" name = "undefined" value = "undefined" values = "undefined" />. Note - values will only be present when the type is "enum"

std::string C_NodeFlagParser::m_default = "<actor name = \"undefined\" category = \"undefined\"> </actor>";

char* C_ExtractedPropString::m_ppParamTypes[10] = { "int", "bool", "float", "enum", "string", "link", "colour", "unknown", "\0" };



void C_ExtractedPropString::Reset() const
{
	m_cursor = 0;
}

/// default constructor

C_ExtractedPropString::C_ExtractedPropString()
{
}

/// construct with input string
C_ExtractedPropString::C_ExtractedPropString(const std::string inputString) 
{
	m_input = inputString;
}


void C_ExtractedPropString::SetString(const std::string inputString)
{
	m_input = inputString;
}

/// Find a string from the current pos
bool C_ExtractedPropString::Find(const std::string &str) const
{
	m_cursor = m_input.find(str, m_cursor);
	return m_cursor != std::string::npos;
}


/// Find first occurence of any character in the input string, starting from the current pos
int C_ExtractedPropString::FindFirstOf(const std::string &str) const
{
	m_cursor = m_input.find_first_of(str, m_cursor);
	return m_cursor != std::string::npos;
}

bool C_ExtractedPropString::GetParameterValue(const std::string& name, S32& value) const
{
	std::string tmp;
	E_ParamType type;
	if (GetParameterValue(name, tmp, &type) && (type == E_ParamType_S32 || type == E_ParamType_Unknown))
	{
		value = atoi(tmp.c_str());
		return true;
	}

	return false;
}

bool C_ExtractedPropString::GetParameterValue(const std::string& name, bool& value) const
{
	std::string tmp;
	E_ParamType type;
	if (GetParameterValue(name, tmp, &type) && 
		(type == E_ParamType_Bool || type == E_ParamType_Unknown))
	{
		if (tmp == "1" || !stricmp("true", tmp.c_str()))
			value = true;
		else
			value = false;
		return true;
	}

	return false;
}

bool C_ExtractedPropString::GetParameterValue(const std::string& name, F32& value) const
{
	std::string tmp;
	E_ParamType type;
	if (GetParameterValue(name, tmp, &type) && (type == E_ParamType_F32 || type == E_ParamType_Unknown))
	{
		value = (F32)atof(tmp.c_str());
		return true;
	}
	return false;
}

bool C_ExtractedPropString::GetParameterValue(
	const std::string& name, 
	F32& rValue, F32& gValue, F32& bValue, F32& aValue) const
{
	// Maya strings will have separate parameters for alpha values therefore the last float will not be there and will be set to 1.0 
	std::string tmp;
	E_ParamType type;
	if (GetParameterValue(name, tmp, &type) && (type == E_ParamType_Colour || type == E_ParamType_Unknown))
	{
		Util::C_String combinedString(tmp.c_str());
		Ut::Vector<Util::C_String> tokenizedStrings;
		combinedString.Tokenise(tokenizedStrings, ' ');
		rValue = (F32)atof( (tokenizedStrings[0]).Chars());
		gValue = (F32)atof( (tokenizedStrings[1]).Chars());
		bValue = (F32)atof( (tokenizedStrings[2]).Chars());

		if (tokenizedStrings.Size() == 4)
		{
			aValue = (F32)atof( (tokenizedStrings[3]).Chars() );
			return true;
		}
		else if (tokenizedStrings.Size() == 3)
		{
			aValue = 1.0f;
			return true;
		}
	}

	return false;
}

bool C_ExtractedPropString::GetParameterValue(
	const std::string& name, std::string& value, E_ParamType* pType) const
{
	Reset();

	std::string search = "name = \"" + name + "\"";

	std::string valuePrefix = "value = \"";

	if (Find(search))
	{
		//Find type
		if (pType)
		{
			if (m_cursor < 2)
				return false;
			size_t typeEnd = m_cursor-2;
			const char* typeSearch = "type = \"";
			size_t typeStart = m_input.rfind(typeSearch, typeEnd)+strlen(typeSearch);
			if (typeStart == std::string::npos)
				return false;

			std::string type = m_input.substr(typeStart, typeEnd-typeStart);
			*pType = GetType(type.c_str());
		}

		m_cursor += search.length();
		if(!Find(valuePrefix))
			return false;

		m_cursor += valuePrefix.length();
		size_t start = m_cursor;

		if(!FindFirstOf("\""))
		{
			return false;
		}

		value = m_input.substr(start, m_cursor - start);

		return true;
	}

	return false;
}

E_ParamType C_ExtractedPropString::GetType(const char* str) const
{
	for (U32 i = 0; i < E_ParamType_Count; ++i)
	{
		if (!strcmp(m_ppParamTypes[i], str))
			return E_ParamType(i);
	}

	return E_ParamType_Unknown;
}

const char* C_ExtractedPropString::GetTypeName(E_ParamType type) const
{
	return m_ppParamTypes[type];
}




void C_NodeFlagParser::Initialise()
{
	m_cursor = 0;
	m_input = m_default;
	m_pNode = 0;
	m_bStringModified = false;
}

/// default constructor
C_NodeFlagParser::C_NodeFlagParser()
: m_readOnly(false)
{
	Initialise();
}

C_NodeFlagParser::C_NodeFlagParser(const std::string &input)
{
	Initialise();
	m_input = input;
	m_readOnly = false;
}

/// construct with input string
C_NodeFlagParser::C_NodeFlagParser(
	const std::string &input, 
	E_Version version,
	bool* pStatus,
	bool queryBaseInstance)
{
	bool status;
	Initialise();
	m_input = input;
	m_readOnly = false;

	if (version == E_Version_1)
	{
		status = ConvertFlags(E_Version_1);
	}
	else if (version == E_Version_2)
	{
		status = ConvertFlags(E_Version_2);
	}
	else
	{
		status = true;
	}

	if (pStatus)
	{
		*pStatus = status;
	}
}

C_NodeFlagParser::C_NodeFlagParser(
	const Aftk::C_SceneNode& node,
	bool* pStatus,
	bool queryBaseInstance)
: m_readOnly(true)
{
	bool status;

	Initialise();
	if (GetFlagString(node, queryBaseInstance))
	{
		status = true;
	}
	else
	{
		status = false;
	}

	m_pNode = const_cast<Aftk::C_SceneNode*>(&node);
	if (pStatus)
		*pStatus = status;
}

C_NodeFlagParser::C_NodeFlagParser(
	Aftk::C_SceneNode& node,
	bool* pStatus, 
	bool queryBaseInstance)
: m_readOnly(false)
{
	bool status;

	Initialise();
	if (GetFlagString(node, queryBaseInstance))
	{
		status = true;
	}
	else
	{
		status = false;
	}

	m_pNode = const_cast<Aftk::C_SceneNode*>(&node);

	if (pStatus)
		*pStatus = status;
}

/// destructor
C_NodeFlagParser::~C_NodeFlagParser()
{
	if (m_pNode && m_bStringModified)
		StoreFlags(*m_pNode);
}

/// add an attribute to a node
void C_NodeFlagParser::AddLegacyAttribute(Aftk::C_SceneNode& node, Aftk::C_GenericAttribute& attribute)
{
	if (node.GetBlindDataResourcePtr() == 0)
	{
		Aftk::C_BlindDataNode bd;
		bd.AddAttribute(attribute);
		node.SetBlindDataResourcePtr(node.GetScenePtr()->AddBlindDataNode(bd));
	}
	else
	{
		Aftk::C_BlindDataNode bd(node.GetBlindDataResourcePtr()->GetContent());
		bd.AddAttribute(attribute);
		node.SetBlindDataResourcePtr(node.GetScenePtr()->AddBlindDataNode(bd));
	}
}

bool C_NodeFlagParser::IsBlindDataFlagged(const Aftk::C_SceneNode& node)
{
	//Look for blind-data if none was found on a dynamic plug.
	const Aftk::C_SharedResource<Aftk::C_BlindDataNode>* pBlindDataResource =
		node.GetBlindDataResourcePtr();
	if (pBlindDataResource)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = 
			pBlindDataResource->GetContent();

		for (U32 i = 0; i < aftkBlindDataNode.GetNumAttributes(); i++) 
		{
			const Aftk::C_GenericAttribute&	aftkAttribute = 
				aftkBlindDataNode.GetAttribute(i);

			if (aftkAttribute.GetName() == "actorString" || aftkAttribute.GetName() == "actorString2" || aftkAttribute.GetName() == "actorString3") 
				return true;
		}  
	}

	return false;
}

bool C_NodeFlagParser::GetFlagString(
	std::string& flagString, 
	const Aftk::C_SceneNode& node,
	bool* pReadOnly,
	bool queryBaseInstance)
{
	if (GetThisFlagString(
		flagString,
		const_cast<Aftk::C_SceneNode&>(node)))
	{
		if (pReadOnly)
			*pReadOnly = false;

		return true;
	}

	if (queryBaseInstance)
	{
		const Aftk::C_SceneNode* pSourceNode = GetInstanceSourceNodePtr(node);
		if (pSourceNode)
		{
			if (pReadOnly)
				*pReadOnly = true;

			if (GetFlagString(
				flagString, 
				*pSourceNode,
				0,
				queryBaseInstance))
				return true;
		}
	}

	flagString.clear();
	return false;
}

/// retrieve the flag string off an aftk node
bool C_NodeFlagParser::GetThisFlagString(
	std::string& flagString, 
	Aftk::C_SceneNode& node)
{
	E_Version version = E_Version_Unknown;

	Aftk::C_DynamicPlug* pFlagPlug;
	pFlagPlug = (Aftk::C_DynamicPlug*)node.GetPlugPtr("actorString3");
	if (pFlagPlug)
	{
		version = E_Version_3;
		pFlagPlug->GetValue(flagString);
		return true;
	}

	pFlagPlug = (Aftk::C_DynamicPlug*)node.GetPlugPtr("actorString2");
	if (pFlagPlug)
	{
		version = E_Version_2;
		pFlagPlug->GetValue(flagString);
		ConvertFlag2String(flagString);
		return true;
	}

	pFlagPlug = (Aftk::C_DynamicPlug*)node.GetPlugPtr("actorString");
	if (pFlagPlug)
	{
		version = E_Version_1;
		pFlagPlug->GetValue(flagString);
		ConvertFlag1String(flagString);
		return true;
	}

	//Look for blind-data flags if none was found on a dynamic plug.
	const Aftk::C_SharedResource<Aftk::C_BlindDataNode>* pBlindDataResource =
		node.GetBlindDataResourcePtr();
	if (pBlindDataResource && flagString.length() == 0)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = 
			pBlindDataResource->GetContent();

		for (U32 i = 0; i < aftkBlindDataNode.GetNumAttributes(); i++) 
		{
			const Aftk::C_GenericAttribute&	aftkAttribute = 
				aftkBlindDataNode.GetAttribute(i);

			if (aftkAttribute.GetName() == "actorString")
			{
				version = E_Version_1;
				aftkAttribute.GetValue(flagString);
				ConvertFlag1String(flagString);
				return true;
			}
			else if (aftkAttribute.GetName() == "actorString2")
			{
				version = E_Version_2;
				aftkAttribute.GetValue(flagString);
				ConvertFlag2String(flagString);
				return true;
			}
			else if (aftkAttribute.GetName() == "actorString3")
			{
				version = E_Version_3;
				aftkAttribute.GetValue(flagString);
				return true;
			}
		}  
	}

	return false;
}

bool C_NodeFlagParser::IsFlagged(const Aftk::C_SceneNode& node, bool queryBaseInstance)
{
	if (IsThisFlagged(node))
		return true;

	if (queryBaseInstance)
	{
		//Recurse, looking for flags.
		const Aftk::C_SceneNode* pSource = GetInstanceSourceNodePtr(node);
		if (pSource)
			return IsFlagged(*pSource, true);
	}
	return false;
}

bool C_NodeFlagParser::IsFlaggedBaseInstance(const Aftk::C_SceneNode& node)
{
	return IsThisFlagged(node);
}

bool C_NodeFlagParser::IsFlaggedInstance(const Aftk::C_SceneNode& node)
{
	if (!IsThisFlagged(node) && IsFlagged(node))
		return true;

	return false;
}

bool C_NodeFlagParser::IsThisFlagged(const Aftk::C_SceneNode& node)
{
	if (IsBlindDataFlagged(node) || node.GetPlugPtr("actorString") || node.GetPlugPtr("actorString2") || node.GetPlugPtr("actorString3"))
		return true;

	return false;
}

/// set the flagged string on an aftk node
bool C_NodeFlagParser::SetFlagString(Aftk::C_SceneNode& node, const std::string &string)
{
	{
		Aftk::C_DynamicPlug* pFlagPlug = (Aftk::C_DynamicPlug*)node.GetPlugPtr("actorString");
		if (pFlagPlug)
			node.DeletePlug(pFlagPlug->GetLogicalIndex());
	}
	{
		Aftk::C_DynamicPlug* pFlagPlug = (Aftk::C_DynamicPlug*)node.GetPlugPtr("actorString2");
		if (pFlagPlug)
			node.DeletePlug(pFlagPlug->GetLogicalIndex());
	}

	Aftk::C_DynamicPlug* pFlagPlug = (Aftk::C_DynamicPlug*)node.GetPlugPtr("actorString3");
	if (!pFlagPlug)
	{
		//Add a new flag plug if needed.
		pFlagPlug = new Aftk::C_DynamicPlug;
		node.AddPlugPtr(pFlagPlug);
		pFlagPlug->SetName("actorString3");
	}

	pFlagPlug->SetValue(string);
	return true;
}

/// delete the flag string off an aftk node
void C_NodeFlagParser::DeleteFlags(Aftk::C_SceneNode& node)
{
	if (!IsThisFlagged(node))
		return;

	//Delete legacy flag string.
	if (const Aftk::C_SharedResource<Aftk::C_BlindDataNode>* pBlindDataResource = 
		node.GetBlindDataResourcePtr())
	{
		Aftk::C_BlindDataNode aftkBlindDataNode = 
			pBlindDataResource->GetContent();

		for (U32 i = 0; i < aftkBlindDataNode.GetNumAttributes(); i++) 
		{
			const Aftk::C_GenericAttribute&	aftkAttribute = aftkBlindDataNode.GetAttribute(i);

			if (aftkAttribute.GetName() == "actorString") 
				aftkBlindDataNode.DeleteAttribute(i);
		}

		node.SetBlindDataResourcePtr(node.GetScenePtr()->AddBlindDataNode(aftkBlindDataNode));
	}

	//Delete any modern flag string.
	Aftk::I_Plug* pFlagPlug = node.GetPlugPtr("actorString");
	if (pFlagPlug)
		node.DeletePlug(pFlagPlug->GetLogicalIndex());
}

/// Clears all flags from node and empties flag string
void C_NodeFlagParser::ClearAllFlags(Aftk::C_SceneNode& node)
{
	DeleteFlags(node);

	Aftk::I_Plug* pFlagPlug = node.GetPlugPtr("actorString3");
	if (pFlagPlug)
		node.DeletePlug(pFlagPlug->GetLogicalIndex());

	//SetFlagString(node, std::string(""));
}

/// set flag string to a node
void C_NodeFlagParser::StoreFlags(Aftk::C_SceneNode& node)
{
	SetFlagString(node, m_input);
}
	
/// get flag string from a node
bool C_NodeFlagParser::GetFlagString(
	const Aftk::C_SceneNode& node, bool queryBaseInstance)
{
	bool readOnly;
	std::string flagString;
	if (GetFlagString(flagString, node, &readOnly, queryBaseInstance))
	{
		m_readOnly = readOnly;
		SetFlagString(flagString);
		return true;
	}

	return false;
}

/// set the flag XML string
void C_NodeFlagParser::SetFlagString(const std::string &input)
{
	m_input = input;
	m_bStringModified = false;
}

/// retrieve the flag string
bool C_NodeFlagParser::GetFlagString(
	std::string& flagString, 
	bool* pReadOnly)
{
	flagString = m_input;
	if (pReadOnly)
		*pReadOnly = m_readOnly;

	if (flagString.length() > 0)
		return true;
	return false;
}

std::string C_NodeFlagParser::GetFlagGroupName() const
{
	Reset();
	return GetValueForNameValuePair("name", E_Version_3);
}

std::string C_NodeFlagParser::GetFlagGroupCategory() const
{
	Reset();
	return GetValueForNameValuePair("category", E_Version_3);
}

bool C_NodeFlagParser::SetFlagGroupName(const std::string &name)
{
	Reset();

	if (m_input == "")
		m_input = m_default;

	if (Find("name"))
	{
		if(FindFirstOf("\""))
		{
			m_cursor++;
			size_t start = m_cursor;
			if(FindFirstOf("\""))
			{
				m_input = m_input.substr(0, start) + name + m_input.substr(m_cursor, m_input.length());
				m_bStringModified = true;
				return true;
			}
		}
	}
	return false;
}

///Set flag category
bool C_NodeFlagParser::SetFlagGroupCategory(const std::string& name)
{
	Reset();

	if (m_input == "")
		m_input = m_default;

	if (Find("category"))
	{
		if(FindFirstOf("\""))
		{
			m_cursor++;
			size_t start = m_cursor;
			if(FindFirstOf("\""))
			{
				m_input = m_input.substr(0, start) + name + m_input.substr(m_cursor, m_input.length());
				m_bStringModified = true;
				return true;
			}
		}
	}
	return false;
}

bool C_NodeFlagParser::DeleteParameter(const std::string& name)
{
	for (U32 i = 0; true; i++)
	{
		char* pParamType = m_ppParamTypes[i];
		if (pParamType[0] == '\0')
			break;
		if (DeleteParameter(pParamType, name))
			return true;
	}
	return false;
}

/// Delete a parameter by name
bool C_NodeFlagParser::DeleteParameter(const std::string& type, const std::string& name)
{
	Reset();

	std::string search = "<param type = \"" + type + "\" name = \"" + name + "\"";

	// find the start of the parameter
	if (!Find(search))
		return false;

	size_t start = m_cursor;

	// find the end of the xml tag
	if (!Find("/>"))
		return false;

	// remove the tag from our xml string
	m_input = m_input.substr(0, start) + m_input.substr(m_cursor+2, m_input.length());

	m_bStringModified = true;
	return true;
}

bool C_NodeFlagParser::SetParameterValue(
	const std::string& name, 
	S32 value)
{
	char buf[128];
	sprintf(buf, "%i", value); 
	return SetParameterValue(name, buf, E_ParamType_S32);
}

bool C_NodeFlagParser::SetParameterValue(
	const std::string& name, 
	bool value)
{
	return SetParameterValue(name, value ? "1" : "0", E_ParamType_Bool);
}

bool C_NodeFlagParser::SetParameterValue(
	const std::string& name, 
	F32 value)
{
	char buf[128];
	sprintf(buf, "%f", value); 
	return SetParameterValue(name, buf, E_ParamType_F32);
}

bool C_NodeFlagParser::SetParameterValue(
	const std::string& name, 
	F32 rValue, 
	F32 gValue, 
	F32 bValue, 
	F32 aValue)
{
	char buf[128];
	sprintf(buf, "%f %f %f %f", rValue, gValue, bValue, aValue); 
	return SetParameterValue(name, buf, E_ParamType_Colour);
}

/// Add a parameter name + value pair
bool C_NodeFlagParser::SetParameterValue(const std::string& name, const std::string &value, E_ParamType type)
{
	//Invalid param type
	if (type >= E_ParamType_Count)
		return false;

	if (m_input == "")
		m_input = m_default;
	else
		DeleteParameter(name);

	Reset();

	if (!Find("<actor "))
		return false;

	if (!Find(">"))
		return false;

	// insert the new tag here
	m_cursor++;
	m_input = m_input.substr(0, m_cursor) + " <param type = \"" + std::string(m_ppParamTypes[type]) + "\" name = \"" + name + "\" value = \"" + value + "\" />" + m_input.substr(m_cursor, m_input.length());

	m_bStringModified = true;
	return true;
}

std::string C_NodeFlagParser::GetValueForNameValuePair(const std::string& name, E_Version version) const
{
	std::string delim;
	if (version == E_Version_1)
		delim = "'";
	else
		delim = "\"";

	if(Find(name))
	{
		if(FindFirstOf(delim))
		{
			m_cursor++;
			size_t start = m_cursor;
			if(FindFirstOf(delim))
			{
				return m_input.substr(start, m_cursor - start);
			}
		}
	}
	return "";
}


std::string C_NodeFlagParser::GetTypeForFlag2(std::string& newParam) const
{
	// Will be the first word after "<"

	m_cursor++;
	size_t start = m_cursor;

	std::string delim;

	delim = " ";
	
	if (FindFirstOf(delim))
	{
		return m_input.substr(start, m_cursor-start);
	}

	return "";
}


bool C_NodeFlagParser::ConvertFlagParam1(std::string& newParam) const
{
	//Find the end of the param section.

	std::string::size_type endPos = m_input.find("/>", m_cursor);

	//Get vars common to all param types.

	std::string name  = GetValueForNameValuePair("name", E_Version_1);
	std::string value = GetValueForNameValuePair("value", E_Version_1);
	std::string type  = GetValueForNameValuePair("type", E_Version_1);

	if (type.length() == 0)
	{
		//No type specified
		type = "unknown";
	}

	//Begin param

	newParam = "<param type = \"";

	//Specify type

	newParam += type.c_str();
	newParam += "\" ";

	//Add name

	newParam += "name = \"" + name + "\" ";

	//Add default value

	newParam += "value = \"" + value + "\"";

	// Get enum values if an enum type

	if (!stricmp(type.c_str(), "enum") )
	{
		std::string enumValues = GetValueForNameValuePair("values", E_Version_1);

		if (enumValues.length() > 0)
		{
			newParam += " values = \"" + enumValues + "\"";
		}
	}

	//End param

	newParam += " />";

	return true;
}


bool C_NodeFlagParser::ConvertFlagParam2(std::string& newParam) const
{
	//Find the end of the param section.

	std::string::size_type endPos = m_input.find("/>", m_cursor);

	//Get vars common to all param types.

	std::string type = GetTypeForFlag2(newParam);

	if (type.length() == 0)
	{
		//No type specified - assume string.
		type = "string";
	}

	std::string name = GetValueForNameValuePair("name", E_Version_2);

	// Get enum values if an enum type

	std::string enumValues;

	if (!stricmp(type.c_str(), "enum") )
	{
		enumValues = GetValueForNameValuePair("values", E_Version_2);
	}

	std::string value = GetValueForNameValuePair("default", E_Version_2);

	//Begin param

	newParam = "<param type = \"";

	//Specify type

	newParam += type.c_str();
	newParam += "\" ";

	//Add name

	newParam += "name = \"" + name + "\" ";

	//Add default value

	newParam += "value = \"" + value + "\"";

	//Add values (if enum)

	if (!stricmp(type.c_str(), "enum") )
	{
		if (enumValues.length() > 0)
		{
			newParam += " values = \"" + enumValues + "\"";
		}
	}

	//End param

	newParam += " />";

	return true;
}


bool C_NodeFlagParser::ConvertFlag1String(std::string& flagString)
{
	bool status;
	C_NodeFlagParser parser(flagString, E_Version_1, &status, false);
	if (!status)
		return false;

	parser.GetFlagString(flagString);
	return true;
}


bool C_NodeFlagParser::ConvertFlag2String(std::string& flagString)
{
	bool status;
	C_NodeFlagParser parser(flagString, E_Version_2, &status, false);
	if (!status)
		return false;

	parser.GetFlagString(flagString);
	return true;
}



bool C_NodeFlagParser::ConvertFlags(E_Version version)
{
	if (version == E_Version_1)
	{
		Tu::SearchAndReplace(m_input, "class", "category");
	}

	//Reset cursor

	Reset();

	int Count = 0;

	while(true)
	{
		Find("<");

		if (Count)
		{
			if (m_cursor == std::string::npos)
			{
				//Break if nothing more to convert.
				break;
			}

			std::string::size_type start = m_cursor;
			std::string::size_type end = m_input.find("/>", start);

			if (end == std::string::npos)
			{
				break;
			}

			end += 2;

			std::string newParam;

			if (version == E_Version_1)
			{
				if (!ConvertFlagParam1(newParam))
				{
					return false;
				}
			}
			else if (version == E_Version_2)
			{
				if (!ConvertFlagParam2(newParam))
				{
					return false;
				}
			}

			m_input = Tu::ReplaceSection(m_input, start, end, newParam);
			m_cursor = end;
		}
		else	// Just skip the first set of brackets
		{
			std::string::size_type start = m_cursor;
			m_cursor = m_input.find(">", start);
		}

		Count ++;
	}

	if (version == E_Version_1)
	{
		Tu::SearchAndReplace(m_input, " = '", " = \"");
		Tu::SearchAndReplace(m_input, "' ", "\" ");
	}
	
	return true;
}

Aftk::C_SceneNode* C_NodeFlagParser::GetInstanceSourceNodePtr(Aftk::C_SceneNode& node)
{
	switch(node.GetType())
	{
		default:
		break;
		
		case Aftk::E_ObjType_SceneInstance:
		{
			Aftk::C_SceneInstanceNode* pInstanceNode = (Aftk::C_SceneInstanceNode*)&node;
			return pInstanceNode->GetSourceNodePtr();
		}
		break; 

		case Aftk::E_ObjType_SceneMeshRemappedInstance:
		{
			Aftk::C_SceneMeshRemappedInstanceNode* pInstanceNode = (Aftk::C_SceneMeshRemappedInstanceNode*)&node;
			return pInstanceNode->GetSourceNodePtr();
		}
		break; 
	}
	
	return 0;
}

const Aftk::C_SceneNode* C_NodeFlagParser::GetInstanceSourceNodePtr(const Aftk::C_SceneNode& node)
{
	return GetInstanceSourceNodePtr(const_cast<Aftk::C_SceneNode&>(node));
}

