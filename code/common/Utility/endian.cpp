#include "stdafx.h"
#pragma warning (disable:4100) // unreferenced formal parameter
#pragma warning (disable:4127) // conditional expression is constant
#include "endian.h"

namespace Ut
{

void DoEndianSwap::EndianSwapQ(void* out, void* in)
{
	assert(out != in);

	char* outPtr = (char*) out;
	char* inPtr  = (char*) in;

	outPtr[0] = inPtr[7];
	outPtr[1] = inPtr[6];
	outPtr[2] = inPtr[5];
	outPtr[3] = inPtr[4];
	outPtr[4] = inPtr[3];
	outPtr[5] = inPtr[2];
	outPtr[6] = inPtr[1];
	outPtr[7] = inPtr[0];
}

void DoEndianSwap::EndianSwapD(void* out, void* in)
{
	assert(out != in);

	char* c_out = (char*) out;
	char* c_in = (char*) in;

	c_out[0] = c_in[3];
	c_out[1] = c_in[2];
	c_out[2] = c_in[1];
	c_out[3] = c_in[0];
}

void DoEndianSwap::EndianSwapW(void* out, void* in)
{
	assert(out != in);

	char* c_out = (char*) out;
	char* c_in = (char*) in;

	c_out[0] = c_in[1];
	c_out[1] = c_in[0];
}

__forceinline void SwapBytes(char& x, char& y)
{
	char tmp = x;
	x = y;
	y = tmp;
}

void DoEndianSwap::EndianSwapQ(void* ptr)
{
	char* cptr = (char*) ptr;
	SwapBytes(cptr[0], cptr[7]);
	SwapBytes(cptr[1], cptr[6]);
	SwapBytes(cptr[2], cptr[5]);
	SwapBytes(cptr[3], cptr[4]);
}

void DoEndianSwap::EndianSwapD(void* ptr)
{
	char*& cptr = (char*&) ptr;
	char c;

	c		= cptr[0];
	cptr[0]	= cptr[3];
	cptr[3]	= c;

	c		= cptr[1];
	cptr[1]	= cptr[2];
	cptr[2]	= c;
}

void DoEndianSwap::EndianSwapW(void* ptr)
{
	char*& cptr = (char*&) ptr;
	char c;

	c		= cptr[0];
	cptr[0]	= cptr[1];
	cptr[1]	= c;
}

void DoEndianSwap::EndianSwap(void *ptr, const char *pszFormat, DWORD dwNumItems)
{
	for (DWORD i=0; i < dwNumItems; i++)
	{
		const char *psz = pszFormat;

		for (; *psz != '\0'; psz++)
		{
			DWORD dwSize(0);

			switch (*psz)
			{
			case 'x':	// Padding
			case 'B':	// Byte - Can effectively skip
			case '1':
			{
				dwSize = 1;
				break;
			}

			case 'S':	// Short, WORD - 8 in 16 swap
			case 'W':
			case '2':
			{
				{
					char *cptr = (char*)ptr;
					char c;

					c		= cptr[0];
					cptr[0]	= cptr[1];
					cptr[1]	= c;
				}

				dwSize = 2;
				break;
			}
			
			case 'L':	// Long, DWORD - 8 in 32 Swap
			case 'D':
			case 'F':
			case '4':
			{
				{
					char *cptr = (char*)ptr;
					char c;

					c		= cptr[0];
					cptr[0]	= cptr[3];
					cptr[3]	= c;

					c		= cptr[1];
					cptr[1]	= cptr[2];
					cptr[2]	= c;
				}

				dwSize = 4;
				break;
			}

			default:
				assert(false);
				break;
			}

			ptr = (void*) ((BYTE*)ptr + dwSize);
		}
	}
}

}
