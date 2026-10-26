#include "SecurityPch.h"

#include "Security\RSASecurity.h"

extern "C"
{
#include "tomcrypt.h"
}

namespace Security
{

Debug::Logger& g_Log = Debug::GetLogger("mercury.security");

RSASecurity::RSASecurity(unsigned int rsaKeyLength)
{
	m_key = m_signature = NULL;
	m_keyLength = m_sigLength = 0;
	m_rsaKeyLength = rsaKeyLength / 8;
}

RSASecurity::~RSASecurity()
{
	delete [] m_key;
	delete [] m_signature;
}

void RSASecurity::InitKey(const U8* key, size_t keyLength)
{
	BIZ_ASSERT(key != NULL && keyLength > 0);

	m_keyLength = keyLength;
	
	m_key = new U8[m_keyLength];
	memcpy(m_key, key, m_keyLength);
}

void RSASecurity::InitSignature(const Btl::String& sigFilename)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	if (!fs.FileExists(sigFilename))
	{
		LOGERROR(g_Log, "RSASecurity::InitSignature: File '%s' does not exist.", sigFilename.Chars());
		return;
	}

	Sys::I_File* file = fs.Open(sigFilename, Sys::E_FileAccess_Read);
	if (file == 0)
	{
		LOGERROR(g_Log, "RSASecurity::InitSignature: File '%s' failed to open.", sigFilename.Chars());
		return;
	}

	// We only load in the signature if it's the correct length for the RSA key
	if (file->Length() == m_rsaKeyLength)
	{
		m_sigLength = (size_t)file->Length();

		m_signature = new U8[m_sigLength];

		file->Read(m_signature, m_sigLength);
	}
	else
	{
		LOGFATAL(g_Log, "RSASecurity::InitSignature: File '%s' is not the right size.", sigFilename.Chars());
	}

	delete file;
}

bool RSASecurity::Verify(const U8* buffer, size_t bufferLength, const char* name) const
{
	if ((m_key == NULL) || (m_keyLength == 0))
	{
		BIZ_ASSERT_FAIL("RSASecurity::Verify - Problem with the public key - Ensure InitKey was called");
		return false;
	}

	if ((m_signature == NULL) || (m_sigLength == 0))
	{
		BIZ_ASSERT_FAIL("RSASecurity::Verify - Problem with the signature - Ensure InitSignature was called with a file that exists");
		return false;
	}

	// Register a math library (in this case TomsFastMath)
	ltc_mp = ltm_desc;

	if (register_hash(&sha1_desc) == -1)
	{
		LOGERROR(g_Log, "Error registering sha1");
		return false;
	}

	int hash_idx = find_hash("sha1");

	// Import the public key
	rsa_key rsaKey;

	int err = rsa_import(m_key, m_keyLength, &rsaKey);
	if (err != CRYPT_OK)
	{
		LOGERROR(g_Log, "Error importing key");
		return false;
	}

	// Check hash with public key now
	bool hashValid = true;
	int stat;

	err = rsa_verify_hash(m_signature, m_sigLength, buffer, bufferLength, hash_idx, 8, &stat, &rsaKey);
	if (err != CRYPT_OK)
	{
		LOGERROR(g_Log, "Error verifying hash for '%s' (%d)", name ? name : "Unknown", err);
		hashValid = false;
	}
	else if (stat == 0)
	{
		LOGERROR(g_Log, "*** Hash doesn't match for '%s' ***", name ? name : "Unknown");
		hashValid = false;
	}

#ifndef CFG_GOLD
	return true;
#else
	return hashValid;
#endif
}

}

