class Sys::C_XmlDomBaseNode;
class Sys::C_XmlDom;

Util::C_String GetLeft(const Util::C_String& string, int s);

Util::C_String GetRight(const Util::C_String& string, int s);

Util::C_String ExpandToolPath(const C_Options& options, const Util::C_String& toolPath);

bool RemoveReadOnlyFlag(const Util::C_String& filePath);

Util::C_String GetAssetFullPath(
	const Util::C_String& assetPath,
	U32 exportInfo,
	const Util::C_String& launchComputerName);