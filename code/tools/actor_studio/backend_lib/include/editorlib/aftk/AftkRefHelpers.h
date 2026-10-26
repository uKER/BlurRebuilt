#ifndef EDITOR_LIB_AFTK_REF_HELPERS_H
#define EDITOR_LIB_AFTK_REF_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <misc_shared/Variant/KeyValue.h>

namespace EditorLib
{
	namespace AftkRefHelpers
	{
		bool RefConversionRequired();

		bool LoadAllReferences(bool convert, bool loadVisualisations, void(*pYieldCallback)());

		bool ReloadReferences(bool convert, bool reloadVisualisations, void(*pYieldCallback)());

		bool GetLatestReferences(void(*pYieldCallback)());

		bool GetLatestReferences(Btl::Vector<Btl::String>& filePaths, void(*pYieldCallback)());

		void UnloadAllReferences(bool unloadVisualisations, void(*pYieldCallback)());

		bool IsToBeHiddenPermanently(const Aftk::C_SceneNode* pSceneNode);

		bool IsActorStudioProperty(const Aftk::C_DynamicPlug* pPlug);

		void CopyAftkAttribute(
			Util::C_KeyValue& editorAttr, 
			const Aftk::C_GenericAttribute& aftkAttr);

		void GetActorStudioAttrs(
			Btl::Vector<Util::C_KeyValue>& attrs,
			const Aftk::C_SceneNode* pAftkSceneNode);

		bool GetAftkActorPlaceholderProperties(
			const Util::C_KeyValue** ppAsDynamic,
			const Util::C_KeyValue** ppAsActor,
			const Btl::Vector<Util::C_KeyValue>& attrs,
			const Btl::String& levelName);

		bool IsSceneOutOfDate(const Btl::String& originalFilePath, Btl::String& cacheFilePath);

		bool ConvertScene(const Btl::String& originalFilePath, Btl::String& cacheFilePath, bool* pConverted = 0);
	}
}

#endif
