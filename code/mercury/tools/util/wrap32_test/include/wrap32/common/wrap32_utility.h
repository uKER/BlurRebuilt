#pragma once

class C_Wrap32Utility
{
public:
	static UINT CreateSharedMemory(UINT size);

	static BOOL DestroySharedMemory();

	static U8* LockSharedMemory();

	static BOOL UnlockSharedMemory(U8* pMappedMemory);

	static BOOL DiscardSharedMemory(U8* pMappedMemory);

#ifdef _PSAPI_H_
	static bool TerminateProcessesByName(const char* pName);
#endif

	template<class T>
	static void SetPointerFromOffset(
		T& pointer,
		const LPVOID pBaseAddress,
		const UINT offset)
	{
		#pragma warning(push)
		{
			#pragma warning(disable: 4312)
			#pragma warning(disable: 4311)
			if (offset == -1)
			{
				pointer = 0;
				return;
			}

			//Fixup offsets so that they become real pointers.
			pointer = reinterpret_cast<T>((UINT)pBaseAddress + offset);
		}
		#pragma warning(pop)
	}
};
