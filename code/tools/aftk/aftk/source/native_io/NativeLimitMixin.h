#ifndef NATIVE_LIMIT_MIXIN_H
#define NATIVE_LIMIT_MIXIN_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeLimitMixin
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_LimitMixin* pMixin);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk, 
			C_LimitMixin* pMixin);
	};
}

#endif
