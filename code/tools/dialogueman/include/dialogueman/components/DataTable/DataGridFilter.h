#ifndef DMAN_DATA_GRID_FILTER_H
#define DMAN_DATA_GRID_FILTER_H

#ifdef _MSC_VER
#pragma once
#endif

class wxRegEx;

namespace DMan
{

class C_Column;

class C_DataGridFilter
{
public:
	enum E_SortOrder
	{
		E_SortOrder_AlphaAsc,
		E_SortOrder_AlphaDesc,
	};

	C_DataGridFilter();
	~C_DataGridFilter();

	void SetTextFilter(const Btl::String& filterText);
	void SetSortOrder(E_SortOrder sortFilter);
	void SetSortColumn(C_Column* sortColumn);
	void SetFilterColumn(C_Column* filterColumn);

	bool MatchText(const Btl::String& text);

	E_SortOrder GetSortOrder();

	C_Column* GetSortColumn();
	C_Column* GetFilterColumn();

private:
	wxRegEx* m_pRegex;
	E_SortOrder m_SortOrder;
	C_Column* m_SortColumn;
	C_Column* m_FilterColumn;
};

}

#endif