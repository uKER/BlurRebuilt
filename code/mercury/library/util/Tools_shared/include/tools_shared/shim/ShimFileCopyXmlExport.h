#ifndef MERCURY_SHIM_FILECOPY_XML_EXPORT_H
#define MERCURY_SHIM_FILECOPY_XML_EXPORT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Tools
{
	namespace FileCopy
	{
		class C_FileCopyList;
	}

	namespace Shim
	{
		typedef Ut::UtVector<Ut::ArrayVector<FileCopy::C_PlatformCopyLocations, Util::E_Platform_Count> >
			CopyLocations;

		class C_FileCopyXmlExport
		{
		public:
			static void ClearXmlFileList(
				const Util::C_String& inputFilePath,
				const Util::C_String& revision);

			static bool ExportXmlFileList(
				const Util::C_String& rootTempPath,
				Tools::FileCopy::C_FileCopyList& uberCopyList,
				const Util::C_String& inputFilePath,
				const CopyLocations& copyLocations,
				const Util::C_String& launchComputerName,
				const Util::C_String& revision,
				Util::C_String* pCopyToWorking = 0,
				Util::C_String* pCopyToImage = 0);

		private:
			static Util::C_String BuildXmlFileListName(
				const Util::C_String& inputFilePath,
				const Util::C_String& suffix);
		};
	}
}

#endif
