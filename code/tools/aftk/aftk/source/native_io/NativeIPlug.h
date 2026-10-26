#ifndef NATIVE_IPLUG_H
#define NATIVE_IPLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeIPlug : public C_NativeIObject
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const I_Plug* pPlug);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk, 
			I_Plug* pPlug);

		static bool ResolveUnmanagedDgNodePointers(
			U32 version,
			C_NativeFtk* pNativeFtk,
			I_Plug* pPlug);
	};
}

#endif

