#ifndef DMAN_DIFFVIEWER_H
#define DMAN_DIFFVIEWER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_ComparisonTable;
class C_AlignedData;
class C_ExcelReader;
class C_Reader;

class C_DiffViewer : public wxFrame
{
public:

	enum {
		ID_RESET = 1,
		ID_ACCEPT_ALL,
	};

	C_DiffViewer(wxWindow* parent,
		C_Reader& reader,
		wxWindowID id = wxID_ANY,
		const wxString& title = wxT("Dialogue Lines Changes"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE);

	void CreateControls();

	std::vector< C_AlignedData > GetAlignedData();
	bool IsDifferent();
	bool AnyMatch();
	int NumOfDifferences();

	void SetModalLoop(wxModalEventLoop* pModalLoop) { m_pModalLoop = pModalLoop; } 
	int GetReturnCode() {  return m_returnCode; }

	void OnAcceptAll(wxCommandEvent& event);
	void OnReset(wxCommandEvent& event);
	void OnOk(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);

private:
	C_Reader& m_reader;
	C_ComparisonTable* m_pCompTable;
	wxModalEventLoop* m_pModalLoop;
	int m_returnCode;

    DECLARE_EVENT_TABLE()
};

class C_AlignedData 
{
public:
	enum Status
	{
		Status_Added,
		Status_Deleted,
		Status_Changed,
		Status_NoChange,
	};

	C_AlignedData() : leftIndex(-1), rightIndex(-1), status(Status_NoChange), accepted(false)
	{}

	int leftIndex;
	int rightIndex;
	Status status;
	std::vector< wxString > leftLine;
	std::vector< wxString > rightLine;
	std::vector< wxString > leftLineBackup;
	bool accepted;
};

class C_ComparisonTable : public wxGrid
{
public:
	C_ComparisonTable(
		const std::vector< wxString >& fieldLabels,
		const std::vector< std::vector< wxString > >& leftData,
		const std::vector< std::vector< wxString > >& rightData,
		const std::vector< std::vector< wxString > >& leftDataToCompare,
		const std::vector< std::vector< wxString > >& rightDataToCompare,
		wxWindow* parent, C_Reader& reader, const wxWindowID id,
	    const wxPoint& pos, const wxSize& size,
	    long style);

	virtual ~C_ComparisonTable(){};

	void OnCellLeftClick(wxGridEvent& event);
	void OnCellLeftDClick(wxGridEvent& event);

	virtual void ProcessGridCellMouseEvent( wxMouseEvent& event );

private:

	bool IsColComparable(const wxString& colLabel);

	C_Reader& m_reader;
	wxGridCellCoords m_prevCoords;
	
	DECLARE_EVENT_TABLE()
};

class C_ComparisonTableBase : public wxGridTableBase
{
public:
	C_ComparisonTableBase(
		const std::vector< wxString >& fieldLabels,
		const std::vector< std::vector< wxString > >& leftData,
		const std::vector< std::vector< wxString > >& rightData,
		const std::vector< std::vector< wxString > >& leftDataToCompare,
		const std::vector< std::vector< wxString > >& rightDataToCompare);

	virtual int GetNumberRows();
	virtual int GetNumberCols();

	virtual bool IsEmptyCell(int row, int col);

	virtual wxString GetValue(int row, int col);
	virtual void SetValue(int row, int col, const wxString& value);

	virtual wxString GetTypeName(int row, int col);

	virtual bool CanGetValueAs(int row, int col, const wxString& typeName);
	virtual bool CanSetValueAs(int row, int col, const wxString& typeName);

	virtual bool   GetValueAsBool(int row, int col);

	virtual void SetValueAsBool(int row, int col, bool value);

	//virtual wxString GetRowLabelValue( int row );
	virtual wxString GetColLabelValue( int col );

	void Accept(int row, bool value = true);

	std::vector< wxString > m_fieldLabels;
	std::vector< std::vector< wxString > > m_leftData;
	std::vector< std::vector< wxString > > m_rightData;
	std::vector< std::pair<int, int> > m_matchedLines;
	std::vector< C_AlignedData > m_alignedData;
	int m_midCol;


private:
	void AlignData();
};

class C_DiffCellBoolEditor : public wxGridCellBoolEditor
{
public:
	C_DiffCellBoolEditor(){}
	virtual void BeginEdit(int row, int col, wxGrid* grid);
	//bool EndEdit(int row, int col, wxGrid* grid);
};

} //namespace DMan

#endif //DMAN_DIFFVIEWER_H