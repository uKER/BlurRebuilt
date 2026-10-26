#pragma once

class AftkSceneCollection
{
public:
	struct AssetFolder
	{
		bool dontExport;
	};

	struct Asset
	{
		Asset() {}

		Asset(const Asset& rhs)
		{
			*this = rhs;
		}

		const Asset& operator=(const Asset& rhs)
		{
			inputFullPath = rhs.inputFullPath;
			outputFullPath = rhs.outputFullPath;
			containsReferences = rhs.containsReferences;
			return *this;
		}

		Util::C_String inputFullPath;
		Util::C_String outputFullPath;
		bool containsReferences;
	};

	static bool GetOutputDirectory(
		Util::C_String& outputDirectory,
		const Sys::C_XmlDomBaseNode& root,
		const C_Options& options);

	static bool GetOutputFilename(
		Util::C_String& outputFilename,
		const Sys::C_XmlDomBaseNode& root,
		const C_Options& options);

	static bool GetAssetArray(
		Ut::UtVector<Ut::DynamicVector<Asset> >& assets, 
		const Util::C_String& partialDirectory,
		const Sys::C_XmlDomBaseNode* pNode,
		const C_Options& options);

private:
	static bool GetFullA3DPath(
		Util::C_String& outputFullPath, 
		const Util::C_String& outputDirectory,
		const Util::C_String& inputFullPath,
		const C_Options& options);
};
