#ifndef DISPLAY_SPU_COMMAND_BUFFER_H
#define DISPLAY_SPU_COMMAND_BUFFER_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



U32 PatchForLocalStallSync(U32 * const pAddress, const U32 size);



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_COMMAND_BUFFER_H
