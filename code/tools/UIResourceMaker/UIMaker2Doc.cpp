// UIMaker2Doc.cpp : implementation of the CUIMaker2Doc class
//

#include "stdafx.h"
#include "UIMaker2.h"

#include "UIMaker2Doc.h"

#include "XMLParser.h"
#include "XMLBuilder.h"

#include "XMLTreeView.h"

#include "commandline.h"
#include "config.h"

#include "FontMaker.h"
#include "SystemCommands.h"
#include ".\uimaker2doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUIMaker2Doc

IMPLEMENT_DYNCREATE(CUIMaker2Doc, CDocument)

BEGIN_MESSAGE_MAP(CUIMaker2Doc, CDocument)
	ON_COMMAND(ID_BUTTON_BUILD, OnBuild)
	ON_COMMAND(ID_BUTTON_BUNDLE, OnBundle)
	
	ON_COMMAND(ID_BUTTON_COPY_TO_SERVER, OnCopyToServer)
	ON_COMMAND(ID_BUTTON_COPY_TO_CONSOLE, OnCopyToConsole)

	ON_COMMAND(ID_BUTTON_BUNDLE_EA, OnBundleEA)
	ON_COMMAND(ID_BUTTON_COPY_EA_TO_CONSOLE, OnCopyEAToConsole)
END_MESSAGE_MAP()


// CUIMaker2Doc construction/destruction

CUIMaker2Doc::CUIMaker2Doc()
{
	// TODO: add one-time construction code here

}

CUIMaker2Doc::~CUIMaker2Doc()
{
}

BOOL CUIMaker2Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CUIMaker2Doc serialization

void CUIMaker2Doc::Serialize(CArchive& ar)
{
	char filename[256];

	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
		sprintf(filename,"%s",ar.m_strFileName);

		gXMLParser.ParseFile(filename);

		POSITION pos = GetFirstViewPosition();
		while(pos != NULL)
		{
			CView* pView = GetNextView(pos);
			if(pView->IsKindOf( RUNTIME_CLASS( CXMLTreeView ) ))
			{
				((CXMLTreeView *)pView)->BuildTree();
			}
		}
	}
}


// CUIMaker2Doc diagnostics

#ifdef _DEBUG
void CUIMaker2Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CUIMaker2Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CUIMaker2Doc commands


// CMainFrame message handlers

void CUIMaker2Doc::OnBuild()
{
	gXMLParser.ProcessFonts(gConfig.mBuildFile);

	POSITION pos = GetFirstViewPosition();
	while(pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if(pView->IsKindOf( RUNTIME_CLASS( CXMLTreeView ) ))
		{
			((CXMLTreeView *)pView)->ClearTree();
			((CXMLTreeView *)pView)->BuildTree();
		}
	}

	UpdateAllViews(NULL);
}

void CUIMaker2Doc::OnBundle()
{
	gFontMaker.MakeBundles(false);

	UpdateAllViews(NULL);
}

void CUIMaker2Doc::OnBundleEA()
{
	gFontMaker.MakeBundles(true);

	UpdateAllViews(NULL);
}

void CUIMaker2Doc::OnCopyToServer()
{
	FILE *fp;
	char command[256];

	gCommandLine.AddLine(TC_BLACK,"");
	gCommandLine.AddLine(TC_BLACK,"Copying To Server...");

	sprintf(command,"%s\\bundles.temp",gConfig.mTextureExport);
	fp=fopen(command,"r");

	if(fp)
	{
		int a=0;
		unsigned char c;
		char bundle_name[128];

		c=fgetc(fp);
		while(c != 0xff)
		{
			bundle_name[a++]=c;
			if(c == 0x0a)	//end of line ---- lets bundle !!!!!
			{
				bundle_name[a-1]=NULL;

				gSystemCommands.Flush();

				char command[256];
				sprintf(command,"xbcp /y \"%s\\%s.bpr\" \"%s\\%s.bpr\"",gConfig.mPCExport,bundle_name,gConfig.mServerExport,bundle_name);
				gSystemCommands.AddCommand(command);

				sprintf(command,"xbcp /y \"%s\\%s.dat\" \"%s\\%s.dat\"",gConfig.mPCExport,bundle_name,gConfig.mServerExport,bundle_name);
				gSystemCommands.AddCommand(command);

				gSystemCommands.Execute();

				gCommandLine.AddLineArg(TC_BLACK,"    Copying %s.xml",bundle_name);

				a=0;
			}

			c=fgetc(fp);
		}

		fclose(fp);

		gCommandLine.AddLine(TC_BLACK,"...Copy Complete");
	}
	else
	{
		AfxMessageBox("Error Opening Temporary Bundle File - Please Build first",MB_OK|MB_ICONINFORMATION);
	}

	UpdateAllViews(NULL);
}




