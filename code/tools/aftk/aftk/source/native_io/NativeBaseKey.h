#ifndef NATIVE_BASE_KEY_H
#define NATIVE_BASE_KEY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeBaseKey
	{
	public:
		static void Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			const C_BaseKey& key)
		{
			*pDiskWriter << key.m_x;
			*pDiskWriter << key.m_value;
		}

		static void Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_BaseKey& key)
		{
			*pDiskReader >> key.m_x;
			*pDiskReader >> key.m_value;
		}

	};
}
#endif
