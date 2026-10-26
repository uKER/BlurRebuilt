#include "stdafx.h"

#include <DialogueMan/Components/DataTable/DataGridFilter.h>

#include "wx/regex.h"

namespace DMan
{

C_DataGridFilter::C_DataGridFilter()
{
	m_pRegex = new wxRegEx;
	
	m_pRegex->Compile(wxString::FromUTF8(".*"),wxRE_ADVANCED);
	m_SortOrder = E_SortOrder_AlphaAsc;

	m_SortColumn = 0;
	m_FilterColumn = 0;
}

C_DataGridFilter::~C_DataGridFilter()
{
	delete m_pRegex;
}

void C_DataGridFilter::SetTextFilter(const Btl::String& textFilter)
{
	wxString pLabel = wxString::FromUTF8(textFilter.Chars());
	
	// If no regex is set, use wildcard to show all items
	if (pLabel == wxString::FromUTF8("")) {
		pLabel = wxString::FromUTF8("*");
	}

	// Escape most regex characters, [](){}+
	wxRegEx regexFilter(wxString::FromUTF8("([\\[\\]\\(\\)\\{\\}\\+\\\\])"),wxRE_ADVANCED);
	regexFilter.Replace(&pLabel,wxString::FromUTF8("\\\\\\1"));

	// Replace the 'normal' wildcard, with the regex wildcard
	pLabel.Replace(wxString::FromUTF8("*"), wxString::FromUTF8(".*"), true);

	// Require the regex to match the whole string, and make it case insensitive
	pLabel.Prepend(wxString::FromUTF8("(?i)^.*"));
	pLabel.Append(wxString::FromUTF8(".*$"));

	m_pRegex->Compile(pLabel,wxRE_ADVANCED);

	if (!m_pRegex->IsValid()) {
		// Invalid regex, resort to show all items
		m_pRegex->Compile(wxString::FromUTF8(".*"),wxRE_ADVANCED);
	}
}

bool C_DataGridFilter::MatchText(const Btl::String& text)
{
	return m_pRegex->Matches(text.Chars());
}

void C_DataGridFilter::SetSortOrder(DMan::C_DataGridFilter::E_SortOrder sortOrder)
{
	m_SortOrder = sortOrder;
}

C_DataGridFilter::E_SortOrder C_DataGridFilter::GetSortOrder()
{
	return m_SortOrder;
}

void C_DataGridFilter::SetFilterColumn(C_Column* filterColumn)
{
	m_FilterColumn = filterColumn;
}

void C_DataGridFilter::SetSortColumn(C_Column* sortColumn)
{
	m_SortColumn = sortColumn;
}

C_Column* C_DataGridFilter::GetFilterColumn()
{
	return m_FilterColumn;
}

C_Column* C_DataGridFilter::GetSortColumn()
{
	return m_SortColumn;
}

}