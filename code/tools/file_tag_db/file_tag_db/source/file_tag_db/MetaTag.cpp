#include "stdafx.h"
#include <file_tag_db/GenericAttribute.h>
#include <file_tag_db/Tag.h>
#include <file_tag_db/MetaTag.h>
#include <sstream>

namespace FileTag
{
	const char* BUILD_TIME = "Build Time";
	const char* VERSION = "Version";
}

using namespace FileTag;

C_MetaTag::C_MetaTag()
: m_pTag(0)
{
}

C_MetaTag::C_MetaTag(C_Tag* pTag)
: m_pTag(pTag)
{
}

// Returns the build time of the current tag.
S64 C_MetaTag::GetBuildTime() const
{
	assert(m_pTag);
	if (!m_pTag)
		return 0;

	S64 value;

	// Find and return the version attribute.
	const C_GenericAttribute* attr = m_pTag->GetAttributePtr(BUILD_TIME);

	if( attr && attr->GetName() == BUILD_TIME )
	{	
		attr->GetValue(value);
		return value;
	}

	return 0;
}

bool C_MetaTag::GetInputs(Ut::Vector<C_Input>& inputs) const
{
	assert(m_pTag);
	for (S32 i = 0; i < m_pTag->GetAttributes().Size(); ++i)
	{
		FileTag::C_GenericAttribute& attr = m_pTag->GetAttributes()[i];
		if (attr.GetDataType() == FileTag::C_GenericAttribute::E_DataType_String &&
			attr.GetName() == "Input")
		{
			Util::C_String toTokenise;
			attr.GetValue(toTokenise);
			Ut::Vector<Util::C_String> inputAndDate;
			toTokenise.Tokenise(inputAndDate, ';');
			if (inputAndDate.Size() != 2)
				//Malformed meta-data.
				return false;

			inputs.PushBack();
			C_Input& input = inputs.Back();
			input.file = inputAndDate[0];

			//Convert date to S64
			std::stringstream ss;
			ss << inputAndDate[1];
			ss >> input.fileTime;
		}
	}

	return true;
}

const C_Tag* C_MetaTag::GetTagPtr() const
{
	return m_pTag;
}

C_Tag* C_MetaTag::GetTagPtr()
{
	return m_pTag;
}

// Set the build time to be written to file.
void C_MetaTag::SetBuildTime(S64 buildTime)
{
	assert(m_pTag);
	if (!m_pTag)
		return;

	// Add new build stamps or updata existing ones.
	if (!m_pTag->GetAttributePtr(BUILD_TIME))
	{
		C_GenericAttribute newAtt;
		newAtt.GetName() = BUILD_TIME;
		newAtt.SetValue(buildTime);
		m_pTag->GetAttributes().PushBack(newAtt);
	}
	else
	{
		// Update existing stamps.
		m_pTag->GetAttributePtr(BUILD_TIME)->SetValue(buildTime);
	}
}

void C_MetaTag::SetInputs(const Ut::Vector<C_Input>& inputs)
{
	assert(m_pTag);
	//Remove existing inputs
	{
		Ut::Vector<C_GenericAttribute> newAttrs;
		newAttrs.Reserve(m_pTag->GetAttributes().Size());
		for (S32 i = 0; i < m_pTag->GetAttributes().Size(); ++i)
		{
			C_GenericAttribute& attr = m_pTag->GetAttributes()[i];
			if (attr.GetName() != "Input")
				newAttrs.PushBack(attr);
		}
		m_pTag->GetAttributes() = newAttrs;
	}

	for (S32 i = 0; i < inputs.Size(); ++i)
	{
		const C_Input& input = inputs[i];

		m_pTag->GetAttributes().PushBack();
		C_GenericAttribute& attr = m_pTag->GetAttributes().Back();
		attr.GetName() = "Input";
		Util::C_String fileAndDate;
		//Convert date to string.
		std::stringstream ss;
		ss << input.fileTime;
		fileAndDate.Printf("%s;%s", input.file.Chars(), ss.str().c_str());
		attr.SetValue(fileAndDate, C_GenericAttribute::E_DataType_String);
	}
}

C_MetaTag::C_MetaTag(const C_MetaTag& rhs)
{
	*this = rhs;
}

const C_MetaTag& C_MetaTag::operator=(const C_MetaTag& rhs)
{
	m_pTag = rhs.m_pTag;
	return *this;
}