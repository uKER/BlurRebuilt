#pragma once

#include "lux_app/CustomControls/DarkDialog.h"

typedef struct
{
	CWnd		*pWnd;
    const char	*pszName;
	bool		bSelected;
} SLayerBox;

typedef Ut::UtVector< Ut::DynamicVector<SLayerBox> > TLayerBoxList;


// CLayerDlg dialog

class CLayerDlg : public CDarkDialog
{
	DECLARE_DYNAMIC(CLayerDlg)

public:
	CLayerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLayerDlg();

// Dialog Data
	enum { IDD = IDD_LAYERS };

	void	AddLayer(const char *pszName);
	void	DeleteControls();
	void	CreateControls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	TLayerBoxList m_BoxList;

	DECLARE_MESSAGE_MAP()
public:
};
