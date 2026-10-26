// XMLTreeView.cpp : implementation file
//

#include "stdafx.h"
#include "UIMaker2.h"
#include "XMLTreeView.h"
#include "XMLParser.h"
#include "XMLBuilder.h"
#include "UIMaker2Doc.h"
#include "commandline.h"
#include "FontMaker.h"
#include "SystemCommands.h"
#include "UIMaker2View.h"
#include "Config.h"
#include ".\xmltreeview.h"


// CXMLTreeView

IMPLEMENT_DYNCREATE(CXMLTreeView, CTreeView)

CXMLTreeView::CXMLTreeView()
{
}

CXMLTreeView::~CXMLTreeView()
{
}

BEGIN_MESSAGE_MAP(CXMLTreeView, CTreeView)
	ON_WM_LBUTTONDOWN()
	ON_WM_DROPFILES()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


// CXMLTreeView diagnostics

#ifdef _DEBUG
void CXMLTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CXMLTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG


// CXMLTreeView message handlers

BOOL CXMLTreeView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class

	dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;

	BOOL Ret = CTreeView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	DragAcceptFiles();
	
	ImageList.Create(16, 16, ILC_COLOR8, 0, 6);

	bm.LoadBitmap(IDB_BITMAP_TICK);
	ImageList.Add(&bm, RGB(0, 0, 0));

	bm1.LoadBitmap(IDB_BITMAP_CROSS);
	ImageList.Add(&bm1, RGB(0, 0, 0));

	bm2.LoadBitmap(IDB_BITMAP_FOLDER);
	ImageList.Add(&bm2, RGB(0, 0, 0));

	bm3.LoadBitmap(IDB_BITMAP_SCREEN);
	ImageList.Add(&bm3, RGB(0, 0, 0));

	bm4.LoadBitmap(IDB_BITMAP_AREA);
	ImageList.Add(&bm4, RGB(0, 0, 0));

	GetTreeCtrl().SetImageList(&ImageList,TVSIL_NORMAL);

	return Ret;
}

void GetPath(CTreeCtrl& tc, HTREEITEM hTI, CString& path)
{
	HTREEITEM hp = tc.GetParentItem(hTI);
	if (hp)
		::GetPath(tc, hp, path);

	if (!hp)
		path.Empty();
	else
		path += "\\";

	path += tc.GetItemText(hTI);
}

void CXMLTreeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	UINT Flags;
	int c=0;
	HTREEITEM hTI = GetTreeCtrl().HitTest(point, &Flags);

	if (hTI && !(Flags&TVHT_ONITEMBUTTON))
	{
		sprintf(mSelectedName,"%s",GetTreeCtrl().GetItemText(hTI));

		while(mSelectedName[c] != '.')
		{
			c++;
		}

		int a=0;
		while(mSelectedName[c] != NULL)
		{
			mSelectedExtension[a]=mSelectedName[c];
			a++;
			c++;
		}

		mSelectedExtension[a]=NULL;


		if(!strcmp(mSelectedExtension,".ini"))
		{
			CString path;
			::GetPath(GetTreeCtrl(), hTI, path);
			sprintf(mSelectedName, "%s", path);
		}
	}

	CTreeView::OnLButtonDown(nFlags, point);
}

void CXMLTreeView::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default

	CTreeView::OnDropFiles(hDropInfo);
}

void CXMLTreeView::ClearTree()
{
	GetTreeCtrl().DeleteAllItems();
}

HTREEITEM hTIMain2;
HTREEITEM ghTIMain[8];
int gRec=0;

void CXMLTreeView::RecBuildDir(const char *dir)
{
	struct _finddata_t file;
    intptr_t hFile;
	char name[256];
	HTREEITEM temp;

	sprintf(name,"%s\\*",dir);

	if( (hFile = _findfirst( name, &file )) != -1)
	{
		if(strcmp(file.name,".") && strcmp(file.name,"..") && !(file.attrib&_A_HIDDEN))
		{
			if(file.attrib&_A_SUBDIR)
			{
				ghTIMain[gRec+1]=GetTreeCtrl().InsertItem(file.name, 2, 2, ghTIMain[gRec]);
				sprintf(name,"%s\\%s",dir,file.name);
				gRec++;
				this->RecBuildDir(name);
			}
			else
			{
				temp=GetTreeCtrl().InsertItem(file.name, 3, 3, ghTIMain[gRec]);
			}
		}

		while( _findnext( hFile, &file ) == 0 )
		{
			if(strcmp(file.name,".") && strcmp(file.name,"..") && !(file.attrib&_A_HIDDEN))
			{
				if(file.attrib&_A_SUBDIR)
				{
					ghTIMain[gRec+1]=GetTreeCtrl().InsertItem(file.name, 2, 2, ghTIMain[gRec]);
					sprintf(name,"%s\\%s",dir,file.name);
					gRec++;
					this->RecBuildDir(name);
				}
				else
				{
					temp=GetTreeCtrl().InsertItem(file.name, 3, 3, ghTIMain[gRec]);
				}
			}
		}
	}

	gRec--;
	_findclose(hFile);
}

