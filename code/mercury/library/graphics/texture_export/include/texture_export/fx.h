#ifndef __TEXTURE_EXPORT_FX_H__
#define __TEXTURE_EXPORT_FX_H__

namespace Sys
{
class I_BinaryStream;
}

bool SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char* flags );

#endif