void CUIMaker2Doc::OnCopyEAToConsole()
{
	FILE *fp;
	char file[128];

	gCommandLine.AddLine(TC_BLACK,"");
	gCommandLine.AddLine(TC_BLACK,"Copying To Console...");

	sprintf(file,"%s\\ea_bundles.temp",gConfig.mTextureExport);
	fp=fopen(file,"r");

	if(fp)
	{
		int a=0;
		unsigned char c;
		char bundle_name[128];

		c=fgetc(fp);
		while(c != 0xff)
		{
			bundle_name[a++]=c;
			if(c == 0x0a)	//end of line ---- lets bundle !!!!!
			{
				bundle_name[a-1]=NULL;

				gSystemCommands.Flush();

				char command[512];
				sprintf(command,"xbcp /y \"%s\\%s.bpr\" \"%s\\%s.bpr\"",gConfig.mPCExport,bundle_name,gFontMaker.GetBundleExportPath(),bundle_name);
				gCommandLine.AddLineArg(TC_BLACK,"Executing ... %s",command);
				gSystemCommands.AddCommand(command);

				sprintf(command,"xbcp /y \"%s\\%s.dat\" \"%s\\%s.dat\"",gConfig.mPCExport,bundle_name,gFontMaker.GetBundleExportPath(),bundle_name);
				gCommandLine.AddLineArg(TC_BLACK,"Executing ... %s",command);
				gSystemCommands.AddCommand(command);

				gSystemCommands.Execute();

				gCommandLine.AddLineArg(TC_BLACK,"    Copying %s",bundle_name);

				a=0;
			}

			c=fgetc(fp);
		}

		fclose(fp);

		gCommandLine.AddLine(TC_BLACK,"...Copy Complete");
	}
	else
	{
		AfxMessageBox("Error Opening Temporary Bundle File - Please Build first",MB_OK|MB_ICONINFORMATION);
	}

	UpdateAllViews(NULL);
}

void CUIMaker2Doc::OnCopyToConsole()
{
	FILE *fp;
	char file[256];

	gCommandLine.AddLine(TC_BLACK,"");
	gCommandLine.AddLine(TC_BLACK,"Copying To Console...");

	sprintf(file,"%s\\bundles.temp",gConfig.mTextureExport);
	fp=fopen(file,"r");

	if(fp)
	{
		int a=0;
		unsigned char c;
		char bundle_name[128];

		c=fgetc(fp);
		while(c != 0xff)
		{
			bundle_name[a++]=c;
			if(c == 0x0a)	//end of line ---- lets bundle !!!!!
			{
				bundle_name[a-1]=NULL;

				gSystemCommands.Flush();

				char command[256];
				sprintf(command,"xbcp /y \"%s\\%s.bpr\" \"%s\\%s.bpr\"",gConfig.mPCExport,bundle_name,gFontMaker.GetBundleExportPath(),bundle_name);
				gCommandLine.AddLineArg(TC_BLACK,"Executing ... %s",command);
				gSystemCommands.AddCommand(command);

				sprintf(command,"xbcp /y \"%s\\%s.dat\" \"%s\\%s.dat\"",gConfig.mPCExport,bundle_name,gFontMaker.GetBundleExportPath(),bundle_name);
				gCommandLine.AddLineArg(TC_BLACK,"Executing ... %s",command);
				gSystemCommands.AddCommand(command);

				gSystemCommands.Execute();

				gCommandLine.AddLineArg(TC_BLACK,"    Copying %s",bundle_name);

				a=0;
			}

			c=fgetc(fp);
		}

		fclose(fp);

		gCommandLine.AddLine(TC_BLACK,"...Copy Complete");
	}
	else
	{
		AfxMessageBox("Error Opening Temporary Bundle File - Please Build first",MB_OK|MB_ICONINFORMATION);
	}

	UpdateAllViews(NULL);
}
