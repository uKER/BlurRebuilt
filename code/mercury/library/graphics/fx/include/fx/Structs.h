#ifndef __FX_STRUCTS_H__
#define __FX_STRUCTS_H__

namespace FX
{

/// add the given struct declaration into to the manager
void RegisterStruct(Struct *pDecl);

/// get a struct declaration for the given struct name
Struct *GetStruct(const Util::C_String &sName);

} // namespace FX

#endif
