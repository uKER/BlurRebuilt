#ifndef NATIVE_GENERIC_ATTRIBUTE_H
#define NATIVE_GENERIC_ATTRIBUTE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeGenericAttribute
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_GenericAttribute* pGenericAttribute, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_GenericAttribute* pGenericAttribute, 
			bool useOuterBlockHeaders);
	};
}

#endif
