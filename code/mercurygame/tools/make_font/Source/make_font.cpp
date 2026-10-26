// MakeFont.cpp : Defines the entry point for the application.
//

#include "StdAfx.h"


enum E_MakeFontFileVersion
{
    E_MakeFontFileVersion_Initial = 0,
    E_MakeFontFileVersion_Current = E_MakeFontFileVersion_Initial
};


HINSTANCE hInst;


template<class T> T Min(const T a, const T b) { return (a > b) ? b : a; }
template<class T> T Max(const T a, const T b) { return (a < b) ? b : a; }

int g_AntiAlias = 0;
int g_Initial256 = 1;
int g_Width = 128, g_Height = 128;

char dialogCharListFileName[256];
Util::C_String charListFileName="";
U32 outputAllGlyphs = 0;

GLYPHSET *glyphdata;
U32 pages[256];
U32 currentPage, pageCount;
U32 pageCharStart, pageCharEnd;


class C_GlyphValue	: public Btl::ListElementSingle<C_GlyphValue>
{
public:
	C_GlyphValue(){}
	~C_GlyphValue(){}
	C_GlyphValue(U32 in){value = in;}

public:
	U32 value;
};

Btl::List <C_GlyphValue> glyphList;
Btl::List <C_GlyphValue> sortedList;


void InitSystem(const char* param)
{
	pageCharStart = 0;
	pageCharEnd = 0;
	currentPage = 0; 
	pageCount = 0;
	
	//Try to read the configuration file
	Sys::C_XmlDom xmlConfigFile;
	xmlConfigFile.Read(param);
	
	Sys::C_XmlDomBaseNode* pRootNode = xmlConfigFile.GetRootNodePtr();
	if (pRootNode)
	{
		const Sys::C_XmlDomBaseNode* pWidthNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByName(pRootNode, "width");
		if (pWidthNode)
		{
			const Sys::C_XmlAttribute* pWidth = 
			((Sys::C_XmlDomElement*)pWidthNode)->GetAttributes().GetAttributeByName("value");
			if (pWidth)
			{
				g_Width = atoi(pWidth->value.Chars());	
			}
		}

		const Sys::C_XmlDomBaseNode* pHeightNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByName(pRootNode, "height");
		if (pHeightNode)
		{
			const Sys::C_XmlAttribute* pHeight = 
			((Sys::C_XmlDomElement*)pHeightNode)->GetAttributes().GetAttributeByName("value");
			if (pHeight)
			{
				g_Height = atoi(pHeight->value.Chars());	
			}
		}

		const Sys::C_XmlDomBaseNode* pAntiAliasNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByName(pRootNode, "antialias");
		if (pAntiAliasNode)
		{
			const Sys::C_XmlAttribute* pAntiAlias = 
			((Sys::C_XmlDomElement*)pAntiAliasNode)->GetAttributes().GetAttributeByName("value");
			if (pAntiAlias)
			{
				g_AntiAlias = atoi(pAntiAlias->value.Chars());	
			}
		}

		const Sys::C_XmlDomBaseNode* pInitial256Node = 
		Sys::C_XmlDomUtility::FindXmlNodeByName(pRootNode, "initial256");
		if (pInitial256Node)
		{
			const Sys::C_XmlAttribute* pInitial256 = 
			((Sys::C_XmlDomElement*)pInitial256Node)->GetAttributes().GetAttributeByName("value");
			if (pInitial256)
			{
				g_Initial256 = atoi(pInitial256->value.Chars());	
			}
		}

		const Sys::C_XmlDomBaseNode* pOutputAllGlyphsNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByName(pRootNode, "outputallglyphs");
		if (pOutputAllGlyphsNode)
		{
			const Sys::C_XmlAttribute* pOutputAllGlyphs = 
			((Sys::C_XmlDomElement*)pOutputAllGlyphsNode)->GetAttributes().GetAttributeByName("value");
			if (pOutputAllGlyphs)
			{
				outputAllGlyphs = atoi(pOutputAllGlyphs->value.Chars());	
			}
		}

		const Sys::C_XmlDomBaseNode* pCharListFileNameNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByName(pRootNode, "charlistfilename");
		if (pCharListFileNameNode)
		{
			const Sys::C_XmlAttribute* pCharListFileName = 
			((Sys::C_XmlDomElement*)pCharListFileNameNode)->GetAttributes().GetAttributeByName("value");
			if (pCharListFileName)
			{
				charListFileName = atoi(pCharListFileName->value.Chars());	
			}
		}

	}	
}

