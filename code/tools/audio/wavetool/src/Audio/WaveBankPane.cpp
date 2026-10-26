#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Xt/XtTool.h"
#include "Audio/WaveBankPane.h"
#include "Resource/Resource.h"
#include <core\mercurycore.h>
#include "core/containers\string.h"
#include "System/Misc/StringUtil.h"
#include "StopWatch.h"
#ifndef WAVEUTILS_H
#include "WaveUtils.h"
#endif


#pragma warning(disable: 4311)

const int kWaveBankForm_List				= 1;
const int kWaveBank_Add						= 2;
const int kWaveBank_Remove					= 3;
const int kWaveBank_EditBankName			= 4;
const int kWaveBank_StreamingTickBox		= 5;
const int kWaveBank_WavesDividingLine		= 6;
const int kWaveBank_BankDividingLine		= 7;
const int kWaveBank_BankFormatCombo			= 8;
const int kWaveBank_WavePresetCombo			= 9;
const int kWaveBank_SelectAllWaves			= 10;
const int kWaveBank_WaveLoopCombo			= 11;
const int kWaveBank_WaveFormatCombo			= 12;
const int kWaveBank_WaveSampleRateEditBox	= 13;
const int kWaveBank_WaveNumBitsEditBox		= 14;
const int kWaveBank_WaveQuantizeMethodCombo	= 15;
const int kWaveBank_OpenPreset				= 16;

const int kWaveListColumn_Name				= 0;
const int kWaveListColumn_Preset			= 1;
const int kWaveListColumn_Format			= 2;
const int kWaveListColumn_Frequency			= 3;
const int kWaveListColumn_Bits				= 4;
const int kWaveListColumn_Loop				= 5;
const int kWaveListColumn_Channels			= 6;
const int kWaveListColumn_QuantizeMethod	= 7;


const int kMultiValue_MixedValues	= -2000;
const int kMultiValue_None			= -1000;

Util::C_String strWaveProperties = "Wave Properties";
Util::C_String strWaves = "Waves";
Util::C_String strBank = "Bank";

bool g_bAudioMode = true;

// TODO:
// - need to open wave files to re-scan wave file format when pane is opened if bank has not just been created
// - write out block for each platform for each wave in the array. store preset and/or format details
// - save out the platform as part of the project dom? otherwise we wont know which platform to use as default
// - if they create two identically named wavebank records i need to detect this and warn them at the build stage
// - create a monitor so when a preset is changed the wavebank is updated
// - auto size columns in listview?
// - try displaying summary of format in preset dropdown eg [XMA, 22000Hz, 16bit] ?
// - custom formats
// - only build wavebanks whose records or .wav files have changed since last time - write xml to disk and compare it with xml in memory
// - store bank version in the xml file? need to force exporting of all banks (ignoring cached ones) when wavebundler version changes
// - make streaming be stored per platform? some banks may want to be streaming on PS3 but not on 360 for example

// BUGS:
// - can get bank with multiple presets if add waves after selecting a different one from the preset combo box

// When adding a new platform:
// - Add enum in WaveBankPane.h
// - Add PlatformSettings field to the schema in WaveBanks.txt
// - Add string to the pchPlatformLookup string array

using namespace Wb;

