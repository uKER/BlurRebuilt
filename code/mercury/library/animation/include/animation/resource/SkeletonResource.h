#ifndef ANIMATION_SKELETONRESOURCE_H
#define ANIMATION_SKELETONRESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource/Resource.h>

namespace Animation
{
	class C_Skeleton;

	/**
		Manages a C_Skeleton using the Mercury resource system.
	*/
	class C_SkeletonResource: public Resource::C_Resource<C_SkeletonResource>
	{
	public:
			C_SkeletonResource();
			
			virtual ~C_SkeletonResource();
	
			/// Returns a pointer to the skeleton object or NULL if it isn't loaded.
			const C_Skeleton* GetSkeleton() const;
		
			/// Loads skeleton from resource file.
			virtual bool LoadResourceData();
			
			/// Frees memory allocated by skeleton.
			virtual bool FreeResourceData();
			
			/// Gets skeleton resource type name.
			static Util::C_String StaticGetResourceTypeName();
	
			/// Gets eviction delay.
			virtual S32 GetEvictDelay();
	
	private:
		C_Skeleton*	m_skeleton;
	};
	
	typedef Resource::C_Handle<C_SkeletonResource> C_SkeletonResourceHandle;
	
	inline const C_Skeleton* C_SkeletonResource::GetSkeleton() const
	{
		return m_skeleton;
	}

	
}

#endif