void CloseSystem()
{
}


inline int NextPower(int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}
/*
// Callback function for the Save Dialog.
UINT CALLBACK HookFunc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch(Msg)
	{
		case WM_INITDIALOG:
			SendDlgItemMessage(Wnd, IDC_ANTIALIAS, BM_SETCHECK, BST_CHECKED, 0);
			SetDlgItemInt(Wnd, IDC_WIDTH, g_Width, FALSE);
			SetDlgItemInt(Wnd, IDC_HEIGHT, g_Height, FALSE);

			CheckRadioButton(Wnd, IDC_RADIO_LIST, IDC_RADIO_ALL, IDC_RADIO_LIST);
			SendDlgItemMessage(Wnd, IDC_INITIAL256, BM_SETCHECK, BST_CHECKED, 0);
			
			if(charListFileName != Util::C_String())
			{
				HWND hc = GetDlgItem(Wnd, IDC_EDIT_FILENAME);
				SendMessage(hc, EM_SETSEL, 0, -1);
				SendMessage(hc, EM_REPLACESEL, 0, (LPARAM)(char*)charListFileName.Chars());
			}
		break;

		case WM_DESTROY:
			g_AntiAlias = (int)SendDlgItemMessage(Wnd, IDC_ANTIALIAS, BM_GETCHECK, 0, 0);
			g_Initial256 = (int)SendDlgItemMessage(Wnd, IDC_INITIAL256, BM_GETCHECK, 0, 0);
			g_Width = GetDlgItemInt(Wnd, IDC_WIDTH, 0, FALSE);
			g_Height = GetDlgItemInt(Wnd, IDC_HEIGHT, 0, FALSE);
		break;
		
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 

			switch (wmId)
			{
				case IDC_RADIO_LIST:
					outputAllGlyphs = 0;

					EnableWindow(GetDlgItem(Wnd, IDC_EDIT_FILENAME), 1);
					EnableWindow(GetDlgItem(Wnd, IDC_BUTTON_BROWSE), 1);
					EnableWindow(GetDlgItem(Wnd, IDC_INITIAL256), 1);
				break;

				case IDC_RADIO_ALL:
					outputAllGlyphs = 1;

					EnableWindow(GetDlgItem(Wnd, IDC_EDIT_FILENAME), 0);
					EnableWindow(GetDlgItem(Wnd, IDC_BUTTON_BROWSE), 0);
					EnableWindow(GetDlgItem(Wnd, IDC_INITIAL256), 0);
				break;
				
				case IDC_BUTTON_BROWSE:
						
					OPENFILENAME Of;
					ZeroMemory(&Of, sizeof(Of));

					Of.lStructSize	= sizeof(Of);
					Of.lpstrFilter	= "Txt Files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
					Of.lpstrFile	= dialogCharListFileName;
					Of.lpstrDefExt	= "txt";
					Of.nMaxFile		= sizeof(dialogCharListFileName);
					Of.Flags		= OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_EXPLORER;

					if(!GetOpenFileName(&Of))
					{
						return 0;
					}

					charListFileName = Util::C_String(dialogCharListFileName);

					HWND hc = GetDlgItem(Wnd, IDC_EDIT_FILENAME);
					SendMessage(hc, EM_SETSEL, 0, -1);
					SendMessage(hc, EM_REPLACESEL, 0, (LPARAM)(char*)charListFileName.Chars());
				break;
			}
		break;
	}

	return 0;
}
*/
U32 IsGlyphInList(U32 glyph)
{
	U32 startCharacter = 0, lastCharacter = 0;
	for(U32 i = 0; i < glyphdata->cRanges; i++)
	{
		startCharacter = glyphdata->ranges[i].wcLow;
		lastCharacter = startCharacter + glyphdata->ranges[i].cGlyphs;
		if(startCharacter <= glyph && glyph < lastCharacter)
			return 1;
	}
	return 0;
}