const UINT IDT_TIMER = 0;


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
WaveBankMdiWindow::WaveBankMdiWindow(SorceryView& view) :
	_view(view)
{
	HFONT defaultFont	= (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT lf;

	GetObject(defaultFont, sizeof(lf), &lf);
	lf.lfWeight = FW_BOLD;
	lf.lfHeight -= 2;

	_boldFont	= CreateFontIndirect(&lf);

	_bListChangeDeferredUpdateRequest = false;
	_bDeferredWriteAndPopulateRequest = false;

	// TODO: find a better place for this? would be better done once only at app startup (after the db is loaded)
	//ProjectSettings::Get().SetDomMonitorCallback(&view._app._db);
	
	// force once only update
	//ProjectSettings::Get().DomCallback(&_dom, &Xt::DomRecordChanged(RecordOf(_dom)), this);
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
WaveBankMdiWindow::~WaveBankMdiWindow()
{
	if (_boldFont != 0)
		DeleteObject(_boldFont);

	KillTimer(_hwnd, IDT_TIMER);

	_list = 0;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
bool WaveBankMdiWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!MdiWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle |= WS_EX_ACCEPTFILES;
	cs->dwExStyle	&= ~WS_EX_CLIENTEDGE;

	return true;
}

//-----------------------------------------------------------------------------
// Name: CreateControls
// Desc: 
//-----------------------------------------------------------------------------
bool WaveBankMdiWindow::CreateControls()
{
	//int curY=0;

	//------------------------------------------------
	// Bank section dividing line
	//------------------------------------------------
	_dividingLineBankTitle = CreateWindowEx(0, WC_STATIC, strBank.Chars(), WS_CHILD | WS_VISIBLE , 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)/*kWaveBank_BankDividingLineTitle*/0, 0, 0);
	_dividingLineBank = CreateWindowEx(/*WS_EX_CLIENTEDGE*/0, WC_STATIC, "", SS_SUNKEN | WS_CHILD | WS_VISIBLE , 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_BankDividingLine, 0, 0);

	//------------------------------------------------
	// platform combo box
	//------------------------------------------------
	_comboBoxPlatform = CreateWindowEx(0, _T("XTWINCOMBOBOX"), "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL /*| CBS_DISABLENOSCROLL | CBS_OWNERDRAWFIXED*/, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t) kWaveBank_BankFormatCombo, 0, 0);
	
	//------------------------------------------------
	// Bank name edit box
	//------------------------------------------------
	_editNameTitle = CreateWindowEx(0, WC_STATIC, "Name:",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)/*kWaveBank_EditBankTitle*/0, 0, 0);
	_editNameBox = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_READONLY /*| WS_DISABLED*/ /*| WS_BORDER*/, 
		0, 0, 0, 0,	*this, (HMENU)(intptr_t) kWaveBank_EditBankName, 0, 0);

	//------------------------------------------------
	// bank preset combo box
	//------------------------------------------------
	//_comboBoxBankPresetTitle = CreateWindowEx(0, WC_STATIC, "Format Preset:",  WS_CHILD | WS_VISIBLE /*| WS_BORDER */, 
	//	0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	//_comboBoxBankPreset = CreateWindowEx(0, _T("XTWINCOMBOBOX"), "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL /*| CBS_DISABLENOSCROLL | CBS_OWNERDRAWFIXED*/, 
	//	0, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_BankPresetCombo, 0, 0);

	//_comboBoxBankPresetModified = CreateWindowEx(0, WC_STATIC, "*",  WS_CHILD | WS_VISIBLE /*| WS_BORDER */, 
	//	0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	
	//------------------------------------------------
	// Bank streaming check box
	//------------------------------------------------
	_checkBoxStreaming = CreateWindowEx(0, WC_BUTTON, "Streaming", BS_AUTOCHECKBOX | BS_CHECKBOX /*| BS_LEFTTEXT*/  | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_StreamingTickBox, 0, 0);

	//------------------------------------------------
	// 'Add' button
	//------------------------------------------------
	_buttonAdd = CreateWindowEx(0, WC_BUTTON, _T("Add"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 
			0, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_Add, 0, 0);

	//------------------------------------------------
	// 'Remove' button
	//------------------------------------------------
	_buttonRemove = CreateWindowEx(0, WC_BUTTON, _T("Remove"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
			60, 0, 	0, 0, *this, (HMENU)(intptr_t)kWaveBank_Remove, 0, 0);

	//------------------------------------------------
	// 'Select All' button
	//------------------------------------------------
	_buttonSelectAll = CreateWindowEx(0, WC_BUTTON, _T("Select All"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
			60, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_SelectAllWaves, 0, 0);

	//------------------------------------------------
	// "Waves" section dividing line
	//------------------------------------------------
	_dividingLineWavesTitle = CreateWindowEx(0, WC_STATIC, strWaves.Chars(), WS_CHILD | WS_VISIBLE , 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)/*kWaveBank_WavesDividingLineTitle*/0, 0, 0);
	_dividingLineWaves = CreateWindowEx(/*WS_EX_CLIENTEDGE*/0, WC_STATIC, "", SS_SUNKEN | WS_CHILD | WS_VISIBLE , 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)/*kWaveBank_WavesDividingLineTitle*/0, 0, 0);

	//------------------------------------------------
	// "Wave Properties" section dividing line
	//------------------------------------------------
	_dividingLinePropertiesTitle = CreateWindowEx(0, WC_STATIC, strWaveProperties.Chars(), WS_CHILD | WS_VISIBLE , 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	_dividingLineProperties = CreateWindowEx(/*WS_EX_CLIENTEDGE*/0, WC_STATIC, "", SS_SUNKEN | WS_CHILD | WS_VISIBLE , 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	
	//------------------------------------------------
	// Wave format preset
	//------------------------------------------------
	_comboBoxWavePresetTitle = CreateWindowEx(0, WC_STATIC, "Format Preset:",  WS_CHILD | WS_VISIBLE | WS_DISABLED /*| WS_BORDER */, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	_comboBoxWavePreset = CreateWindowEx(0, WC_COMBOBOX/*_T("XTWINCOMBOBOX")*/, "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_DISABLED, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_WavePresetCombo, 0, 0);

	//------------------------------------------------
	// 'Open' button for preset
	//------------------------------------------------
	_buttonOpenPreset = CreateWindowEx(0, WC_BUTTON, _T("Open"), WS_CHILD | WS_VISIBLE | WS_DISABLED, 
			0, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_OpenPreset, 0, 0);

	//------------------------------------------------
	// wave Loop combo box
	//-----------------------------------------------
	_comboWaveLoopTitle = CreateWindowEx(0, WC_STATIC, "Loop:",  WS_CHILD | WS_VISIBLE | WS_DISABLED /*| WS_BORDER */, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	_comboWaveLoop = CreateWindowEx(0, WC_COMBOBOX/*_T("XTWINCOMBOBOX")*/, "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_DISABLED, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_WaveLoopCombo, 0, 0);

	//------------------------------------------------
	// wave Format combo box
	//-----------------------------------------------
	_comboWaveFormatTitle = CreateWindowEx(0, WC_STATIC, "Format:",  WS_CHILD | WS_VISIBLE | WS_DISABLED /*| WS_BORDER */, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	_comboWaveFormat = CreateWindowEx(0, WC_COMBOBOX/*_T("XTWINCOMBOBOX")*/, "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_DISABLED, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_WaveFormatCombo, 0, 0);

	//------------------------------------------------
	// wave sample rate edit box
	//------------------------------------------------
	_editWaveSampleRateTitle = CreateWindowEx(0, WC_STATIC, "Sample Rate:",  WS_CHILD | WS_VISIBLE | WS_DISABLED, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	_editWaveSampleRate = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_DISABLED, 
		0, 0, 0, 0,	*this, (HMENU)(intptr_t) kWaveBank_WaveSampleRateEditBox, 0, 0);

	//------------------------------------------------
	// wave number bits edit box
	//------------------------------------------------
	_editWaveNumBitsTitle = CreateWindowEx(0, WC_STATIC, "Bit Depth:",  WS_CHILD | WS_VISIBLE | WS_DISABLED, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	_editWaveNumBits = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_DISABLED, 
		0, 0, 0, 0,	*this, (HMENU)(intptr_t) kWaveBank_WaveNumBitsEditBox, 0, 0);
	
	//------------------------------------------------
	// quantize method combo box
	//-----------------------------------------------
	_comboWaveQuantizeMethodTitle = CreateWindowEx(0, WC_STATIC, "Quantize Method:",  WS_CHILD | WS_VISIBLE | WS_DISABLED , 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)0, 0, 0);
	_comboWaveQuantizeMethod = CreateWindowEx(0, WC_COMBOBOX, "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_DISABLED, 
		0, 0, 0, 0, *this, (HMENU)(intptr_t)kWaveBank_WaveQuantizeMethodCombo, 0, 0);

	//------------------------------------------------
	// List
	//------------------------------------------------
	if ((_list = CreateWindowExW(
			WS_EX_CLIENTEDGE /*| WS_EX_ACCEPTFILES*/, WC_LISTVIEWW, L"", LVS_REPORT | WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_SHOWSELALWAYS | LVS_SORTASCENDING, 
			0, 0, 
			0, 0, 
			*this, (HMENU)(intptr_t)kWaveBankForm_List, 0, 0
		)) == 0)
	{
		return false;
	}
	
	/*DragAcceptFiles(*this, true);*/

	ListView_SetExtendedListViewStyleEx(_list,
			LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER ,
			LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER 
		);

	{
		LVCOLUMN col = { 0 };

		col.mask	= LVCF_TEXT | LVCF_WIDTH;
		col.pszText	= _T("Name");
		col.cx		= 190;

		ListView_InsertColumn(_list, kWaveListColumn_Name, &col);
	}
	{
		LVCOLUMN col = { 0 };

		col.mask	= LVCF_TEXT | LVCF_WIDTH;
		col.pszText	= _T("Preset");
		col.cx		= 100;

		ListView_InsertColumn(_list, kWaveListColumn_Preset, &col);
	}
	{
		LVCOLUMN col = { 0 };

		col.mask	= LVCF_TEXT | LVCF_WIDTH;
		col.pszText	= _T("Format");
		col.cx		= 50;

		ListView_InsertColumn(_list, kWaveListColumn_Format, &col);
	}
	{
		LVCOLUMN col = { 0 };

		col.mask	= LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		col.fmt		= LVCFMT_CENTER;
		col.pszText	= _T("Frequency");
		col.cx		= 90;

		ListView_InsertColumn(_list, kWaveListColumn_Frequency, &col);
	}
	{
		LVCOLUMN col = { 0 };

		col.mask	= LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		col.fmt		= LVCFMT_CENTER;
		col.pszText	= _T("Channels");
		col.cx		= 60;

		ListView_InsertColumn(_list, kWaveListColumn_Channels, &col);
	}
	{
		LVCOLUMN col = { 0 };

		col.mask	= LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		col.fmt		= LVCFMT_CENTER;
		col.pszText	= _T("Bits");
		col.cx		= 55;

		ListView_InsertColumn(_list, kWaveListColumn_Bits, &col);
	}
	{
		LVCOLUMN col = { 0 };

		col.mask	= LVCF_TEXT | LVCF_WIDTH;
		col.pszText	= _T("Loop");
		col.cx		= 40;

		ListView_InsertColumn(_list, kWaveListColumn_Loop, &col);
	}
	{
		LVCOLUMN col = { 0 };

		col.mask	= LVCF_TEXT | LVCF_WIDTH;
		col.pszText	= _T("Quantize");
		col.cx		= 80;

		ListView_InsertColumn(_list, kWaveListColumn_QuantizeMethod, &col);
	}
	
	HWND hwnd = GetFirstChild(*this);

	while (hwnd != 0)
	{
		if (hwnd == _dividingLineWavesTitle || hwnd == _dividingLineBankTitle || hwnd == _dividingLinePropertiesTitle)
			SetWindowFont(hwnd, _boldFont, false);
		else
			SetWindowFont(hwnd, GetStockObject(DEFAULT_GUI_FONT), false);

		hwnd = GetNextSibling(hwnd);
	}

	{
	//LOGFONT lf = { 0 };
	//GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	//lstrcpy(lf.lfFaceName, TEXT("Arial Unicode MS"));
	////lf.lfHeight -= 1;
	////lf.lfWidth += 1;
	//HFONT font = CreateFontIndirect(&lf);
	//SetWindowFont(_list, font, FALSE); 
	}

	SetTimer(_hwnd, IDT_TIMER, 250, (TIMERPROC) NULL);     // 0.25 of a second

	Populate(true);

	UpdateWindowTitle();

	RegisterDialogWindow(this);

	return true;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
LRESULT WaveBankMdiWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_SETFOCUS:
			
			ProjectSettings::Get().SetMessagePrintCallback(MessageBoxPrintFn, &_view._app);
			ProjectSettings::Get().Update(_view._app._db);

			Populate();
			
		break;
		case WM_TIMER:
		{
			if (_bDeferredWriteAndPopulateRequest)
			{
				WriteToDom();
				Populate();
				_bDeferredWriteAndPopulateRequest = false;
			}

			if (_bListChangeDeferredUpdateRequest)
			{
				if (_list)
				{
					// populate wave properties if the list's selection or entries' values have changed
					if (ListView_GetSelectedCount(_list))
					{
						SetWindowRedraw(_list, TRUE);
						EnableWindow(_comboBoxWavePresetTitle, true);
						EnableWindow(_comboBoxWavePreset, true);
						EnableWindow(_comboWaveLoopTitle, true);
						EnableWindow(_comboWaveLoop, true);

						PopulateWavePresetComboBox();
						PopulateWaveLoopComboBox();
						PopulateWaveFormatComboBox();
						PopulateWaveQuantizeMethodComboBox();

						if (GetFocus() != _editWaveSampleRate)
							PopulateWaveSampleRateEditBox();
						if (GetFocus() != _editWaveNumBits)
							PopulateWaveNumBitsEditBox();

						// disable quantize option if no loops selected
						int iLoopVal = GetWaveMultipleProperty(eWavePropertyLoop);
						if (iLoopVal != kMultiValue_None && iLoopVal == 0)
						{
							EnableWindow(_comboWaveQuantizeMethodTitle, false);
							EnableWindow(_comboWaveQuantizeMethod, false);
						}
						else
						{
							EnableWindow(_comboWaveQuantizeMethodTitle, true);
							EnableWindow(_comboWaveQuantizeMethod, true);
						}

						// disable/enable wave properties depending on the preset
						// if the preset is <None> then we enable these wave properties
						int selectedIndex = (int)SendMessage(_comboBoxWavePreset, CB_GETCURSEL, 0, 0);
						int noneIndexOffset = (int)SendMessage(_comboBoxWavePreset, CB_FINDSTRINGEXACT, -1, (LPARAM)strPresetNone.Chars());
						if (selectedIndex != -1 && selectedIndex == noneIndexOffset)
						{
							EnableWindow(_comboWaveFormatTitle, true);
							EnableWindow(_comboWaveFormat, true);
							EnableWindow(_editWaveSampleRateTitle, true);
							EnableWindow(_editWaveSampleRate, true);
							EnableWindow(_editWaveNumBitsTitle, true);
							EnableWindow(_editWaveNumBits, true);
							EnableWindow(_buttonOpenPreset, false);
						}
						else
						{
							EnableWindow(_comboWaveFormatTitle, false);
							EnableWindow(_comboWaveFormat, false);
							EnableWindow(_editWaveSampleRateTitle, false);
							EnableWindow(_editWaveSampleRate, false);
							EnableWindow(_editWaveNumBitsTitle, false);
							EnableWindow(_editWaveNumBits, false);
							EnableWindow(_buttonOpenPreset, true);
						}
					}
				}

				_bListChangeDeferredUpdateRequest = false;
			}

		
			// disable all wave properties controls if no waves selected in the listview
			if (_list)
			{
				if (!ListView_GetSelectedCount(_list) && IsWindowEnabled(_comboBoxWavePreset))
				{
					SendMessage(_comboWaveLoop, CB_RESETCONTENT, 0, 0);
					SendMessage(_comboBoxWavePreset, CB_RESETCONTENT, 0, 0);
					SendMessage(_comboWaveFormat, CB_RESETCONTENT, 0, 0);
					SetWindowText(_editWaveSampleRate, "");
					SetWindowText(_editWaveNumBits, "");

					EnableWindow(_comboBoxWavePresetTitle, false);
					EnableWindow(_comboBoxWavePreset, false);
					EnableWindow(_comboWaveLoopTitle, false);
					EnableWindow(_comboWaveLoop, false);
					EnableWindow(_comboWaveFormatTitle, false);
					EnableWindow(_comboWaveFormat, false);
					EnableWindow(_editWaveSampleRateTitle, false);
					EnableWindow(_editWaveSampleRate, false);
					EnableWindow(_editWaveNumBitsTitle, false);
					EnableWindow(_editWaveNumBits, false);
					EnableWindow(_comboWaveQuantizeMethod, false);
					EnableWindow(_comboWaveQuantizeMethodTitle, false);
					EnableWindow(_buttonOpenPreset, false);
				}
			}
		}
		break;

		case WM_KEYDOWN:
		{
			/*int id		= LOWORD(wParam);
			int code	= HIWORD(wParam);

			char str[128];
			sprintf(str, "msg %x", msg);
			OutputDebugString(str);
			OutputDebugString("\n");*/

		}
		break;
		
		case WM_CREATE:
			// TODO: do this with a monitor?
			//ProjectSettings::Get().Update(&_view._app._db);

			if (CreateControls() == false)
				return -1;
		break;

		case WM_DESTROY:
			UnregisterDialogWindow(this);
		break;

		case WM_CLOSE:
			_view.Close();
		return 0;

		case WM_SIZE:
		{
			WinRect rcClient(*this);

			// position controls here

			//int listTop = 100;
			int curY = 4;
			int vertMargin = 8;
			int leftEdge = rcClient.left + vertMargin;
			int rightEdge = rcClient.right - vertMargin;
			int rightTabX = 90;
			int textYOffset = 3;
			SIZE txtSize;
			int tmpTxtWidth;
			HDC ic = CreateIC(_T("DISPLAY"), 0, 0, 0);

			// platform drop box
			SetWindowPos(_comboBoxPlatform, 0, rightEdge - 70, rcClient.top + curY, 70, 21 + 200, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += 4;

			// bank dividing line
			SelectObject(ic, _boldFont/*GetStockObject(DEFAULT_GUI_FONT)*/);
			GetTextExtentPoint32(ic, strBank.Chars(), strBank.Length(), &txtSize);
			tmpTxtWidth = txtSize.cx+vertMargin;
			SetWindowPos(_dividingLineBankTitle, 0, leftEdge, rcClient.top + curY, tmpTxtWidth, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += txtSize.cy>>1;
			SetWindowPos(_dividingLineBank, 0, leftEdge+tmpTxtWidth, rcClient.top + curY, rcClient.Width() - vertMargin*2 - tmpTxtWidth - vertMargin - 70, 2, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += 10;

			// bank name edit box
			SetWindowPos(_editNameTitle, 0, leftEdge, rcClient.top + curY + textYOffset, rightTabX/*50*/, 14, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_editNameBox, 0, leftEdge + rightTabX, rcClient.top + curY, 200, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += 26;

			// bank format preset combo box
			//SetWindowPos(_comboBoxBankPresetTitle, 0, leftEdge, rcClient.top + curY + textYOffset, rightTabX/*70*/, 14, SWP_NOACTIVATE | SWP_NOZORDER);
			//SetWindowPos(_comboBoxBankPreset, 0, leftEdge + rightTabX, rcClient.top + curY, 200, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			//SetWindowPos(_comboBoxBankPresetModified, 0, leftEdge + rightTabX + 200, rcClient.top + curY, 12, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			//curY += 26;

			// streaming check box
			SetWindowPos(_checkBoxStreaming, 0, leftEdge + rightTabX, rcClient.top + curY, 90, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += 26;

			// "Waves" dividing line
			GetTextExtentPoint32(ic, strWaves.Chars(), strWaves.Length(), &txtSize);
			tmpTxtWidth = txtSize.cx+vertMargin;
			SetWindowPos(_dividingLineWavesTitle, 0, leftEdge, rcClient.top + curY, tmpTxtWidth, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += txtSize.cy>>1;
			SetWindowPos(_dividingLineWaves, 0, leftEdge+tmpTxtWidth, rcClient.top + curY, rcClient.Width() - vertMargin*2 - tmpTxtWidth, 2, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += 10;

			// add and remove waves button
			SetWindowPos(_buttonAdd, 0, leftEdge, rcClient.top + curY, 60, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_buttonRemove, 0, leftEdge + 62, rcClient.top + curY, 60, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(_buttonSelectAll, 0, leftEdge + 62 + 62 + 16, rcClient.top + curY, 60, 21, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += 26;

			// "Properties" dividing line
			GetTextExtentPoint32(ic, strWaveProperties.Chars(), strWaveProperties.Length(), &txtSize);
			//int sectionHeight = 20 + 20 + 22 + 22 + 22 + 22 + 10 + (txtSize.cy>>1) + 8;
			int vertSpaceCombo = 22;
			int vertSpaceEditBox = 20;
			int sectionHeight = vertSpaceEditBox*2 + vertSpaceCombo*4 + 10 + (txtSize.cy>>1) + 8;
			int minListHeight = 60;

			// the current Y pos for the bottom section
			int curY2 = Max((int)rcClient.bottom-sectionHeight, (int)curY+minListHeight);

			// waves list
			SetWindowPos(_list, 0, leftEdge, rcClient.top + curY, rcClient.Width() - 16, curY2 - curY - 8, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += 250 + 13;

			int txtHeight = 14;
			int ctlHeight = 18 + 200;

			tmpTxtWidth = txtSize.cx+vertMargin;
			SetWindowPos(_dividingLinePropertiesTitle, 0, leftEdge, rcClient.top + curY2, tmpTxtWidth, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			curY2 += txtSize.cy>>1;
			SetWindowPos(_dividingLineProperties, 0, leftEdge + tmpTxtWidth, rcClient.top + curY2, rcClient.Width() - tmpTxtWidth - vertMargin*2, 2, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			curY2 += 10;

			// loop combo
			SetWindowPos(_comboWaveLoopTitle, 0, leftEdge, rcClient.top + curY2 + textYOffset, rightTabX, txtHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			SetWindowPos(_comboWaveLoop, 0, leftEdge + rightTabX, rcClient.top + curY2, 200, ctlHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			curY2 += vertSpaceCombo;

			// quantize method combo
			SetWindowPos(_comboWaveQuantizeMethodTitle, 0, leftEdge, rcClient.top + curY2 + textYOffset, rightTabX, txtHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			SetWindowPos(_comboWaveQuantizeMethod, 0, leftEdge + rightTabX, rcClient.top + curY2, 200, ctlHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			curY2 += vertSpaceCombo;
			
			// wave preset
			SetWindowPos(_comboBoxWavePresetTitle, 0, leftEdge, rcClient.top + curY2 + textYOffset, rightTabX, txtHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			SetWindowPos(_comboBoxWavePreset, 0, leftEdge + rightTabX, rcClient.top + curY2, 200, ctlHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			SetWindowPos(_buttonOpenPreset, 0, leftEdge + rightTabX + 200 + vertMargin/2, rcClient.top + curY2 + 2, 60, 18, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			curY2 += vertSpaceCombo;

			// wave format combo
			SetWindowPos(_comboWaveFormatTitle, 0, leftEdge, rcClient.top + curY2 + textYOffset, rightTabX, txtHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			SetWindowPos(_comboWaveFormat, 0, leftEdge + rightTabX, rcClient.top + curY2, 200, ctlHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			curY2 += vertSpaceCombo;

			// sample rate box
			SetWindowPos(_editWaveSampleRateTitle, 0, leftEdge, rcClient.top + curY2 + textYOffset, rightTabX, txtHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			SetWindowPos(_editWaveSampleRate, 0, leftEdge + rightTabX, rcClient.top + curY2, 200, ctlHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			curY2 += vertSpaceEditBox;

			// number of bits
			SetWindowPos(_editWaveNumBitsTitle, 0, leftEdge, rcClient.top + curY2 + textYOffset, rightTabX/*70*/, txtHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			SetWindowPos(_editWaveNumBits, 0, leftEdge + rightTabX, rcClient.top + curY2, 200, ctlHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
			curY2 += vertSpaceEditBox;
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);
			//SetBkColor(ps.hdc, 0x00000000);		// hard coded fun. USE RGB(r, g, b) instead
			SetBkColor(ps.hdc, GetSysColor(COLOR_BTNFACE));	// this works for me but not for nicks
			ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, 0, 0, 0);
			EndPaint(_hwnd, &ps);
		}
		return 0;

		//case WM_CTLCOLORSTATIC:
		//{
		//	int id		= LOWORD(wParam);
		//	// might have to switch based on the window id. some windows might not work being white
		//	//if (/*id != kWaveBank_WavesDividingLine &&*/ id != kWaveBank_BankDividingLine)
		//		//return (int)(void*)GetSysColorBrush(COLOR_WINDOW);
		//		return (int)(void*)GetSysColorBrush(COLOR_MENU);
		//}
		//break;

		case WM_COMMAND:
		{
			int	id		= LOWORD(wParam);
			int code	= HIWORD(wParam);

			if ((id == kWaveBank_Add) && (code == BN_CLICKED))
			{
				AddWavesFromFileSelector();
			}
			else if ((id == kWaveBank_Remove) && (code == BN_CLICKED))
			{
				DeleteSelectedWaves();
			}
			else if ((id == kWaveBank_SelectAllWaves) && (code == BN_CLICKED))
			{
				//LRESULT res = PostMessage(_hwnd, WM_NEXTDLGCTL, (WPARAM)_list, true);
				//LRESULT res = PostMessage(_hwnd, WM_NEXTDLGCTL, 0, 0);
				
				// this is not the proper way to do it as it doesnt set the default button etc 
				// (ie the black outline round the button stays on the Select All button)
				// and more importantly doesnt set the keyboard focus to the new button
				// so clicking select all and pressing the delete key doesnt work.
				// see here http://blogs.msdn.com/oldnewthing/archive/2004/08/02/205624.aspx
				// WM_NEXTDLGCTL is the way to do it but I couldnt get it to work
				SendMessage(_list, WM_SETFOCUS, 0, 0);
				ListView_SetItemState(_list, 0xffffffff, LVIS_SELECTED, LVIS_SELECTED);
			}
			else if ((id == kWaveBankForm_List) && (code == BN_CLICKED))
			{
				char str[128];
				sprintf_s(str, 128, "code %x", code);
				OutputDebugString(str);
				OutputDebugString("\n");
			}
			/*else if ((id == kWaveBank_BankPresetCombo) && (code == CBN_SETFOCUS))
			{
				PopulateBankPresetComboBox();
			}*/
			else if ((id == kWaveBank_BankFormatCombo) && (code == CBN_SELCHANGE))
			{
				int index = (int)SendMessage(_comboBoxPlatform, CB_GETCURSEL, 0, 0);

				int indexOld = Bank().PresetIndex();

				ProjectSettings::Get().SetPlatformCurr((ePlatform)index);

				int topIndex = ListView_GetTopIndex(_list);

				ListView_DeleteAllItems(_list);

				Bank().PopulateFromDom(AsRecord(_view._document->_dom));
				
				for (int i=0; i<Bank().NumWaves(); i++)
				{
					Wave* pWave = Bank().Waves(i);
					if (!pWave)
						continue;

					AddEntryToListView(pWave, i);
				}
			}
			// user has changed the bank "streaming" tick box
			else if ((id == kWaveBank_StreamingTickBox) && (code == BN_CLICKED))
			{
				bool ticked = (int)SendMessage(_checkBoxStreaming, BM_GETCHECK, 0, 0) == BST_CHECKED;
				Bank().SetStreaming(ticked);

				WriteToDom();
			}
			// user has clicked "open" preset
			else if ((id == kWaveBank_OpenPreset) && (code == BN_CLICKED))
			{
				int selectedIndex = (int)SendMessage(_comboBoxWavePreset, CB_GETCURSEL, 0, 0);
				if (selectedIndex)
				{
					FormatPreset* pPreset = PresetManager::Get().Preset(selectedIndex-1);
					if (pPreset)
					{
						Xt::DomRecord* pRecord = pPreset->GetRecord(_view._app._db);
						if (pRecord)
							_view._app._window.OpenRecord(pRecord);
					}
				}
			}
			else if ((id == kWaveBank_WavePresetCombo) && (code == CBN_SETFOCUS))
			{
				PopulateWavePresetComboBox();
			}
			// user has changed the "preset" combo box
			else if ((id == kWaveBank_WavePresetCombo) && (code == CBN_SELCHANGE))
			{
				int selectedIndex = (int)SendMessage(_comboBoxWavePreset, CB_GETCURSEL, 0, 0);
				int presetIndex = ePreset_None;
				int noneIndexOffset = (int)SendMessage(_comboBoxWavePreset, CB_FINDSTRINGEXACT, -1, (LPARAM)strPresetNone.Chars());
				
				if (noneIndexOffset == CB_ERR)
					noneIndexOffset = 0;

				if (selectedIndex >= noneIndexOffset)
				{
					selectedIndex -= noneIndexOffset;

					if (selectedIndex == 0)
						presetIndex = ePreset_None;
					else
					if (selectedIndex <= PresetManager::Get().Presets().Size() && selectedIndex > 0)
						presetIndex = selectedIndex-1;

					int position = -1;
					if (int numInList = ListView_GetSelectedCount(_list))
					{
						for(;;)
						{
							position = ListView_GetNextItem(_list, position, LVNI_SELECTED);
							
							if (!numInList-- || position == -1)
								break;

							int waveIndex = LookupWaveIndex(position);
							if (waveIndex != -1)
								Bank().Waves(waveIndex)->SetPreset(presetIndex);
						}
					}

					WriteToDom();
					Populate();
				}
			}
			// user has changed the "loop" combo box
			else if ((id == kWaveBank_WaveLoopCombo) && (code == CBN_SELCHANGE))
			{
				int selectedIndex = (int)SendMessage(_comboWaveLoop, CB_GETCURSEL, 0, 0);
				int startOffset = (int)SendMessage(_comboWaveLoop, CB_FINDSTRINGEXACT, -1, (LPARAM)"Yes");

				if (selectedIndex != -1 && startOffset != -1)
				{
					if (selectedIndex >= startOffset)
					{
						bool bLoop = (selectedIndex - startOffset == 0) ? true : false;

						int position = -1;
						if (int numInList = ListView_GetSelectedCount(_list))
						{
							for(;;)
							{
								position = ListView_GetNextItem(_list, position, LVNI_SELECTED);
								
								if (!numInList-- || position == -1)
									break;

								int waveIndex = LookupWaveIndex(position);
								if (waveIndex != -1)
									Bank().Waves(waveIndex)->SetLoop(bLoop);
							}
						}

						WriteToDom();
						Populate();
					}
				}
			}
			// user has changed the "quantize" combo box
			else if ((id == kWaveBank_WaveQuantizeMethodCombo) && (code == CBN_SELCHANGE))
			{
				int selectedIndex = (int)SendMessage(_comboWaveQuantizeMethod, CB_GETCURSEL, 0, 0);
				int startOffset = (int)SendMessage(_comboWaveQuantizeMethod, CB_FINDSTRINGEXACT, -1, (LPARAM)"Default");

				if (selectedIndex != -1 && startOffset != -1)
				{
					if (selectedIndex >= startOffset)
					{
						int position = -1;
						if (int numInList = ListView_GetSelectedCount(_list))
						{
							for(;;)
							{
								position = ListView_GetNextItem(_list, position, LVNI_SELECTED);
								
								if (!numInList-- || position == -1)
									break;

								int waveIndex = LookupWaveIndex(position);
								if (waveIndex != -1 && Bank().Waves(waveIndex)->Loop())
									Bank().Waves(waveIndex)->SetQuantizeMethod((eQuantizeMethod)(selectedIndex - startOffset - 1));
							}
						}

						WriteToDom();
						Populate();
					}
				}
			}
			// user has changed the "format" combo box
			else if ((id == kWaveBank_WaveFormatCombo) && (code == CBN_SELCHANGE))
			{
				int selectedIndex = (int)SendMessage(_comboWaveFormat, CB_GETCURSEL, 0, 0);
				int startOffset = (int)SendMessage(_comboWaveFormat, CB_FINDSTRINGEXACT, -1, (LPARAM)FormatName(0).Chars());

				if (selectedIndex != -1 && startOffset != -1)
				{
					if (selectedIndex >= startOffset)
					{
						int toSelect=-1;
						int position = -1;
						if (int numInList = ListView_GetSelectedCount(_list))
						{
							for(;;)
							{
								position = ListView_GetNextItem(_list, position, LVNI_SELECTED);
								
								if (!numInList-- || position == -1)
									break;
	
								int waveIndex = LookupWaveIndex(position);
								if (waveIndex != -1)
								{
									Wave* pWave = Bank().Waves(waveIndex);
									if (pWave && pWave->PresetIndex() == ePreset_None)
										pWave->Preset()->SetFormat((eWaveFormat)(selectedIndex - startOffset));
								}
							}
						}

						WriteToDom();
						Populate();
					}
				}
			}
			// user has edited the "sample rate" combo box
			else if ((id == kWaveBank_WaveSampleRateEditBox) && (code == EN_CHANGE))
			{
				if (GetFocus() == _editWaveSampleRate)
				{
					char pchCurTxt[32] = {0};
					GetWindowText(_editWaveSampleRate, pchCurTxt, 32);
					int iRate = strtol(pchCurTxt, 0, 10);
					int position = -1;

					if (int numInList = ListView_GetSelectedCount(_list))
					{
						for(;;)
						{
							position = ListView_GetNextItem(_list, position, LVNI_SELECTED);
							
							if (!numInList-- || position == -1)
								break;

							int waveIndex = LookupWaveIndex(position);
							if (waveIndex != -1)
								Bank().Waves(waveIndex)->Preset()->SetSampleRate(iRate);
						}
					}
					_bDeferredWriteAndPopulateRequest = true;
				}
			}
			// user has edited the "bit depth" combo box
			else if ((id == kWaveBank_WaveNumBitsEditBox) && (code == EN_CHANGE))
			{
				// only write changes if we are actually editing the box. 
				// otherwise we get a horrible cyclic thing going on
				if (GetFocus() == _editWaveNumBits)		
				{
					char pchCurTxt[32] = {0};
					GetWindowText(_editWaveNumBits, pchCurTxt, 32);
					int iBitDepth = strtol(pchCurTxt, 0, 10);
					int position = -1;

					if (int numInList = ListView_GetSelectedCount(_list))
					{
						for(;;)
						{
							position = ListView_GetNextItem(_list, position, LVNI_SELECTED);
							
							if (!numInList-- || position == -1)
								break;

							int waveIndex = LookupWaveIndex(position);
							if (waveIndex != -1)
								Bank().Waves(waveIndex)->Preset()->SetBitDepth(iBitDepth);
						}
					}

					_bDeferredWriteAndPopulateRequest = true;
				}
			}
			
		}
		break;

		/*case WM_NEXTDLGCTL:
			OutputDebugString("dlgctl\n");
			return 0;
		break;*/

		case WM_NOTIFY:
		{
			NMHDR* hdr = (NMHDR*)lParam;
			
			// respond to control notifications here

			if (hdr->idFrom == kWaveBankForm_List)
			{
				switch(hdr->code)
				{
					case LVN_KEYDOWN:
					{	
						// took this out because its inconsistent that it doesnt work after clicking on select all
						/*NMLVKEYDOWN* nmlKeyDown = (NMLVKEYDOWN*) lParam; 
						if (nmlKeyDown->wVKey == VK_DELETE)
						{
							DeleteSelectedWaves();
						}*/
					}
					break;
					case LVN_HOTTRACK:
					break;
					case LVN_ITEMCHANGED:
						//OutputDebugString("#");
						_bListChangeDeferredUpdateRequest = true;
					break;
					case NM_RELEASEDCAPTURE:
						//OutputDebugString("$ released capture\n");
					break;
					case NM_CLICK:
						//OutputDebugString("$ click\n");
					break;
					case LVN_ITEMACTIVATE:
						OutputDebugString("$ activate\n");
					break;
					default:
					{
						/*char str[128];
						sprintf_s(str, "code %x (%d)", hdr->code, LVN_FIRST - hdr->code);
						OutputDebugString(str);
						OutputDebugString("\n");*/
					}
					break;
				}
			}
		}
		break;

		case WM_DROPFILES:
			ProcessDroppedFiles((HDROP)wParam);
			AddFromWaveFileList();
		break;

		case WM_SETTEXT:
		{
			// if set text message comes through from tree view window then see if the name of the bank has been changed
			Xt::DomRecord* record = AsRecord(_view._document->_dom);
			if (record)
			{
				Util::C_String rawName = ExtractBankName((const char*)record->name);
				if (Bank().Name() != rawName)
				{
					Bank().Name() = rawName;
					SetWindowText(_editNameBox, Bank().Name().Chars());
				}
			}
		}
		break;

	    case DM_GETDEFID: return 0;
	    case DM_SETDEFID: return 0;

		default:
			//{
			//char str[128];
			//sprintf(str, "Unprocessed msg %x", msg);
			//OutputDebugString(str);
			//OutputDebugString("\n");
			//}
			break;
	}

	return MdiWindow::WndProc(msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: AddWavesFromFileSelector()
// Desc: gets filenames using the file selector and adds those files as waves
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::AddWavesFromFileSelector()
{
	static int filterIndex = 1;
	static bool bFirstTime = true;
	static Util::C_String strCurrentPath;
	
	//ProjectSettings::Get().Update(&_view._app._db);

	const int FNAME_BUF_SIZE = 8192;
	static TCHAR pchFileList[FNAME_BUF_SIZE] = "";
	memset(pchFileList, 0, FNAME_BUF_SIZE);
	
	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);

	// notes: If lpstrFile contains a path, that path is the initial directory
	// Otherwise, lpstrInitialDir specifies the initial directory
	OPENFILENAME ofn = { 0 };

	enum eDirectory
	{
		eLetWindowsDecidePath=0,
		eUseBankPath,
		eUseSampleRootPath,
		eUseLastRememberedPath,
	};

	eDirectory eDir=eLetWindowsDecidePath;
	Util::C_String strBankPath;

	if (Bank().NumWaves())
	{
		strBankPath = Bank().GetCommonPath().Chars();
		ofn.lpstrInitialDir = strBankPath.Chars();
		eDir = eUseBankPath;
	}
	
	if (!ofn.lpstrInitialDir)
	{
		if (bFirstTime)
		{
			// get initial dir from sample's root path
			ofn.lpstrInitialDir	= ProjectSettings::Get().SamplesRootPath().Chars();
			eDir = eUseSampleRootPath;
			//OutputDebugString("Initial dir set from sample's root path\n");
		}
		else
		{
			// get initial dir set from last remembered dir
			ofn.lpstrInitialDir	= strCurrentPath.Chars();
			eDir = eUseLastRememberedPath;
			//OutputDebugString("Initial dir set from last remembered dir\n");
		}
	}

	ofn.lStructSize			= sizeof(ofn);
	ofn.hwndOwner			= *this;
	ofn.lpstrFilter			= _T("All supported files (*.wav; *.adpcm; *.xma)\0*.wav;*.adpcm;*.xma\0Wav files (*.wav)\0*.wav\0Xma files (*.xma)\0*.xma\0Adpcm files (*.adpcm)\0*.adpcm\0All files (*.*)\0*.*\0");
	ofn.nFilterIndex		= filterIndex;
	ofn.lpstrFile			= pchFileList;
	ofn.nMaxFile			= FNAME_BUF_SIZE;				
	ofn.lpstrTitle			= _T("Add Wave Files...");
	ofn.Flags				= OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;

	if (GetOpenFileName(&ofn) != FALSE)
	{
		// count number of strings
		char* pChar = pchFileList;
		int numStrings=0;
		while (pChar < pchFileList + FNAME_BUF_SIZE)
		{
			pChar += strlen(pChar) + 1;
			numStrings++;
			if (!(*pChar))
				break;
		}

		_waveFilesToAdd.Clear();

		Util::C_String strPath;
		if (numStrings == 1)
		{
			// special case. they selected one file only
			_waveFilesToAdd.PushBack((char*)pchFileList);
			strPath = Util::ExtractDirectory(Util::C_String(pchFileList));
		}
		else
		{
			// if multiple files were selected first string is the path 
			// following strings are just the fname only - null terminated
			strPath = pchFileList;
			pChar = pchFileList;

			for (int i=0; i<numStrings-1; i++)
			{
				pChar += strlen(pChar) + 1;
				_waveFilesToAdd.PushBack(strPath + "\\" + pChar);
			}
		}
		
		// remember the path for next time if it has changed
		if (eDir != eUseBankPath || (eDir == eUseBankPath && strCurrentPath != ofn.lpstrInitialDir))
			strCurrentPath = strPath;

		AddFromWaveFileList();

		bFirstTime = false;
	}

	// put the working dir back to how it was. 
	// must do this otherwise it will break sorcery!
	SetCurrentDirectory(currentDir);
}

//-----------------------------------------------------------------------------
// Name: DeleteSelectedWaves()
// Desc: deletes the waves currently selected
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::DeleteSelectedWaves()
{
	if (Xt::DomArray* waves = GetWaves())
	{
		if (ListView_GetSelectedCount(_list))
		{
			int position = -1;

			for(;;)
			{
				if ((position = ListView_GetNextItem(_list, -1, LVNI_SELECTED)) == -1)
					break;

				LVITEM item = { 0 };
				char pchText[255] = { 0 };

				item.mask		= LVIF_PARAM;
				item.iItem		= (int)position;
				item.iSubItem	= 0;
				item.lParam		= -1;
				
				ListView_GetItem(_list, &item);

				if (item.lParam != -1)
				{
					ListView_DeleteItem(_list, position);

					for (int i=0; i<Bank().NumWaves(); i++)
					{
						if (Bank().Waves(i)->Id() == item.lParam)
						{
							Bank().DestroyWave(i);
							break;
						}
					}
				}

				// delete from dom
				for (Xt::DomArray::IndexIterator it(waves); it; ++it)
				{
					Xt::DomBlock* block = Xt::AsBlock(it);
					if (!block)
						continue;
					if (Xt::ParseId(AsText(At(block, kXtField_WaveId))) != item.lParam)
						continue;
					Xt::Remove(waves, it.index);
					break;
				}
			}

			WriteToDom();
		}
	}
}

//-----------------------------------------------------------------------------
// Name: ProcessDroppedFiles()
// Desc: add names of dragged files to the list of files to be added to the bank
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::ProcessDroppedFiles(HDROP hDrop)
{
	unsigned int fileCount = 0, filenameLength = 0;
	char *name = NULL;

	// Find out how many files were dropped:
	fileCount = DragQueryFile(hDrop, 0xffffffff, NULL, 0);

	_waveFilesToAdd.Clear();

	// Cycle through the Files and Convert each in turn
	for(unsigned int i = 0; i < fileCount; i++)
	{
		filenameLength = DragQueryFile(hDrop, i, NULL, 0); // Length of this filename
		name = (char*)malloc(filenameLength + 1); if (name == NULL) continue;
		memset(name, 0, filenameLength + 1);

		DragQueryFile(hDrop, i, name, filenameLength + 1); // Get filename

		#ifdef _DEBUG
		OutputDebugString("Adding file \"");
		OutputDebugString(name);
		OutputDebugString("\"\n");
		#endif

		_waveFilesToAdd.PushBack(name);

		free(name);
		name = NULL;
	}

	DragFinish(hDrop); // Deallocate files
}

//-----------------------------------------------------------------------------
// Name: SetListViewTextSrcDst()
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::SetListViewTextSrcDst(int index, int columnId, int valIn, int valOut)
{
	LV_ITEMW lvi;
	WCHAR wcs[128] = {0};

	lvi.iSubItem = columnId;
	
	if (valIn == valOut)
		swprintf(wcs, 128, L"%d", valIn);
	else					// in  ->   out
	{
		if (valIn == -1)
			swprintf(wcs, 128, L"?  \x0336> %d", valOut);
		else
			swprintf(wcs, 128, L"%d  \x0336> %d", valIn, valOut);
	}

	lvi.pszText = wcs;

	SendMessage(_list, LVM_SETITEMTEXTW, (WPARAM)index, (LPARAM)(LV_ITEMW *)&lvi);
}

//-----------------------------------------------------------------------------
// Name: AddEntryToListView()
// Desc: Add an entry to the listview given a Wave*
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::AddEntryToListView(Wave* pWave, int index)
{		
	LVITEM item = { 0 };

	item.mask		= LVIF_TEXT | LVIF_PARAM;
	item.iItem		= index;
	item.iSubItem	= 0;
	item.lParam		= pWave->Id();
	Util::C_String32 str32 = pWave->FileStatus() == Wave::eNotFoundOnDisk ? Util::C_String().Printf("?%s?", pWave->Name().Chars()) : pWave->Name();
	item.pszText	= (LPSTR)str32.Chars();

	int pos = ListView_InsertItem(_list, &item);

	// add to the listview
	if (pos != -1)
	{
		ListView_SetItemText(_list, pos, kWaveListColumn_Channels, (LPSTR) (pWave->Channels() == -1) ? "?" : (const char*)Util::C_String().Printf("%d", pWave->Channels()).Chars());

		ListView_SetItemText(_list, pos, kWaveListColumn_Preset, (LPSTR)(const char*)pWave->Preset()->Name().Chars());
		ListView_SetItemText(_list, pos, kWaveListColumn_Format, (LPSTR)(const char*)FormatName(pWave->Preset()->Format()).Chars());

		SetListViewTextSrcDst(pos, kWaveListColumn_Frequency, pWave->SampleRate(), pWave->Preset()->SampleRate());
		SetListViewTextSrcDst(pos, kWaveListColumn_Bits, pWave->BitDepth(), pWave->Preset()->BitDepth());

		ListView_SetItemText(_list, pos, kWaveListColumn_Loop, pWave->Loop() ? "Yes" : "No");

		if (pWave->Loop())
		{
			Util::C_String strQuantize;
			if (pWave->QuantizeMethod() == eQuantizeDefault)
				strQuantize = Util::C_String().Printf("[%s]", ProjectSettings::Get().DefaultQuantizeMethod()==eQuantizeResample ? "Resample" : "Insert Delete");
			else
				strQuantize = pWave->QuantizeMethod() == eQuantizeResample ? "Resample" : "Insert Delete";

			ListView_SetItemText(_list, pos, kWaveListColumn_QuantizeMethod, (LPSTR)(const char*)strQuantize.Chars());
		}
	}
}

//-----------------------------------------------------------------------------
// Name: AddFromWaveFileList()
// Desc: Add waves to the wavebank from a previously built list of wave filenames
//       Note: this does not erase the wave list. It can be used to add waves to 
//       existing wavebank.
// TODO: make this function on an async thread with a progress bar? probably OTT
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::AddFromWaveFileList()
{
	if (_waveFilesToAdd.Empty())
		return;

	SetWindowRedraw(_list, FALSE);

	int topIndex = ListView_GetTopIndex(_list);

	Util::C_String strMessage;

	bool bWereErrors = false;

	for(int i = 0; i < _waveFilesToAdd.Size(); i++)
	{
		LVITEM item = { 0 };
		Util::C_String rawFilename = Util::ExtractFilename(_waveFilesToAdd[i]);
		Wave* pWave = Bank().Lookup(rawFilename);

		if (pWave)
			continue;	// dont add waves if they have the same filename

		WaveFileInfo info;

		info.strFilename = _waveFilesToAdd[i];

		eQueryWaveFileResult eQueryResult = eQuerySuccess;
		eQueryResult = QueryWaveFile(info);

		if (eQueryResult == eQuerySuccess)
		{
			// add to our wave list
			Wave *pWave = Bank().CreateWave();
			if (!pWave)
			{
				strMessage +=  Util::C_String().Printf("Could not create bank wave for %s...\n", info.strFilename);
				bWereErrors = true;
				continue;
			}

			pWave->Path() = Util::ExtractDirectory(_waveFilesToAdd[i]);
			pWave->Filename() = Util::ExtractFilename(_waveFilesToAdd[i]);
			pWave->SetBitDepth(info.bitDepth);
			pWave->SetLoop(info.bLoop ? true : false);
			pWave->SetChannels(info.nChannels);
			pWave->SetSampleRate(info.sampleRate);

			if (Bank().AddWave(pWave) == false)
			{
				strMessage +=  Util::C_String().Printf("Could not add wave %s...\n", info.strFilename.Chars());
				bWereErrors = true;
				delete pWave;
				continue;
			}

			// add to the windows list view form thing
			AddEntryToListView(pWave, i);
		}
		else
		{
			strMessage +=  Util::C_String().Printf("Could not get wav file info for %s... Reason: %s\n", info.strFilename.Chars(), LookupQueryWavResultString(eQueryResult));

			bWereErrors = true;
		}
	}

	if (bWereErrors)
		MessageBox(_view._app._window, _T(strMessage.Chars()), _T("Hey"), MB_OK);

	//ListView_SortItems(_list, LocFormCompare, &ctx);

	ListView_EnsureVisible(_list, topIndex, TRUE);

	SetWindowRedraw(_list, TRUE);

	WriteToDom();
}

//-----------------------------------------------------------------------------
// Name: LookupWaveIndex()
// Desc: 
//-----------------------------------------------------------------------------
int WaveBankMdiWindow::LookupWaveIndex(int listViewIndex)
{
	LVITEM item = { 0 };
	char pchText[255] = { 0 };

	item.mask		= LVIF_PARAM;
	item.iItem		= listViewIndex;
	item.iSubItem	= 0;
	item.lParam		= -1;
	
	ListView_GetItem(_list, &item);

	if (item.lParam == -1)
		return -1;

	return Bank().LookupIndex((int)item.lParam);
}

//-----------------------------------------------------------------------------
// Name: Populate()
// Desc: From the dom populate the waves list, list view and all controls.
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::Populate(bool bInital)
{
	PopulatePresetLists();
	PopulateWaveBankAndListViewFromDom(bInital);
	PopulatePlatformComboBox();

	// streaming check box
	SendMessage(_checkBoxStreaming, BM_SETCHECK, Bank().IsStreaming() ? BST_CHECKED : BST_UNCHECKED, 0);
}

//-----------------------------------------------------------------------------
// Name: PopulateFromDom()
// Desc: Query the record's dom structure to populate the wavebank and the
//       waves listview.
//       Note: this erases the wavebank and the listview before populating.
// TODO: call this when preset has been modified
// note from cakey: to detect when a preset has changed i can store an array of
// monitors for the presets. this triggers a callback when the presets have 
// changed.
// i could just re-populate the waves every time the window regains focus but
// it is more bulletproof to do the monitor approach.
// note: extra logic will need to be added if i am planning on modifying
// the presets from the wavebank pane
//-----------------------------------------------------------------------------
bool WaveBankMdiWindow::PopulateWaveBankAndListViewFromDom(bool bRescanWavFiles)
{
	Xt::DomArray* waves = GetWaves();

	if (!waves)
		return false;

	SetWindowRedraw(_list, FALSE);

	// store a list of selected items so we can reselct them on list population
	Ut::Vector<int> listReselect;

	int position = -1;
	for(;;)
	{
		position = ListView_GetNextItem(_list, position, LVNI_SELECTED);
		
		if (position == -1)
			break;

		listReselect.PushBack(position);
	}

	int topIndex = ListView_GetTopIndex(_list);

	ListView_DeleteAllItems(_list);

	Bank().PopulateFromDom(AsRecord(_view._document->_dom));
	
	// do we need to rescan wave files?

	// TODO:
	// only rescan wave files when opening the window of an existing bank DONE
	// if couldnt lookup wave info such as channels etc dont write -1s to the dom. DONE

	// TODO: after wav files have been rescanned Populate() is being called from SETFOCUS msg which is setting the listview from the dom
	// and reading out the old values. if could not find a file it is overwriting the -1s for samplerate etc.
	// just dont update from the dom the fields which have -1 in the Bank() ?

	// TODO: when rescanned wav files if files havent changed will still mark record as modified

	if (bRescanWavFiles)
	{
		Bank().RescanWavFiles();

		// write updated info back to dom
		Bank().WriteToDom(AsRecord(_view._document->_dom));

		if (!Bank().FoundAllWavFiles())
			ListView_SetTextColor(_list, RGB(255, 0, 0));
	}

	for (int i=0; i<Bank().NumWaves(); i++)
	{
		Wave* pWave = Bank().Waves(i);
		if (!pWave)
			continue;

		AddEntryToListView(pWave, i);
	}

	Util::C_String32 strName32 = Bank().Name();
	SetWindowText(_editNameBox, strName32);

	// reselect previously selected items
	for (int i=0; i<listReselect.Size(); i++)
		ListView_SetItemState(_list, listReselect[i], LVIS_SELECTED, LVIS_SELECTED);

	if (listReselect.Empty())
	{
		int topIndex = ListView_GetTopIndex(_list);
		ListView_EnsureVisible(_list, topIndex, FALSE);
	}
	else
	{
		ListView_EnsureVisible(_list, ListView_GetItemCount(_list)-1, FALSE);
		ListView_EnsureVisible(_list, topIndex, FALSE);
	}

	SetWindowRedraw(_list, TRUE);

	return true;
}

//-----------------------------------------------------------------------------
// Name: PopulatePlatformComboBox()
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::PopulatePlatformComboBox()
{
	SetWindowRedraw(_list, FALSE);

	SendMessage(_comboBoxPlatform, CB_RESETCONTENT, 0, 0);

	for (int i=0; i<NumPlatforms; i++)
	{
		SendMessage(_comboBoxPlatform, CB_ADDSTRING, 0, (LPARAM)PlatformName(i).Chars());
	}

	// TODO: set current platform here 
	SendMessage(_comboBoxPlatform, CB_SETCURSEL, 0, 0);
	//SendMessage(_comboBoxPlatform, CB_SETDROPPEDWIDTH, Max(4 + width + 4, 300), 0);

	SetWindowRedraw(_list, TRUE);
}

//-----------------------------------------------------------------------------
// Name: PopulatePresetLists()
// Desc: query the db for the presets and build a list of them ready for
//	     populating the preset combo box
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::PopulatePresetLists()
{
	//SetWindowRedraw(_list, FALSE);

	PresetManager::Get().PopulateFromDom(_view._app._db);

	//SetWindowRedraw(_list, TRUE);
}

//-----------------------------------------------------------------------------
// Name: PopulatePresetComboBox()
// Desc: populate the audio format preset combo from the presets in the database
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::PopulatePresetComboBox(HWND hCombo, bool bAddBlankEntry)
{
	//SetWindowRedraw(_list, FALSE);
	SetWindowRedraw(hCombo, FALSE);

	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
	if (bAddBlankEntry)
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T(""));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T(strPresetNone.Chars()));

	int width = 200;//0;

	for (int i=0; i<PresetManager::Get().Presets().Size(); i++)
	{
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)PresetManager::Get().Presets()[i].Name().Chars()/*it->label*/);
		//int addIndex = (int)SendMessage(_comboBoxBankPreset, CB_buttonAddSTRING, 0, (LPARAM)_T("String")/*it->label*/);

		//if (addIndex >= 0)
		//	SendMessage(_comboBoxBankPreset, CB_SETITEMDATA, addIndex, 0/*it->id*/);

		//WinSize sz;
		//GetTextExtentPoint32(ic, it->label, it->label.Length(), &sz);

		//width = Max(width, (int)sz.cx);
	}

	/*int packWidth = 0;

	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&Form()->Db()->packs); it; ++it)
	{
		XtString packName = "[" + it->name + "]";

		WinSize szPack;
		GetTextExtentPoint32(ic, packName, packName.Length(), &szPack);

		packWidth = Max(packWidth, (int)szPack.cx);
	}*/

	SetWindowRedraw(hCombo, TRUE);

	SendMessage(hCombo, CB_SETCURSEL, Min(PresetManager::Get().Presets().Size(), 1), 0);
	SendMessage(hCombo, CB_SETDROPPEDWIDTH, 200, 0);

	//if (Bank().presetIndex == kPresetNone)
	//	SendMessage(hCombo, CB_SETCURSEL, 0, 0);	// <None> preset
	//else
	//	SendMessage(hCombo, CB_SETCURSEL, Bank().presetIndex+1, 0);

	//SetWindowRedraw(_list, TRUE);
}

//-----------------------------------------------------------------------------
// Name: GetWaveMultipleProperty()
// Desc: get property of multiply selected listview items
//-----------------------------------------------------------------------------
S32 WaveBankMdiWindow::GetWaveMultipleProperty(eWaveProperty eProperty)
{
	S32 multiValue = kMultiValue_None;
	int position = -1;

	if (int numInList = ListView_GetSelectedCount(_list))
	{
		for(;;)
		{
			position = ListView_GetNextItem(_list, position, LVNI_SELECTED);
			
			if (!numInList-- || position == -1)
				break;

			int waveIndex = LookupWaveIndex(position);
			if (waveIndex != -1)
			{
				Wave* pWave = Bank().Waves(waveIndex);
				if (!pWave)
					continue;

				U32 curValue = 0;
				if (eProperty == eWavePropertyLoop)
					curValue = (U32)pWave->Loop();
				else
				if (eProperty == eWavePropertyQuantizeMethod)
					curValue = (U32)pWave->QuantizeMethod();
				else
				if (eProperty == eWavePropertyPreset)
				{
					if (pWave->PresetIndex() == ePreset_InheritFromBank)
						curValue = ePreset_InheritFromBank;
					else
					if (pWave->PresetIndex() == ePreset_None)
						curValue = ePreset_None;
					else
						curValue = pWave->PresetIndex() + 1;
				}
				else
				if (eProperty == eWavePropertyFormat)
					curValue = (U32)pWave->Preset()->Format();
				else
				if (eProperty == eWavePropertySampleRate)
					curValue = (U32)pWave->Preset()->SampleRate();
				else
				if (eProperty == eWavePropertyNumBits)
					curValue = (U32)pWave->Preset()->BitDepth();

				if (multiValue == kMultiValue_None)
				{
					multiValue = curValue;
				}
				else
				if (curValue != multiValue)
				{
					multiValue = kMultiValue_MixedValues;
					break;
				}
			}
		}
	}

	return multiValue;
}


//-----------------------------------------------------------------------------
// Name: PopulateQuantizeMethodComboBox()
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::PopulateWaveQuantizeMethodComboBox()
{
	SetWindowRedraw(_comboWaveQuantizeMethod, FALSE);

	S32 multiValue = GetWaveMultipleProperty(eWavePropertyQuantizeMethod);

	if (multiValue != kMultiValue_None)
	{
		SendMessage(_comboWaveQuantizeMethod, CB_RESETCONTENT, 0, 0);

		// were quantize values mixed? if so add a blank combo entry
		if (multiValue == kMultiValue_MixedValues)
			SendMessage(_comboWaveQuantizeMethod, CB_ADDSTRING, 0, (LPARAM)"");

		SendMessage(_comboWaveQuantizeMethod, CB_ADDSTRING, 0, (LPARAM)"Default");
		SendMessage(_comboWaveQuantizeMethod, CB_ADDSTRING, 0, (LPARAM)"Resample");
		SendMessage(_comboWaveQuantizeMethod, CB_ADDSTRING, 0, (LPARAM)"Insert or Delete Samples");

		SetWindowRedraw(_comboWaveQuantizeMethod, TRUE);

		int toSelect;
		if (multiValue == kMultiValue_MixedValues)		
			toSelect = 0;
		else
		//if (multiValue)
		{
			if (multiValue == eQuantizeDefault)
				toSelect = 0;
			else
				toSelect = multiValue+1;
		}
		//else
		//	toSelect = 1;

		SendMessage(_comboWaveQuantizeMethod, CB_SETCURSEL, toSelect, 0);	
	}

	SetWindowRedraw(_comboWaveQuantizeMethod, TRUE);
}

//-----------------------------------------------------------------------------
// Name: PopulateWaveLoopComboBox()
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::PopulateWaveLoopComboBox()
{
	SetWindowRedraw(_comboWaveLoop, FALSE);

	S32 multiValue = GetWaveMultipleProperty(eWavePropertyLoop);

	if (multiValue != kMultiValue_None)
	{
		SendMessage(_comboWaveLoop, CB_RESETCONTENT, 0, 0);

		// were loop values mixed? if so add a blank combo entry
		if (multiValue == kMultiValue_MixedValues)
			SendMessage(_comboWaveLoop, CB_ADDSTRING, 0, (LPARAM)"");

		SendMessage(_comboWaveLoop, CB_ADDSTRING, 0, (LPARAM)"Yes");
		SendMessage(_comboWaveLoop, CB_ADDSTRING, 0, (LPARAM)"No");

		SetWindowRedraw(_comboWaveLoop, TRUE);

		int toSelect;
		if (multiValue == kMultiValue_MixedValues)		
			toSelect = 0;
		else
		if (multiValue)
			toSelect = 0;
		else
			toSelect = 1;

		SendMessage(_comboWaveLoop, CB_SETCURSEL, toSelect, 0);	
	}

	SetWindowRedraw(_comboWaveLoop, TRUE);
}

//-----------------------------------------------------------------------------
// Name: PopulateWaveFormatComboBox()
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::PopulateWaveFormatComboBox()
{
	SetWindowRedraw(_comboWaveFormat, FALSE);

	S32 multiValue = GetWaveMultipleProperty(eWavePropertyFormat);

	if (multiValue != kMultiValue_None)
	{
		SendMessage(_comboWaveFormat, CB_RESETCONTENT, 0, 0);

		// were loop values mixed? if so add a blank combo entry
		if (multiValue == kMultiValue_MixedValues)
			SendMessage(_comboWaveFormat, CB_ADDSTRING, 0, (LPARAM)"");

		for (int i=0; i<NumWaveFormats; i++)
			SendMessage(_comboWaveFormat, CB_ADDSTRING, 0, (LPARAM)FormatName(i).Chars());

		SetWindowRedraw(_comboWaveFormat, TRUE);

		int toSelect;
		if (multiValue == kMultiValue_MixedValues)		
			toSelect = 0;
		else
			toSelect = multiValue;

		SendMessage(_comboWaveFormat, CB_SETCURSEL, toSelect, 0);	
	}

	SetWindowRedraw(_comboWaveFormat, TRUE);
}

//-----------------------------------------------------------------------------
// Name: PopulateWavePresetComboBox()
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::PopulateWavePresetComboBox()
{
	int position = -1;

	S32 multiPreset = GetWaveMultipleProperty(eWavePropertyPreset);

	PopulatePresetLists();
	PopulatePresetComboBox(_comboBoxWavePreset, multiPreset == kMultiValue_MixedValues ? true : false);

	if (multiPreset != kMultiValue_None)
	{
		if (multiPreset == kMultiValue_MixedValues)	// wave's presets were mixed
		{
			SendMessage(_comboBoxWavePreset, CB_SETCURSEL, 0, 0);	
		}
		else
		{
			if (multiPreset == ePreset_InheritFromBank)
				SendMessage(_comboBoxWavePreset, CB_SETCURSEL, Bank().PresetIndex() + 1, 0);	// bank preset
			else
			if (multiPreset == ePreset_None)
				SendMessage(_comboBoxWavePreset, CB_SETCURSEL, 0, 0);				// entry 0 in the combo is for the custom/none preset
			else
				SendMessage(_comboBoxWavePreset, CB_SETCURSEL, multiPreset, 0);	
		}
	}
}

//-----------------------------------------------------------------------------
// Name: PopulateWaveSampleRateEditBox()
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::PopulateWaveSampleRateEditBox()
{
	//SetWindowRedraw(_editWaveSampleRate, FALSE);

	S32 multiValue = GetWaveMultipleProperty(eWavePropertySampleRate);

	if (multiValue != kMultiValue_None)
	{
		if (multiValue == kMultiValue_MixedValues)
			SetWindowText(_editWaveSampleRate, "");
		else
			SetWindowText(_editWaveSampleRate, Util::C_String().Printf("%d", multiValue).Chars());
	}

	//SetWindowRedraw(_editWaveSampleRate, TRUE);
}

//-----------------------------------------------------------------------------
// Name: PopulateWaveNumBitsEditBox()
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::PopulateWaveNumBitsEditBox()
{
	//SetWindowRedraw(_editWaveNumBits, FALSE);

	S32 multiValue = GetWaveMultipleProperty(eWavePropertyNumBits);

	if (multiValue != kMultiValue_None)
	{
		if (multiValue == kMultiValue_MixedValues)
			SetWindowText(_editWaveNumBits, "");
		else
			SetWindowText(_editWaveNumBits, Util::C_String().Printf("%d", multiValue).Chars());

	}

	//SetWindowRedraw(_editWaveNumBits, TRUE);
}

//-----------------------------------------------------------------------------
// Name: WriteToDom()
// Desc: write the wavebank's details to the Dom hierarchy so it will get
// written to disk.
//-----------------------------------------------------------------------------
bool WaveBankMdiWindow::WriteToDom()
{
	// cakey's example code to write to the Dom by hand
	/*if (Xt::DomRecord* record = AsRecord(_view._document._dom))
	{
		Xt::Set(&record->root, "Hello", XtString("Sam"));

		Xt::DomBlock* block = AcquireBlock(&record->root, "MyBlock");

		Set(block, "Fish", XtString("Cat"));

		Xt::DomArray* arr = AcquireArray(&record->root, "MyArray");

		for(int i = 0; i < 3; i++)
		{
			Xt::DomBlock* mySubBlock = Xt::Append(arr, new Xt::DomBlock);

			Set(mySubBlock, "Val", XtString("Hi"));
		}
	}*/

	SetWindowRedraw(_list, FALSE);

	bool bOk = Bank().WriteToDom(AsRecord(_view._document->_dom));

	SetWindowRedraw(_list, TRUE);

	return bOk;
}

//-----------------------------------------------------------------------------
// Name: UpdateWindowTitle()
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::UpdateWindowTitle()
{
	Xt::DomRecord* record = AsRecord(_view._document->_dom);

	XtString title;

	if (record)
		title = record->name + "." + record->root.type;
	else
		title = "(unnamed)";

	if (record->dirty)
		title = title + "*";

	SetWindowText(*this, title);
}


//-----------------------------------------------------------------------------
// Name: GetWaves()
// Desc: 
//-----------------------------------------------------------------------------
Xt::DomArray* WaveBankMdiWindow::GetWaves()
{
	if (Xt::DomRecord* record = AsRecord(_view._document->_dom))
		return Xt::AcquireArray(&record->root.data, "waves");

	return 0;
}

//-----------------------------------------------------------------------------
// Name: AdjustWindowSize
// Desc: adjust the wavebank pane window to fit
//-----------------------------------------------------------------------------
void WaveBankMdiWindow::AdjustWindowSize()
{
	// cakey said dont do this as it breaks the MDI stuff
	//SendMessage (_hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);

	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof(wp);

	// The GetWindowPlacement function retrieves the show state and the restored, minimized, and maximized positions of the specified window
	if (GetWindowPlacement(*this, &wp) == FALSE)
		return;

	// just set the window size to 800x600
	wp.rcNormalPosition.right = wp.rcNormalPosition.left + 700;
	wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + 600;
	
	// The SetWindowPlacement function sets the show state and the restored, minimized, and maximized positions of the specified window
	SetWindowPlacement(*this, &wp);
}
