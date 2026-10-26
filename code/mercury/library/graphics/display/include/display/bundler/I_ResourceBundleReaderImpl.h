#ifndef DISPLAY_I_RESOURCE_BUNDLE_READER_IMPL_H_
#define DISPLAY_I_RESOURCE_BUNDLE_READER_IMPL_H_

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
		class C_ResourceBundleReader;

		class I_ResourceBundleReaderImpl
		{
		public:
			virtual ~I_ResourceBundleReaderImpl() {}

		protected:
			virtual bool Read(
				U32 version,
				U32& sysMemorySize,
				U8*& pSysMemory,
				U32& vidMemorySize,
				U8*& pVidMemory,
				I_ResourceBundle& bundle,
				Sys::I_File& file) = 0;

			friend class C_ResourceBundleReader;
		};
	}
}; // namespace Display

#endif
