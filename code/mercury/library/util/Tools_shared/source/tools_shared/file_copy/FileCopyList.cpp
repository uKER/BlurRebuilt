#include "stdafx.h"

#include <tools_shared/PlatformHelpers.h>
#include <tools_shared/file_copy/FileCopyList.h>

using namespace Tools::FileCopy;

C_FileCopyItem::C_FileCopyItem()
:	m_FileCopySource(""),
	m_FileCopyDestination(""),
	m_FileCopyType(E_FileCopyItemType_File)
{
}


C_FileCopyItem::C_FileCopyItem(Util::C_String src, Util::C_String dst, C_FileCopyItem::E_FileCopyItemType type)
:	m_FileCopySource(src),
	m_FileCopyDestination(dst),
	m_FileCopyType(type)
{
}


C_FileCopyItem::~C_FileCopyItem()
{
}


C_FileCopyItem::C_FileCopyItem(const C_FileCopyItem &item)
:
	m_FileCopySource(item.m_FileCopySource),
	m_FileCopyDestination(item.m_FileCopyDestination),
	m_FileCopyType(item.m_FileCopyType)
{
}


const C_FileCopyItem& C_FileCopyItem::operator =(const C_FileCopyItem &item)
{
	m_FileCopySource = item.m_FileCopySource;
	m_FileCopyDestination = item.m_FileCopyDestination;
	m_FileCopyType = item.m_FileCopyType;

	return *this;
}


bool C_FileCopyItem::operator ==(const C_FileCopyItem& item) const
{
	return
		((m_FileCopySource == item.m_FileCopySource) &&
		 (m_FileCopyDestination == item.m_FileCopyDestination) &&
		 (m_FileCopyType == item.m_FileCopyType));
}


bool C_FileCopyItem::operator !=(const C_FileCopyItem& item) const
{
	return !((*this) == item);
}




C_FileCopyOption::C_FileCopyOption()
:	m_Option(""),
	m_Value("")
{
}


C_FileCopyOption::C_FileCopyOption(Util::C_String option, Util::C_String value)
:	m_Option(option),
	m_Value(value)
{
}


C_FileCopyOption::C_FileCopyOption(const C_FileCopyOption &option)
:	m_Option(option.m_Option),
	m_Value(option.m_Value)
{
}


const C_FileCopyOption& C_FileCopyOption::operator =(const C_FileCopyOption &option)
{
	m_Option = option.m_Option;
	m_Value = option.m_Value;

	return *this;
}


bool C_FileCopyOption::operator ==(const C_FileCopyOption &option) const
{
	return((m_Option.Lower() == option.m_Option.Lower()) && m_Value.Lower() == option.m_Value.Lower());
}


bool C_FileCopyOption::operator !=(const C_FileCopyOption &option) const
{
		return !((*this) == option);
}






C_FileCopyList::C_FileCopyList()
{
	m_FileCopyItems.Resize(Util::E_Platform_Count);
	m_FileCopyOptions.Resize(Util::E_Platform_Count);
}


C_FileCopyList::C_FileCopyList(const C_FileCopyList &list)
{
	m_FileCopyItems = list.m_FileCopyItems;
	m_FileCopyOptions = list.m_FileCopyOptions;
}


C_FileCopyList::~C_FileCopyList()
{
}


FileCopyItemArray& C_FileCopyList::GetFileCopyItemArray(Util::E_Platform platform)
{
	return m_FileCopyItems[platform];
}

const FileCopyItemArray& C_FileCopyList::GetFileCopyItemArray(Util::E_Platform platform) const
{
	return m_FileCopyItems[platform];
}

U32 C_FileCopyList::AddFileCopyItem(Util::E_Platform platform, C_FileCopyItem &item)
{
	m_FileCopyItems[platform].PushBack(item);
	return (U32)m_FileCopyItems[platform].Size()-1; 
}


C_FileCopyItem& C_FileCopyList::GetFileCopyItem(Util::E_Platform platform, U32 instance)
{
	return m_FileCopyItems[platform][instance];
}


const C_FileCopyItem& C_FileCopyList::GetFileCopyItem(Util::E_Platform platform, U32 instance) const
{
	return m_FileCopyItems[platform][instance];
}


void C_FileCopyList::SetFileCopyItem(Util::E_Platform platform, U32 instance, C_FileCopyItem &item)
{
	m_FileCopyItems[platform][instance] = item;
}


U32 C_FileCopyList::GetNumFileCopyItems(Util::E_Platform platform)
{
	return (U32)m_FileCopyItems[platform].Size()-1; 
}


U32 C_FileCopyList::GetNumFileCopyItems(Util::E_Platform platform) const
{
	return (U32)m_FileCopyItems[platform].Size()-1; 
}


FileCopyOptionArray& C_FileCopyList::GetFileCopyOptionArray(Util::E_Platform platform)
{
	return m_FileCopyOptions[platform];
}


const FileCopyOptionArray& C_FileCopyList::GetFileCopyOptionArray(Util::E_Platform platform) const
{
	return m_FileCopyOptions[platform];
}


U32 C_FileCopyList::AddFileCopyOption(Util::E_Platform platform, const C_FileCopyOption &option)
{
	m_FileCopyOptions[platform].PushBack(option);
	return (U32)m_FileCopyOptions[platform].Size()-1;
}


C_FileCopyOption& C_FileCopyList::GetFileCopyOption(Util::E_Platform platform, U32 instance)
{
	return m_FileCopyOptions[platform][instance];
}


const C_FileCopyOption& C_FileCopyList::GetFileCopyOption(Util::E_Platform platform, U32 instance) const
{
	return m_FileCopyOptions[platform][instance];
}


C_FileCopyOption* C_FileCopyList::FindFileCopyOptionByName(Util::E_Platform platform, const char *name)
{
	FileCopyOptionArray& platformOptions = m_FileCopyOptions[platform];

	for (S32 i=0; i<platformOptions.Size(); ++i)
	{
		if (platformOptions[i].m_Option == name)
			return &platformOptions[i];
	}
	return 0;
}


const C_FileCopyOption* C_FileCopyList::FindFileCopyOptionByName(Util::E_Platform platform, const char *name) const
{
	const FileCopyOptionArray& platformOptions = m_FileCopyOptions[platform];

	for (S32 i=0; i<platformOptions.Size(); ++i)
	{
		if (platformOptions[i].m_Option == name)
			return &platformOptions[i];
	}
	return 0;
}


void C_FileCopyList::SetFileCopyOption(Util::E_Platform platform, U32 instance, const C_FileCopyOption &option)
{
	m_FileCopyOptions[platform][instance] = option;
}


U32 C_FileCopyList::GetNumFileCopyOptions(Util::E_Platform platform)
{
	return m_FileCopyOptions[platform].Size()-1;
}


U32 C_FileCopyList::GetNumFileCopyOptions(Util::E_Platform platform) const
{
	return m_FileCopyOptions[platform].Size()-1;
}

void C_FileCopyList::ClearItems()
{
	for (U32 i = 0; i < (U32)m_FileCopyItems.Size(); i++)
	{
		m_FileCopyItems[i].Clear();
	}
}
