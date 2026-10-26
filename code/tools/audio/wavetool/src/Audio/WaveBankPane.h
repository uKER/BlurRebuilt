#ifndef WaveBankMgrPANE_H
#define WaveBankMgrPANE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\mercurycore.h>
#include <core\containers.h>
#include "WaveBundlerDll.h"
#include "WaveBankMgr.h"

extern void BuildAllBanks(SorceryApp& app);
extern void BuildCurrentBank(SorceryApp& app);
extern void BuildBanksSubset(SorceryApp& app, const char* pchFilter);

extern void MessageBoxPrintFn(int msgType, Util::C_String& str, void* pContext);

using namespace Wb;

enum eWaveProperty
{
	eWavePropertyLoop=0,
	eWavePropertyPreset,
	eWavePropertyFormat,
	eWavePropertyNumBits,
	eWavePropertySampleRate,
	eWavePropertyQuantizeMethod,
};

//-----------------------------------------------------------------------------
// Name: WaveBankMdiWindow
//-----------------------------------------------------------------------------
class WaveBankMdiWindow : public MdiWindow
{
public:
	WaveBankMdiWindow(class SorceryView& view);
	virtual ~WaveBankMdiWindow();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void AdjustWindowSize();

public:
	void UpdateWindowTitle();

	void ProcessDroppedFiles(HDROP hDrop);		
	void DeleteSelectedWaves();
	void AddWavesFromFileSelector();
	void AddFromWaveFileList();
	void AddEntryToListView(Wave* pWave, int index);

	bool WriteToDom();
	bool PopulateWaveBankAndListViewFromDom(bool bRescanWavFiles = false);
	Xt::DomArray* GetWaves();
	
	void PopulatePlatformComboBox();

	void PopulatePresetLists();
	void PopulatePresetComboBox(HWND hCombo, bool bAddBlankEntry=false);				// populate a combo box from the list of presets in the db
	void PopulateWavePresetComboBox();
	void PopulateWaveLoopComboBox();
	void PopulateWaveQuantizeMethodComboBox();
	void PopulateWaveFormatComboBox();
	void PopulateWaveSampleRateEditBox();
	void PopulateWaveNumBitsEditBox();

	void Populate(bool bInitial = false);

	S32 GetWaveMultipleProperty(eWaveProperty eProperty);

	bool CreateControls();

	void SetListViewTextSrcDst(int index, int columnId, int valIn, int valOut);

	int LookupWaveIndex(int listViewIndex);	// get wave index from listview index

	WaveBankMgr& Bank() { return _WaveBankMgr; }
public:
	inline SorceryView& View()
	{
		return _view;
	}

private:

	HWND _dividingLineBank;
	HWND _dividingLineBankTitle;
	HWND _comboBoxPlatform;
	//HWND _comboBoxPlatformTitle;
	HWND _editNameBox;
	HWND _editNameTitle;
	HWND _checkBoxStreaming;
	HWND _comboBoxBankPresetTitle;
	HWND _comboBoxBankPreset;
	HWND _comboBoxBankPresetModified;
		
	HWND _dividingLineWaves;
	HWND _dividingLineWavesTitle;
	HWND _buttonAdd;
	HWND _buttonRemove;
	HWND _buttonSelectAll;
	HWND _list;
	
	HWND _dividingLineProperties;
	HWND _dividingLinePropertiesTitle;

	//HWND _editNameBox;
	//HWND _editNameTitle;

	HWND _buttonOpenPreset;

	HWND _comboBoxWavePreset;
	HWND _comboBoxWavePresetTitle;

	HWND _comboWaveLoop;
	HWND _comboWaveLoopTitle;

	HWND _comboWaveQuantizeMethod;
	HWND _comboWaveQuantizeMethodTitle;

	HWND _comboWaveFormat;
	HWND _comboWaveFormatTitle;

	HWND _editWaveSampleRate;
	HWND _editWaveSampleRateTitle;

	HWND _editWaveNumBits;
	HWND _editWaveNumBitsTitle;

	HFONT _boldFont;

private:
	SorceryView&	_view;
	
private:
	bool											_bListChangeDeferredUpdateRequest;
	bool											_bDeferredWriteAndPopulateRequest;

	Ut::UtVector<Ut::DynamicVector<Util::C_String>> _waveFilesToAdd;						// temp list of wave files to process and add to the WaveBankMgr

	//made temp static
	WaveBankMgr										_WaveBankMgr;								// our copy of the dom's WaveBankMgr info. for easy reference
};

#endif // WaveBankMgrPANE_H