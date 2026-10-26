#ifndef DISPLAY_I_RESOURCE_BUNDLE_READER_IMPL2_H_
#define DISPLAY_I_RESOURCE_BUNDLE_READER_IMPL2_H_

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

		class C_ResourceBundleReaderImpl2 : public I_ResourceBundleReaderImpl
		{
		public:
			virtual ~C_ResourceBundleReaderImpl2() {}

		protected:
			virtual bool Read(
				U32 version,
				U32& sysMemorySize,
				U8*& pSysMemory,
				U32& vidMemorySize,
				U8*& pVidMemory,
				I_ResourceBundle& bundle,
				Sys::I_File& file);
		};
	}
}; // namespace Display

#endif
