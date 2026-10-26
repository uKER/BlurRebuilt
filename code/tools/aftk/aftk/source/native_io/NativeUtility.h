#ifdef _MSC_VER_
#pragma once
#endif

#ifndef NATIVE_UTILITY_H
#define NATIVE_UTILITY_H

namespace Aftk
{
	class C_NativeUtility
	{
	public:
		FORCE_INLINE static bool IsLittleEndian() 
		{ 
			U32 dummy=1;
			return 1==*((U8*)&dummy); 
		}

		template<typename T>
		FORCE_INLINE static T EndianSwitch(T in)
		{
			T out;

			// endian switching
			char *pIn = (char*)&in;
			char *pOut = (char*)&out;

			int c = sizeof(T);
			for (int i=0; i<c; i++)
				pOut[i] = pIn[c - i - 1];

			return out;
		}

		static void EndianSwitch(void* p, U32 nBytes);

		static U32 GetBlockIdInt(const char* id);
	};
}
#endif
