#ifndef SCENE_GRAPH_FILTER_H
#define SCENE_GRAPH_FILTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorApp/Components/scene_graph/SceneGraphEnums.h>

namespace Editor
{

class C_SceneGraphFilter
{
public:
	enum E_SortFilter
	{
		E_SortFilter_AlphaAsc,
		E_SortFilter_AlphaDesc,
		E_SortFilter_Placement,
		E_SortFilter_TypeAsc,
		E_SortFilter_TypeDesc
	};

	C_SceneGraphFilter(void);
	void SetTextFilter(const Btl::String& filterText);
	void SetGlobalFilter(E_SceneGraphDataType objType, bool value);
	void SetActorFilter(const Btl::String& actor, bool value);
	void SetSortFilter(E_SortFilter sortFilter);

	bool MatchText(const Btl::String& text);
	bool MatchDgNode(E_SceneGraphDataType dgNodeType, const EditorLib::I_DgNode* dgNode);

	E_SortFilter GetSortFilter();

private:
	wxRegEx regex;

	E_SortFilter m_SortFilter;

	Ut::UtMap<Btl::String,bool> m_ActorFilters;
	Ut::UtMap<E_SceneGraphDataType,bool> m_GlobalFilters;
};

}

#endif