#ifndef NATIVE_STATIC_PLUG_H
#define NATIVE_STATIC_PLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	template<class T>
	class C_NativeStaticPlug : C_NativeNullPlug
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_StaticPlug<T>* pPlug);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk, 
			C_StaticPlug<T>* pPlug);
	};

	#include "NativeStaticPlug.inl"
}

#endif

