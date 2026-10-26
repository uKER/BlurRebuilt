#ifdef _MSC_VER_
#pragma once
#endif

#ifndef NATIVE_WEIGHT_H
#define NATIVE_WEIGHT_H

namespace Aftk
{
	template <class T>
	class C_NativeWeight
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_Weight<T>* pWeight)
		{
			*pDiskWriter << (S32)pWeight->GetIndex();
			*pDiskWriter << pWeight->GetWeight();

			return C_Status::E_Status_Success;
		}

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_Weight<T>* pWeight)
		{
			S32 index;
			*pDiskReader >> index;
			pWeight->SetIndex(index);

			T weight;
			*pDiskReader >> weight;
			pWeight->SetWeight(weight);

			return C_Status::E_Status_Success;
		}
	};
}
#endif
