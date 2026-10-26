#pragma once

class Sys::C_XmlDomBaseNode;
class Sys::C_XmlDom;

class NexusExportWrapper
{
public:
	static bool Convert(	
		const C_Options& options,
		const Sys::C_XmlDomBaseNode& rootNode,
		const Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> >& assets,
		Ut::UtVector<Ut::DynamicVector<Util::C_String> >& exportedAssets);

	static bool Merge(	
		const C_Options& options,
		const Sys::C_XmlDomBaseNode& rootNode,
		const Util::C_String& outputFilePath,
		const Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> >& assets,
		Util::C_String& exportedAsset);

private:
	struct C_NexusOptions
	{
		//Nexus arguments.
		Util::C_String args;
	};

	static bool ParseOptions(C_NexusOptions& options, const Sys::C_XmlDomBaseNode& rootNode);
};
