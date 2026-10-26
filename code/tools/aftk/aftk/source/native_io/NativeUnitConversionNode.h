#ifndef NATIVE_UNIT_CONVERSION_NODE_H
#define NATIVE_UNIT_CONVERSION_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeUnitConversionNode : public C_NativeIObject
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_UnitConversionNode* pUnitConversionNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk, 
			C_UnitConversionNode* pUnitConversionNode, 
			bool useOuterBlockHeaders);
	};
}
#endif
