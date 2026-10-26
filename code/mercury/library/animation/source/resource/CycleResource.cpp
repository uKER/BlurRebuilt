#include "stdafx.h"
#include <animation/resource/CycleResource.h>
#include <animation/common/StringUtil.h>
#include <animation/cycle/Cycle.h>
#include <animation/cycle/CycleDeserialise.inl>
#include <btl/utility/string.h>

namespace Animation
{

const Sys::C_FilePath C_CycleResource::kHeaderOnlySuffix("$header");

C_CycleResource::C_CycleResource():
	m_cycle(NULL)
{
	
}

C_CycleResource::~C_CycleResource()
{
	delete m_cycle;
}

bool C_CycleResource::LoadResourceData()
{
	Sys::C_FilePath magicSuffix(kHeaderOnlySuffix);
	Sys::C_FilePath fn = GetResourceFilename();

	U32 loadFlags = 0;

	// Look for suffix that indicates only the cycle header should be loaded (a cheap way of saving memory).
	S32 magicSuffixPos = fn.Search(magicSuffix);
	if (magicSuffixPos >= 0 && magicSuffixPos == fn.Length() - magicSuffix.Length())
	{
		fn = fn(0, magicSuffixPos);
		loadFlags = Animation::C_Cycle::kLoadHeaderOnly;
	}
			
	Sys::I_File* pFile = OpenResourceFile(fn);

	if( !pFile )
	{
		delete m_cycle;
		m_cycle = NULL;
		return false;
	}
	
	bool success = true;
	
	// Create cycle if one doesn't already exist.
	if (m_cycle == NULL)
	{
		m_cycle = new C_Cycle;
		#if defined(PLATFORM_PS3)
		BIZ_ASSERT( (((U32) m_cycle) & 0xf) == 0 );
		#endif
	}
	
	// Read data.
	{
		Sys::BinaryDeserialiser< Ut::NoEndianSwap > ar(*pFile);
		success = m_cycle->Deserialise(ar, loadFlags);
	}

	delete pFile;
	
	if (success)
	{
		// Set cycle's name
		Btl::String128 name;
		StringUtil::ExtractFilename(name, GetResourceFilename());
		if (loadFlags == Animation::C_Cycle::kLoadHeaderOnly)
		{
			name += '*';
		}
		m_cycle->SetName(name.Chars());
	}
	else
	{
		// An error occurred, so destroy the cycle.
		delete m_cycle;
		m_cycle = NULL;
	}
	
	return success;
}

bool C_CycleResource::FreeResourceData()
{
	delete m_cycle;
	m_cycle = NULL;
	return true;
}

Util::C_String C_CycleResource::StaticGetResourceTypeName()
{
	return "Cycle";
}

S32 C_CycleResource::GetEvictDelay()
{
	return 1;
}

} // end Animation
