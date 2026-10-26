#ifndef ANIMATION_CYCLERESOURCE_H
#define ANIMATION_CYCLERESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource/Resource.h>

namespace Sys
{
	class C_FilePath;
}

namespace Animation
{
	class C_Cycle;

	/**
		Manages a C_Cycle using the Mercury resource system.
	*/
	class C_CycleResource: public Resource::C_Resource<C_CycleResource>
	{
	public:
		static const Sys::C_FilePath kHeaderOnlySuffix;
	
		C_CycleResource();
		
		virtual ~C_CycleResource();
	
		/// Returns cycle object or NULL if cycle not loaded.
		const C_Cycle* GetCycle() const;
	
		/// Loads cycle from resource file.
		virtual bool LoadResourceData();
		
		/// Frees memory allocated by cycle.
		virtual bool FreeResourceData();
		
		/// Gets cycle resource type name.
		static Util::C_String StaticGetResourceTypeName();

		/// Gets eviction delay.
		virtual S32 GetEvictDelay();
	
	private:
		C_Cycle*	m_cycle;
	};
	
	typedef Resource::C_Handle<C_CycleResource> C_CycleResourceHandle;

	inline const C_Cycle* C_CycleResource::GetCycle() const
	{
		if (m_cycle != NULL)
		{
			BIZ_ASSERT(IsDataValid());
		}
		return m_cycle;
	}
}

#endif

