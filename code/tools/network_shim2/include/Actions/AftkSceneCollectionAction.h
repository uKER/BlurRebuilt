#pragma once

#include <assets/AftkSceneCollection.h>

class AftkSceneCollectionAction
{
public:
	static bool Process(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, E_ShimActionTag tag, Ut::Vector<Util::C_String>& outputAssets);

private:
	static bool GetAssetList(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> >& assets, const Util::C_String& outputDirectory);

	static bool ProcessCombine(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, Ut::Vector<Util::C_String>& outputAssets);

	static bool ProcessMultiple(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, Ut::Vector<Util::C_String>& outputAssets);

	static bool ProcessSingle(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, Ut::Vector<Util::C_String>& outputAssets);

	static bool Execute(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, const Util::C_String& outputFilePath, const Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> >& sceneAssets, Ut::Vector<Util::C_String>& outputAssets, bool doMerge);
};
