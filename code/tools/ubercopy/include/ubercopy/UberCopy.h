#ifndef UBERCOPY_INCLUDE
#define UBERCOPY_INCLUDE

#include "tools_shared/ToolsShared.h"
#include "tools_shared/file_copy/FileCopy.h"

namespace FileCopy
{
	class C_FileCopyItem;
	class C_FileCopyList;
	class C_FileCopyListXml;
}

namespace UberCopy
{

class C_UberCopy
{
public:

	static bool CopyItem(
		const Tools::FileCopy::C_FileCopyItem &copyItem,
		Util::E_Platform platform,
		const Util::C_String *console = 0);

	static bool CopyUnknownItem(
		const Util::C_String &src,
		const Util::C_String &dst,
		Util::E_Platform platform,
		const Util::C_String *console = 0);

	static bool CopyList(
		const Tools::FileCopy::C_FileCopyList &copyList,
		bool skipPlatformOnFailure = false,
		U32 platforms = 0xffffffff);


	static bool CopyXmlList(
		const Tools::FileCopy::C_FileCopyListXml &copyList,
		bool skipPlatformOnFailure = false,
		U32 platforms = 0xffffffff);

private:

	static bool RecursivelyCopyDirectory(
		const Util::C_String &src,
		const Util::C_String &dst,
		Util::E_Platform  platform,
		const Util::C_String *console = 0);
	
	static bool CopySingleFile(
		const Util::C_String &src,
		const Util::C_String &dst,
		Util::E_Platform platform,
		const Util::C_String *console = 0);
};

}
#endif