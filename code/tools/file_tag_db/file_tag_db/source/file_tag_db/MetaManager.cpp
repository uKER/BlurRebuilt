#include "stdafx.h"
#include <file_tag_db/GenericAttribute.h>
#include <file_tag_db/Tag.h>
#include <file_tag_db/TagDB.h>
#include <file_tag_db/TagDBManager.h>
#include <file_tag_db/MetaTag.h>
#include <file_tag_db/MetaManager.h>
#include <system/misc/StringUtil.h>

using namespace FileTag;

C_MetaManager::~C_MetaManager()
{
}

bool C_MetaManager::AddTag(C_MetaTag& metaTag, const Util::C_String &filePath)
{
	Util::C_String lcase = Util::FormatUnixFileString(filePath).Lower();
	C_Tag* pTag = C_TagDBManager::Instance().AddTag(lcase);
	if (pTag)
	{
		metaTag = C_MetaTag(pTag);
		return true;
	}
	return false;
}

bool C_MetaManager::GetTag(C_MetaTag& metaTag, const Util::C_String &filePath)
{
	Util::C_String lcase = Util::FormatUnixFileString(filePath).Lower();
	C_Tag* pTag = C_TagDBManager::Instance().GetTagPtr(lcase);
	if (pTag)
	{
		metaTag = C_MetaTag(pTag);
		return true;
	}
	return false;
}

bool C_MetaManager::Write()
{
	return C_TagDBManager::Instance().Write();
}

void C_MetaManager::Clear()
{
	C_TagDBManager::Instance().Clear();
	C_TagDBManager::Instance().DirtyQueryCache();
}

