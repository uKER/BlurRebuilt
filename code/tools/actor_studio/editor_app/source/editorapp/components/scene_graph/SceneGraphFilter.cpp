#include "stdafx.h"
#include <wx/treectrl.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>

#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Misc/StringUtil.h>

#include <EditorApp/Components/Scene_graph/SceneGraphFilter.h>

namespace Editor
{

C_SceneGraphFilter::C_SceneGraphFilter(void)
{
	regex.Compile(".*",wxRE_ADVANCED);
	m_SortFilter = E_SortFilter_Placement;
}

void C_SceneGraphFilter::SetTextFilter(const Btl::String& textFilter)
{
	wxString pLabel = textFilter.Chars();
	
	// If no regex is set, use wildcard to show all items
	if (pLabel == "" ) {
		pLabel = "*";
	}

	// Escape most regex characters, [](){}+
	wxRegEx regexFilter("([\\[\\]\\(\\)\\{\\}\\+\\\\])",wxRE_ADVANCED);
	regexFilter.Replace(&pLabel,"\\\\\\1");

	// Replace the 'normal' wildcard, with the regex wildcard
	pLabel.Replace("*", ".*", true);

	// Require the regex to match the whole string, and make it case insensitive
	pLabel.Prepend("(?i)^.*");
	pLabel.Append(".*$");

	regex.Compile(pLabel,wxRE_ADVANCED);

	if (!regex.IsValid()) {
		// Invalid regex, resort to show all items
		regex.Compile(".*",wxRE_ADVANCED);
	}
}

void C_SceneGraphFilter::SetGlobalFilter(E_SceneGraphDataType objType, bool value)
{
	if (value) {
		m_GlobalFilters[objType] = true;
	} else {
		Ut::UtMap<E_SceneGraphDataType,bool>::Iterator item = m_GlobalFilters.Find(objType);
		if (item != m_GlobalFilters.End()) {
			m_GlobalFilters.Erase(objType);
		}
	}
}

void C_SceneGraphFilter::SetActorFilter(const Btl::String& actor, bool value)
{
	if (value) {
		m_ActorFilters[actor] = true;
	} else {
		Ut::UtMap<Btl::String,bool>::Iterator item = m_ActorFilters.Find(actor);
		if (item != m_ActorFilters.End()) {
			m_ActorFilters.Erase(item);
		}
	}
}

bool C_SceneGraphFilter::MatchText(const Btl::String& text)
{
	return regex.Matches(text.Chars());
}

bool C_SceneGraphFilter::MatchDgNode(E_SceneGraphDataType dgNodeType, const EditorLib::I_DgNode* dgNode)
{
	// Run the global filters
	if (m_GlobalFilters.Size() > 0) {
		bool typeFound = false;
		
		for (Ut::UtMap<E_SceneGraphDataType,bool>::Iterator i=m_GlobalFilters.Begin(); i!=m_GlobalFilters.End(); ++i) {
			if (dgNodeType == i->GetKey()) {
				typeFound = true;
			}
		}
		
		if (!typeFound)
			return false;
	}

	// Run the actor filters
	if (m_ActorFilters.Size() > 0 && dgNode && dgNode->AsXtObject())
	{
		const EditorLib::C_XtObject* pXtObject = dgNode->AsXtObject();
		const Btl::String& actorType = pXtObject->GetXtTypeName();

		bool typeFound = false;
		
		for (Ut::UtMap<Btl::String,bool>::Iterator i=m_ActorFilters.Begin(); i!=m_ActorFilters.End(); ++i) {
			Btl::String val = i->GetKey();
			if (actorType == i->GetKey()) {
				typeFound = true;
			}
		}

		if (!typeFound)
			return false;
	}

	return true;
}

C_SceneGraphFilter::E_SortFilter C_SceneGraphFilter::GetSortFilter()
{
	return m_SortFilter;
}

void C_SceneGraphFilter::SetSortFilter(C_SceneGraphFilter::E_SortFilter sortFilter)
{
	m_SortFilter = sortFilter;
}

}