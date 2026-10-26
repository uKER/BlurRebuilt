#ifndef NATIVE_NULL_PLUG_H
#define NATIVE_NULL_PLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeNullPlug : public C_NativeIPlug
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_NullPlug* pPlug);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk, 
			C_NullPlug* pPlug);
	};
}

#endif