void SetPageLimits()
{
	pageCharStart = currentPage * 256;
	pageCharEnd = pageCharStart + 256;
}

void IncrementPage()
{
	currentPage++;
	SetPageLimits();
}

void SetupPages()
{
	U32 currentCharacter = 0;
	Btl::List<C_GlyphValue>::Iterator it = glyphList.Begin();

	SetPageLimits();

	while(it != glyphList.End())
	{
		currentCharacter = it->value;

		while(currentCharacter >= pageCharStart)
		{
			if(currentCharacter < pageCharEnd)
			{
				pages[pageCount] = currentPage;
				pageCount++;

				IncrementPage();
			}
			else
			{
				IncrementPage();
			}
		}
		++it;
	}
}


int WINAPI WinMain(HINSTANCE Instance, HINSTANCE, LPSTR cmdLine, int)
{      
	InitSystem(cmdLine);

	{
		U32 startCharacter = 0, currGlyph = 0;

		//Attk::C_Image		TheTga;

		hInst = Instance;
		CHOOSEFONT Cf;
		LOGFONT Lf;
		OPENFILENAME Of;
		char userFileName[256];

		ZeroMemory(&Cf, sizeof(Cf));
		ZeroMemory(&Lf, sizeof(Lf));
		ZeroMemory(&Of, sizeof(Of));
		ZeroMemory(userFileName, sizeof(userFileName));

		// If a charlist.txt file exists in base directory, set this as the default file
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(".\\charlist.txt", Sys::E_FileAccess_Read);
		if(pFile != NULL)
		{
			charListFileName = ".\\charlist.txt";

			SAFE_DELETE_PTR(pFile);
		}
		else
		{
			charListFileName = "";
		}

		
		Cf.lStructSize	= sizeof(Cf);
		Cf.lpLogFont	= &Lf;
		Cf.Flags		= CF_SCREENFONTS;

		if(!ChooseFont(&Cf))
		{
			CloseSystem();
			return 0;
		}

		HDC		Dc		= CreateDC("DISPLAY", 0, 0, 0);
		HFONT	Font	= CreateFontIndirect(&Lf);
		HFONT	OldFont	= (HFONT)SelectObject(Dc, Font);
		

		// Retrieve Glyph Data from the Font
		DWORD GlyphSize = GetFontUnicodeRanges(Dc, NULL);
		glyphdata = (GLYPHSET *)new unsigned char[ GlyphSize ];
		GetFontUnicodeRanges(Dc, glyphdata); 


		// Calculate Dimensions of Glyphs
		SIZE MaxSize;
		MaxSize.cx = 0;
		MaxSize.cy = 0;
		char sizeBuf[2];
		sizeBuf[1] = 0;
		for(U32 currSizingRange = 0; currSizingRange < glyphdata->cRanges; currSizingRange++)
		{
			startCharacter = glyphdata->ranges[currSizingRange].wcLow;
			for(U32 currSizingGlyph = 0; currSizingGlyph < glyphdata->ranges[currSizingRange].cGlyphs; currSizingGlyph++)
			{
				sizeBuf[0] = startCharacter + currSizingGlyph;
				SIZE Sz;

				if(GetTextExtentPoint32(Dc, sizeBuf, 1, &Sz))
				{
					MaxSize.cx = Max(MaxSize.cx, Sz.cx);
					MaxSize.cy = Max(MaxSize.cy, Sz.cy);
				}
			}
		}
		MaxSize.cx += 10;
		MaxSize.cy += 10;

		g_Width = g_Height = NextPower(Max(MaxSize.cx, MaxSize.cy));

		Of.lStructSize		= sizeof(Of);
		Of.lpstrFilter		= "TGA Files (*.tga)\0*.tga\0All files (*.*)\0*.*\0";
		Of.lpstrFile		= userFileName;
		Of.lpstrDefExt		= "tga";
		Of.nMaxFile			= sizeof(userFileName);
		Of.Flags			= OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;
		Of.hInstance		= Instance;
		Of.lpTemplateName	= NULL;
		Of.lpfnHook			= NULL;


		if(!GetSaveFileName(&Of))
		{
			SelectObject(Dc, OldFont);
			DeleteObject(Font);
			DeleteObject(Dc);

			CloseSystem();
			return 0;
		}
		
		SelectObject(Dc, OldFont);
		DeleteObject(Font);

		if(g_AntiAlias)
			Lf.lfQuality = ANTIALIASED_QUALITY;
		else
			Lf.lfQuality = NONANTIALIASED_QUALITY;

		Font = CreateFontIndirect(&Lf);
		OldFont = (HFONT)SelectObject(Dc, Font);

		MaxSize.cx = g_Width;
		MaxSize.cy = g_Height;


		HBITMAP		Bitmap			= CreateCompatibleBitmap(Dc, MaxSize.cx, MaxSize.cy);
		HBITMAP		OldBitmap		= (HBITMAP)SelectObject(Dc, Bitmap);
		int			OldBkMode		= SetBkMode(Dc, OPAQUE);
		COLORREF	OldBkColour		= SetBkColor(Dc, RGB(0, 0, 0));
		COLORREF	OldTextColour	= SetTextColor(Dc, RGB(255, 255, 255));

		RECT Rc;

		Rc.left = Rc.top = 0;
		Rc.right = MaxSize.cx;
		Rc.bottom = MaxSize.cy;


		// Setup the Pages of Characters that are to be Written out
		if(outputAllGlyphs || (charListFileName == Util::C_String("") && g_Initial256))
		{
			if(outputAllGlyphs)
			{
				for(U32 i = 0; i < glyphdata->cRanges; i++)
				{
					startCharacter = glyphdata->ranges[i].wcLow;
					for(U32 j = 0; j < glyphdata->ranges[i].cGlyphs; j++)
					{
						C_GlyphValue *newItem = new C_GlyphValue(startCharacter + j);
						glyphList.PushBack(*newItem);
					}
				}
			}
			else
			{
				for(U32 checkInitial = 0; checkInitial < 256; checkInitial++)
				{
					if(IsGlyphInList(checkInitial))
					{
						C_GlyphValue *newItem = new C_GlyphValue(checkInitial);
						glyphList.PushBack(*newItem);
					}
				}
			}

			Util::C_String filename = userFileName;
			S32 fileFormatPtr = filename.RevSearch('.');
			filename = filename(0, fileFormatPtr);
			filename = filename + "_";
			filename = filename + "charlist.txt";

			Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(filename, Sys::E_FileAccess_Write );
			if(pFile)
			{
				Sys::TextSerialiser ar(*pFile);

				Util::C_String output;

				C_GlyphValue* currentGlyph = NULL;
				Btl::List<C_GlyphValue>::Iterator it = glyphList.Begin();
				while(it != glyphList.End())
				{
					output.Printf("%d\n", it->value);
					ar << output;

					++it;
				}

				SAFE_DELETE_PTR( pFile );
			}
		}
		else
		{
			Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(charListFileName, Sys::E_FileAccess_Read);
			
			if(pFile)
			{
				U32 newGlyph = 0;

				Sys::TextDeserialiser ar(*pFile);
				//Sys::I_StreamTextPtr textStreamPtr = Sys::ReadStreamText( Sys::I_StreamPtr(filePtr) );

				if(g_Initial256)
				{
					for(U32 checkInitial = 0; checkInitial < 256; checkInitial++)
					{
						if(IsGlyphInList(checkInitial))
						{
							C_GlyphValue *newItem = new C_GlyphValue(checkInitial);
							glyphList.PushBack(*newItem);
						}
					}

					Util::C_String input;
					while(pFile->GetPosition() <= pFile->Length())
					{
						ar >> input;

						newGlyph = atoi(input);
						if(newGlyph > 256 && IsGlyphInList(newGlyph))
						{
							C_GlyphValue *newItem = new C_GlyphValue(newGlyph);
							glyphList.PushBack(*newItem);
						}
					}
				}
				else
				{
					Util::C_String input;
					while(pFile->GetPosition() <= pFile->Length())
					{
						ar >> input;

						newGlyph = atoi(input);
						if(IsGlyphInList(newGlyph))
						{
							C_GlyphValue *newItem = new C_GlyphValue(newGlyph);
							glyphList.PushBack(*newItem);
						}
					}
				}

				SAFE_DELETE_PTR( pFile );
			}
		}
		SetupPages();


		// Write out Characters to Display and Read them into an Image

		Btl::List<C_GlyphValue>::Iterator it = glyphList.Begin();
		U32 pageGlyphStart = 0, pageGlyphEnd = 0, baseLoc = 0;

		for(U32 imageCount = 0; imageCount < pageCount; imageCount++)
		{
			Attk::C_Status status;

			Attk::C_Image		TheImage;

			//TheImage.AddFrame();
			//TheImage.AddLayer(MaxSize.cx * 16, MaxSize.cy * 16, 1);
			//TheImage.AddMipLevel();

			//TheImage.NewPixelData( MaxSize.cx * 16, MaxSize.cy * 16, 1, Attk::E_PixelFormat_B8G8R8A8U );
			//TheImage.Fill( Attk::C_PixelR32G32B32A32F(1.0f, 1.0f, 1.0f, 0.0f) );

			//TheImage.Initialise(Image::ImageTypeRGBA, MaxSize.cx * 16, MaxSize.cy * 16, 4);
			//TheImage.SetRGBWhite();
			//TheImage.ClearAlpha();

			TheImage.NewPixelData(MaxSize.cx * 16, MaxSize.cy * 16, 1, Attk::E_PixelFormat_B8G8R8A8U);
			TheImage.Fill(Attk::C_PixelR32G32B32A32F(1.0f, 1.0f, 1.0f, 0.0f));

			pageGlyphStart = pages[imageCount] * 256;
			pageGlyphEnd = pageGlyphStart + 256;

			while((it != glyphList.End()) && (it->value < pageGlyphEnd))
			{
				unsigned short Buf[2];
				Buf[0] = it->value;
				Buf[1] = 0;
				ExtTextOutW(Dc, 5, 5, ETO_OPAQUE, &Rc, (LPCWSTR)Buf, 1, 0);

				baseLoc = it->value - pageGlyphStart;

				int Dy = (baseLoc / 16) * MaxSize.cy;
				int Dx = (baseLoc % 16) * MaxSize.cx;

				for(int y = 0; y < MaxSize.cy; y++)
				{
					for(int x = 0; x < MaxSize.cx; x++)
					{
						COLORREF Rgb = GetPixel(Dc, x, y);

						Attk::C_PixelR32G32B32A32F imagePixel = TheImage.GetPixel(Dx + x, Dy + y);
						imagePixel.alpha = (F32)GetGValue(Rgb) / 255.0f;
						
						TheImage.SetPixel(imagePixel, Dx + x, Dy + y);
					}
				}

				++it;
			}


			// Remove the File Format from the Filename
			Util::C_String filename = userFileName;
			S32 fileFormatPtr = filename.RevSearch('.');
			filename = filename(0, fileFormatPtr);
			Util::C_String extension;
			extension.Printf("_%03d.tga", pages[imageCount]);
			filename = filename + extension;

			//Attk::C_Options options;

			TheImage.Write(filename.Chars(), Attk::E_ImageFormat_Tga);//, &options);

			/*

			Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(filename, Sys::E_FileAccess_Write, Sys::E_FileCreate_CreateAlways );

			if(pFile)
			{
				Image::ConvertOptions Co;
				strcpy(Co.textureName, "");
				Co.fileFormat		= Image::FileFormatTGA;
				Co.textureFormat	= Image::TextureFormat32;
				Co.clutFormat		= Image::ClutFormatNone;

				Image::Tga tgaFile;
				// Convert Image to Tga format
				tgaFile.ConvertFromImage(TheImage, Co);

				// Save Tga file
				tgaFile.Write( *pFile );

				SAFE_DELETE_PTR(pFile);
			}
			*/
		}

		SetBkMode(Dc, OldBkMode);
		SetBkColor(Dc, OldBkColour);
		SetTextColor(Dc, OldTextColour);
		SelectObject(Dc, OldFont);
		SelectObject(Dc, OldBitmap);
		DeleteObject(Bitmap);
		DeleteObject(Font);
		DeleteObject(Dc);
	}
		
	CloseSystem();

	return 0;
}

