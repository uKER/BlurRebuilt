#ifndef DMAN_COLUMN_SET_H
#define DMAN_COLUMN_SET_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_Column;

class C_ColumnSet
{
public:
	C_ColumnSet();
	~C_ColumnSet();
	
	void		AddColumn(C_Column* pColumn);
	C_Column&	InsertColumn(const wxString& colKey, LarvaField* pLarvaField, S32 index = -1);
	C_Column*	GetColumn(const wxString& colKey);
	C_Column*	GetColumn(size_t colSetKeysIndex);
	U32			GetNumColumns() const;

private:
	std::vector< C_Column* >	m_keys;
	typedef std::vector< C_Column* >::iterator KeysIt;
	typedef std::vector< C_Column* >::const_iterator KeysConstIt;
};



class C_Column
{
public:
	C_Column(const wxString& key, LarvaField* pLarvaField);

	LarvaField* GetLarvaField() const
	{
		return m_pLarvaField;
	}

	const wxString& GetKey() const 
	{
		return m_key;
	}

	const wxString& GetLabel() const
	{
		return m_label;
	}

	bool IsHidden()const
	{
		return m_hidden;
	}

	bool IsSortable() const
	{
		return m_sortable;
	}

	bool IsImportable() const
	{
		return m_importable;
	}

	bool IsComparable() const
	{
		return m_comparable;
	}

	E_DataType GetDataType() const
	{
		return m_dataType;
	}

	const Btl::String& GetMetaData() const
	{
		return m_metaData;
	}

	int GetWidth() const
	{
		return m_width;
	}

	C_Column& SetLabel(const wxString& label) 
	{
		m_label = label;
		return *this;
	}

	C_Column& SetHidden(bool hidden) 
	{
		m_hidden = hidden;
		return *this;
	}

	C_Column& SetSortable(bool sortable) 
	{
		m_sortable = sortable;
		return *this;
	}

	C_Column& SetImportable(bool importable) 
	{
		m_importable = importable;
		return *this;
	}

	C_Column& SetComparable(bool comparable) 
	{
		m_comparable = comparable;
		return *this;
	}

	C_Column& SetDataType(E_DataType dataType) 
	{
		m_dataType = dataType;
		return *this;
	}

	C_Column& SetMetaData(const Btl::String& metaData)
	{
		m_metaData = metaData;
		return *this;
	}
	
	C_Column& SetWidth(int width)
	{
		m_width = width;
		return *this;
	}

	C_Column* Clone()
	{
		C_Column* p		= new C_Column(m_key, m_pLarvaField);
		p->m_label		= m_label;
		p->m_hidden		= m_hidden;
		p->m_sortable	= m_sortable;
		p->m_importable	= m_importable;
		p->m_comparable	= m_comparable;
		p->m_dataType	= m_dataType;
		p->m_metaData	= m_metaData;
		p->m_width		= m_width;
		return p;
	}

private:
	const wxString	m_key;
	wxString		m_label;
	bool			m_hidden;
	bool			m_sortable;
	bool			m_importable;
	bool			m_comparable;
	E_DataType		m_dataType;
	LarvaField*		m_pLarvaField;
	Btl::String		m_metaData;
	int				m_width;
};

}

#endif //DMAN_COLUMN_SET_H