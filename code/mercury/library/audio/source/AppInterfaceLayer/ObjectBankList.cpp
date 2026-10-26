#include "AudioPch.h"

#include "ObjectBankList.h"

#include <core/BizAssert.h>
#include <System/memory/MemoryTracker.h>
#include <System/Serialise/I_FileSystem.h>
#include <system/misc/BtlStringUtil.h>

#include "StringHash.h"
//#include "XtAudio.h"
//#include <Xt/XtGame.h>

Audio::C_ObjectBankList::C_ObjectBankList()
{
}

Audio::C_ObjectBankList::~C_ObjectBankList()
{
	Clear();
}

bool Audio::C_ObjectBankList::Load(const Sys::C_FilePath& strPath)
{
	MEMORY_TRACKER_PUSH("Audio.BankLists");

	Sys::I_FileSystem& fs = System::Get()->GetFileSystem();

	AUDIO_SYS_DEBUG("Loading bank list file ...", strPath.Chars());

	Sys::C_FilePath fullPath = Sys::ConstructFilePath(strPath, "DatBankLists", "wbl");

	bool bOk = LoadBankList(fullPath);
	if (!bOk)
		AUDIO_SYS_WARN(DBSTR" Failed to load bank list file '%s'", fullPath.Chars());

	AUDIO_SYS_DEBUG("Loaded %d bank list files", m_ObjectBankMap.Size());

	MEMORY_TRACKER_POP("Audio.BankLists");

	return true;
}

const Btl::String Audio::C_ObjectBankList::StripCarriageReturns(const Btl::String& strFname)
{
	S32 length = strFname.Length();

	Btl::Vector<char> tmp;
	
	// reserve enough chars for the whole string so we dont have to dynamically resize
	tmp.Reserve(length+1);

	const char* pSrcChars = strFname.Chars();
	for (S32 i=0; i<length; i++)
	{
		const char c = pSrcChars[i];
		if (c != '\n' && c != '\r')
			tmp.PushBack(c);
	}

	tmp.PushBack('\0');

	return Btl::String(&tmp[0]);
}

void Audio::C_ObjectBankList::Add(const Btl::String& strFname, const Btl::String& str)
{
	BankList bankList;

	// if string is "nil" just add an empty list
	if (str != "nil")
		str.Tokenise(bankList, ',');

	// strip carriage returns 
	Btl::String stripped = StripCarriageReturns(strFname);

	StringHash hash(stripped.Chars());

	#ifndef CFG_GOLD
	if (Lookup(strFname))
	{
		AUDIO_SYS_ERROR(DBSTR" potential string hash collision! Trying to add using the hash of '%s' but it's already in the list.", stripped.Chars());
		return;
	}
	#endif

	//AUDIO_SYS_INFO(DBSTR" Adding '%s', hash 0x%x to object bank lists", stripped.Chars(), hash.GetValue());

	m_ObjectBankMap.Insert(hash, bankList);
}

bool Audio::C_ObjectBankList::LoadBankList(const Sys::C_FilePath& strFname)
{
	Sys::I_FileSystem& fs = System::Get()->GetFileSystem();

	Sys::I_File* pFile = fs.Open(strFname.Chars());

	if (!pFile)
		return false;

	if (!pFile->Length())
	{
		delete pFile;
		return false;
	}

	U32 uSize = static_cast<U32>(pFile->Length());

	const char* pBuf = new char[uSize];

	if (!pBuf)
	{
		delete pFile;
		return false;
	}

	if (pFile->Read((void*)pBuf, uSize) == false)
	{
		delete pFile;
		delete [] pBuf;
		return false;
	}

	// convert file contents to a string
	Btl::String strAll(pBuf, uSize);
	delete [] pBuf;

	Btl::Vector<Btl::String> strings;

	// now get a list of strings
	strAll.Tokenise(strings, ';');

	for (Btl::Vector<Btl::String>::Iterator it=strings.Begin(); it!=strings.End(); ++it)
	{
		Btl::String str = *it;

		S32 commaPos = str.Search(',');
		if (commaPos != -1)
		{
			Btl::String datName = str(0, commaPos);
			Btl::String bafsList = str(commaPos+1, str.Length()-1);
			Add(datName, bafsList);
		}
	}


	delete pFile;

	return true;
}

void Audio::C_ObjectBankList::Clear()
{
	MEMORY_TRACKER_PUSH("Audio.BankLists");

	m_ObjectBankMap.Clear();

	MEMORY_TRACKER_POP("Audio.BankLists");
}

const Audio::BankList* Audio::C_ObjectBankList::Lookup(const Btl::String& strObjectName) const
{
	StringHash hash(strObjectName.Chars());

	Btl::Map<StringHash, BankList>::ConstIterator it = m_ObjectBankMap.Find(hash);

	if (it == m_ObjectBankMap.End())
		return NULL;

	return &it->GetValue();
}
