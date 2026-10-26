#ifndef DISPLAY_RESOURCE_BUNDLE_READER_H_
#define DISPLAY_RESOURCE_BUNDLE_READER_H_

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class I_File;
}

namespace Display
{
	namespace ResourceBundler
	{
		class I_ResourceBundle;

		class C_ResourceBundleReader
		{
		public:
			static bool Read(
				U32& sysMemorySize,
				U8*& pSysMemory,
				U32& vidMemorySize,
				U8*& pVidMemory,
				I_ResourceBundle& bundle,
				const Sys::C_FilePath& fileName);

			static bool Read(
				U32& sysMemorySize,
				U8*& pSysMemory,
				U32& vidMemorySize,
				U8*& pVidMemory,
				I_ResourceBundle& bundle,
				Sys::I_File& file);

		private:
			static U32 GetHeader(const char* headerString);
		};
	}
}; // namespace Display

#endif