void CXMLTreeView::BuildTree()
{
	HTREEITEM hTIMain;

	CPoint cp;
	::GetCursorPos(&cp);
	ScreenToClient(&cp);

	hTIMain = GetTreeCtrl().HitTest(cp);
	hTIMain2 = GetTreeCtrl().HitTest(cp);

	if (!hTIMain) 
	{
		hTIMain = GetTreeCtrl().InsertItem(gXMLParser.GetFlowFileName(),4,4);
	}
	// Do Font File
	GetTreeCtrl().InsertItem(gConfig.mBuildFile, 3, 3, hTIMain);

	if (!hTIMain2) 
	{
		hTIMain2 = GetTreeCtrl().InsertItem(gConfig.mTextDirectory,4,4);
	}

	gRec=0;
	ghTIMain[0]=hTIMain2;
	RecBuildDir(gConfig.mTextDirectory);

	GetDocument()->UpdateAllViews(this);
}

void CXMLTreeView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	/*
	UINT Flags;
	char name[128];

	CTreeView::OnRButtonDown(nFlags, point);

	RECT r;
	GetWindowRect(&r);

	HTREEITEM hTI = GetTreeCtrl().HitTest(point, &Flags);
	sprintf(name,"%s",GetTreeCtrl().GetItemText(hTI));

	// check to make sure it's selectable - i.e. not a direcotry
	HTREEITEM hChild = GetTreeCtrl().GetChildItem(hTI);

	// allow the user to click on the top most item
	HTREEITEM hParent = GetTreeCtrl().GetParentItem(hTI);

	if(!hChild || !hParent)
	{
		if (hTI && !(Flags&TVHT_ONITEMBUTTON))
		{
			GetTreeCtrl().Select(hTI, TVGN_CARET);
			sprintf(name,"%s",GetTreeCtrl().GetItemText(hTI));

			CMenu menu;
			int pMenuID = 0;
			
			VERIFY(menu.LoadMenu(IDR_CHECKOUTMENU));

			CMenu* popup = menu.GetSubMenu(pMenuID);
			int sel=popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x+r.left,point.y+r.top, this);

			if(sel == ID_CHECKINMENU_CHECKINFILE) 
			{
				gCommandLine.AddLineArg(TC_RED,"Checking In File %s",name);
			}
			if(sel == ID_CHECKOUTMENU_CHECKOUT) 
			{
				gCommandLine.AddLineArg(TC_RED,"Checking Out File %s",name);
			}
			if(sel == ID_CHECKOUTMENU_GOTOTHISSCREEN || sel == ID_CHECKINMENU_GOTOTHISSCREEN) 
			{
				gCommandLine.AddLineArg(TC_RED,"Going to Screen %s",name);
			}

			RefreshImages();
			GetDocument()->UpdateAllViews(this);
		}
	}
	*/
}

void _RecFunc(CTreeCtrl& tc, HTREEITEM hTI)
{
	while (hTI)
	{
		HTREEITEM hChild = tc.GetChildItem(hTI);
		if (hChild)
		{
			_RecFunc(tc, hChild);
		}
		else
		{
			char xml_name[128];
			sprintf(xml_name,"%s",tc.GetItemText(hTI));

			int len=(int)strlen(xml_name);

			//make sure it's an xml file and not a directory without any children
			if(xml_name[len-1] == 'l' && xml_name[len-2] == 'm' && xml_name[len-3] == 'x')
			{

			}
		}

		hTI = tc.GetNextItem(hTI, TVGN_NEXT);
	}
}

void CXMLTreeView::RefreshImages()
{
	HTREEITEM hTI;

	hTI=GetTreeCtrl().GetRootItem();
	_RecFunc(GetTreeCtrl(), hTI);
}

void CXMLTreeView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	char command[256];
	sprintf(command,"\"%s\" %s",gConfig.mEditorExe,mSelectedName);
	
	gSystemCommands.Flush();
	gSystemCommands.AddCommand(command);
	gSystemCommands.Execute();
	//system(command);
}

