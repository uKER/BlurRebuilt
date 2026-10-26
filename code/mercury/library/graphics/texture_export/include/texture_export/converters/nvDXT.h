#ifndef NVDXT_H__INCLUDED
#define NVDXT_H__INCLUDED

namespace ActTools
{
	class TexelFormat;
}

#define NVDXT_QUALITY_FASTEST  "fastest"
#define NVDXT_QUALITY_NORMAL   "normal"
#define NVDXT_QUALITY_HIGHEST  "highest"

namespace NvDXT
{
ActTools::TexelFormat* GetTexelFormat(const char* format, const char* quality = NVDXT_QUALITY_NORMAL);
}

#undef NVDXT_API

#endif // NVDXT_H__INCLUDED

