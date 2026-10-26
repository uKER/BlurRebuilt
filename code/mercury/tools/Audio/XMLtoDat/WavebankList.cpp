#include "AudioPch.h"

#include "WaveBankList.h"

using namespace Audio;

bool WaveBankList::Add(const Btl::String& strBankFname)
{
	Btl::Vector<Btl::String>::Iterator it;

	for (it = m_vFilenames.Begin(); it != m_vFilenames.End(); ++it)
		if (*it	==strBankFname)
			return true;

	m_vFilenames.PushBack(strBankFname);
	return true;
}

bool WaveBankList::WriteToDisk(const Btl::String& strFname)
{
	if (m_vFilenames.Size() == 0)
	{
		Add("nil");
	}

	printf("writing wave bank list file '%s'...\n", strFname.Chars());

	FILE* pFile = fopen(strFname.Chars(), "wb");

	if (! pFile)
		return false;

	Btl::Vector<Btl::String>::Iterator it;

	for (it = m_vFilenames.Begin(); it != m_vFilenames.End(); ++it)
	{
		const char* pchBuf = it->Chars();
		size_t sNumToWrite = it->Length();

		size_t sWritten = fwrite(pchBuf, sizeof(char), sNumToWrite, pFile);

		if (sWritten < sNumToWrite)
			return false;

		if (!fwrite(",", 1, 1, pFile))
			return false;
	}

	if (fclose(pFile) != 0)
		return false;

	return true;
}