#ifndef __SYSTEM_MEMORY_CALLSTACK_H__
#define __SYSTEM_MEMORY_CALLSTACK_H__

namespace Sys
{
class I_BinaryStream;
}

namespace Memory
{

/// Capture the callstack and store as a series of function pointers
///		pCallStack = is the pointer array to receive the callstack
///		cMax = is the maximum size of the callstack array
int CaptureCallStack(void **pCallStack, int cMax);

/// save module info to binary file - for reading in with mtune (PC only)
void SaveModuleInfo(Sys::I_BinaryStream *pStream);

/// lookup address and return symbol string
void GetAddressInfo(void *pAddress, char *buffer);

}

#endif
