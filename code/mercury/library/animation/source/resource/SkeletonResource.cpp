#include "stdafx.h"
#include <animation/resource/SkeletonResource.h>
#include <animation/common/StringUtil.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/skeleton/SkeletonDeserialise.inl>
#include <btl/utility/string.h>

namespace Animation
{

C_SkeletonResource::C_SkeletonResource():
	m_skeleton(NULL)
{
}

C_SkeletonResource::~C_SkeletonResource()
{
	delete m_skeleton;
}

bool C_SkeletonResource::LoadResourceData()
{
	Sys::I_File* pFile = OpenResourceFile();

	if( !pFile )
	{
		delete m_skeleton;
		m_skeleton = NULL;
		return false;
	}
	
	bool success = true;
	
	if (m_skeleton == NULL)
	{
		m_skeleton = new C_Skeleton;
		#if defined(PLATFORM_PS3)
		BIZ_ASSERT( (((U32) m_skeleton) & 0xf) == 0 );
		#endif
	}

	{
		Sys::BinaryDeserialiser< Ut::NoEndianSwap > ar(*pFile);
		success = m_skeleton->Deserialise(ar);
	}

	delete pFile;
	
	if (success)
	{
		Btl::String128 name;
		StringUtil::ExtractFilename(name, GetResourceFilename());
		m_skeleton->SetName(name.Chars());
	}
	else
	{
		delete m_skeleton;
		m_skeleton = NULL;
	}
	
	return success;
}

bool C_SkeletonResource::FreeResourceData()
{
	delete m_skeleton;
	m_skeleton = NULL;
	return true;
}

Util::C_String C_SkeletonResource::StaticGetResourceTypeName()
{
	return "Skeleton";
}

S32 C_SkeletonResource::GetEvictDelay()
{
	return 1;
}

} // end Animation
