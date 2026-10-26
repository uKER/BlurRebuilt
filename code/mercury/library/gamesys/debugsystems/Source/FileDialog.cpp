#include "DebugSystemsPch.h"
#include <Render/font/Font.h>

#include <DebugSystems/FileDialog.h>

#include <Render/debug/DebugRender.h>
#include <System/Serialise/I_FileSystem.h>
#include <System/misc/StringUtil.h>

namespace DebugSystems
{


//////////////////////////////////////////////////////////////////////////
//	C_FileDialogSkin
//////////////////////////////////////////////////////////////////////////
void C_FileDialogSkin::Reset()
{
	m_Position				= Maths::HVec2( 512, 48.0f );
	m_TitleFontScale		= Maths::HVec2( 32.0f, 48.0f );
	m_NormalFontScale		= Maths::HVec2( 28.0f, 32.0f );

	m_SelctionColour		= Maths::RGBA(228,  224,    232,    0x20);
	m_MarginColour          = Maths::RGBA(27,  24,    96,    0xff);
																
	m_SelectedFontColour	= Maths::RGBA( 1.0f,1.0f,1.0f,1.0f );
	m_DeselectedFontColour	= Maths::RGBA( 0.0f,0.0f,0.0f,1.0f );
	m_TitleFontColour		= Maths::RGBA( 1.0f,1.0f,1.0f,1.0f );
	

	m_MaxHeight				= 500.f;
	
	m_FontName				= "dontship/fonts/arialdebug.font";

	m_pFrame				= NULL;
}
//////////////////////////////////////////////////////////////////////////
// C_FileDialog
//////////////////////////////////////////////////////////////////////////
C_FileDialog::C_FileDialog(Util::C_String tittle, Util::C_String fileExtension, Util::C_String initFolder, I_FileDialogController* pController, const C_FileDialogSkin PCREF fileDialogSkin)
{
	m_Tittle = tittle;
	m_FileExtension = fileExtension;
	m_CurrentFolder = initFolder;
	m_pController = pController;
	if (fileDialogSkin.m_pFrame == NULL)
	{
		//Default frame
		m_pFrame = C_Frame::GetCommonFrame();
	}
	else
		m_pFrame = fileDialogSkin.m_pFrame;
	m_Skin = fileDialogSkin;

	//Font renderers
	m_pSelectedFont = Render::Font::FontSystem()->GetFontRenderer(m_Skin.m_FontName);
	m_pSelectedFont->SetScaleX(m_Skin.m_NormalFontScale.X());
	m_pSelectedFont->SetScaleY(m_Skin.m_NormalFontScale.Y());
	m_pSelectedFont->SetColourTop(m_Skin.m_SelectedFontColour);
	m_pSelectedFont->SetColourBottom(m_Skin.m_SelectedFontColour);

	m_pDeselectedFont = Render::Font::FontSystem()->GetFontRenderer(m_Skin.m_FontName);
	m_pDeselectedFont->SetScaleX(m_Skin.m_NormalFontScale.X());
	m_pDeselectedFont->SetScaleY(m_Skin.m_NormalFontScale.Y());
	m_pDeselectedFont->SetColourTop(m_Skin.m_DeselectedFontColour);
	m_pDeselectedFont->SetColourBottom(m_Skin.m_DeselectedFontColour);

	m_pTitleFont = Render::Font::FontSystem()->GetFontRenderer(m_Skin.m_FontName);
	m_pTitleFont->SetScaleX(m_Skin.m_TitleFontScale.X());
	m_pTitleFont->SetScaleY(m_Skin.m_TitleFontScale.Y());
	m_pTitleFont->SetColourTop(m_Skin.m_TitleFontColour);
	m_pTitleFont->SetColourBottom(m_Skin.m_TitleFontColour);
	m_pTitleFont->SetShadowEnable( true );
	m_pTitleFont->SetColourOutline(Maths::RGBA(0,0,25,255));
	m_pTitleFont->SetOutlineSize(0.35f);

	m_Active = true;

	UpdateFolder("");
}

C_FileDialog::~C_FileDialog()
{
	SAFE_DELETE_PTR(m_pSelectedFont);
	SAFE_DELETE_PTR(m_pDeselectedFont);
	SAFE_DELETE_PTR(m_pTitleFont);
}

void C_FileDialog::UpdateFolder(Util::C_String selectedFolder)
{
	//Clear list
	m_FileList.Clear();
	m_FileListPosition = 0;

	//List folder
	Sys::I_FileSystem *fs = Sys::GetDefaultFileSystem();
	
	Sys::C_DirectoryInfo directoryInfo;

	bool correctCurrentFolder = fs->DirectoryInfo(m_CurrentFolder.Chars(), &directoryInfo);
	{

		//Add back
		m_FileList.PushBack(C_FileItem("..", true));

		//Fill list with files and directories
		U32 numFiles = directoryInfo.NumFiles();	
		for (U32 i = 0; i < numFiles; i++)
		{
			Sys::C_FileInfo& fileInfo = directoryInfo.FileInfo(i);
			Sys::C_FilePath fileName = Sys::ExtractFilename(fileInfo.m_FileName);
			if (fileInfo.m_FileType == Sys::E_FileType_Directory)
			{
				//directory found
				m_FileList.PushBack(C_FileItem(fileName.Chars(),true));

				if (selectedFolder == fileName.Chars())
					m_FileListPosition = m_FileList.Size() - 1;
			}
			else if (m_FileExtension == "*")
			{
				//Add
				m_FileList.PushBack(C_FileItem(fileName.Chars(),false));
			}
			else if ((ExtractFileExtension(fileName) == m_FileExtension))
			{
				//Add
				m_FileList.PushBack(C_FileItem(fileName.Chars(),false));
			}

		}	
	}

}

void C_FileDialog::Update(const Maths::C_TimeS &timeStep)
{
	if (m_Active)
	{
		if (m_pController->MoveUp())
			Up();
		else if (m_pController->MoveDown())
			Down();
		else if (m_pController->Confirm())
			Confirm();
		else if (m_pController->Cancel())
			Cancel();
	}
}

const char* directoryMark = "<>";

void C_FileDialog::Render()
{	
	if (!m_Active)
		return;
	
	//Calculate size
	Maths::HVec2 windowSize(0.f,0.f);

	Maths::HVec2 tittleDimension = m_pTitleFont->GetDimensions(0,m_Tittle.Chars());

	Maths::HVec2 currentFolderDimension = m_pDeselectedFont->GetDimensions(0,m_CurrentFolder.Chars());
	Maths::HVec2 dirDimension = m_pDeselectedFont->GetDimensions(0,directoryMark);

	windowSize.Y(m_Skin.m_MaxHeight);


	windowSize.X( tittleDimension.X());
	windowSize.X(Maths::Max(windowSize.X(), currentFolderDimension.X()));

	for (S32 i = 0; i < m_FileList.Size(); i++)
	{
		Maths::HVec2 dimensions( m_pDeselectedFont->GetDimensions(0,m_FileList[i].m_Name));
		
		if (m_FileList[i].m_Directory)
		{
			dimensions.X(dimensions.X() + dirDimension.X());
		}

		windowSize.X(Maths::Max(windowSize.X(), dimensions.X()));
	}

	//How many fit
	S32 count = S32((windowSize.Y() - tittleDimension.Y() - currentFolderDimension.Y() - 2.f) / m_Skin.m_NormalFontScale.Y());

	//Calculate the render init
	S32 firstShowFileIndex = Maths::Max(S32(0), m_FileListPosition - count / 2);

	bool MoreItems = false;

	if ((firstShowFileIndex + count) > m_FileList.Size())
	{
		//Show back
		firstShowFileIndex = Maths::Max(S32(0),m_FileList.Size() - count + 1);
	}
	else
	{
		MoreItems = true;
		count--;
	}

	if (m_pFrame)
		//Draw frame
		m_pFrame->Draw(m_Skin.m_Position, windowSize);

	//Draw title
	m_pTitleFont->Print(Maths::HVec2(m_Skin.m_Position.X() + windowSize.X()*0.5f, m_Skin.m_Position.Y()), m_Tittle.Chars(), Render::Font::E_Justification_Centre);

	//Draw current folder
	m_pDeselectedFont->Print(Maths::HVec2(m_Skin.m_Position.X() + windowSize.X()*0.5f, m_Skin.m_Position.Y() + tittleDimension.Y()), m_CurrentFolder.Chars(), Render::Font::E_Justification_Centre);
	
	//Draw separators
	Render::Debug::Immediate()->SetBlendMode((m_MarginBlendMode)?DbUtil::E_BlendMode_Alpha:DbUtil::E_BlendMode_Solid);
	Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(m_Skin.m_Position.X(), m_Skin.m_Position.Y() + tittleDimension.Y()), Maths::HVec2(windowSize.X(), 1.f), m_Skin.m_MarginColour);
	Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(m_Skin.m_Position.X(), m_Skin.m_Position.Y() + tittleDimension.Y() + currentFolderDimension.Y()), Maths::HVec2(windowSize.X(), 1.f), m_Skin.m_MarginColour);
	Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);

	Maths::HVec2 itemPosition(m_Skin.m_Position.X(), m_Skin.m_Position.Y() + tittleDimension.Y() + currentFolderDimension.Y() + 2.f );
	//Draw items
	for (S32 i = firstShowFileIndex; i < m_FileList.Size() && ((i-firstShowFileIndex) < count); i++)
	{
		Render::Font::C_FontRenderer *pFontRenderer = (i == m_FileListPosition)?m_pSelectedFont:m_pDeselectedFont;

		if (i == m_FileListPosition)
		{
			Render::Font::C_FontRenderer *pFontRenderer = m_pSelectedFont;

			//Draw bar if needed
			if (m_Skin.m_SelctionColour.A() > 0)
			{
				Render::Debug::Immediate()->SetBlendMode((m_Skin.m_SelctionColour)?DbUtil::E_BlendMode_Alpha:DbUtil::E_BlendMode_Solid);
				Render::Debug::Immediate()->Draw2dBar(itemPosition, Maths::HVec2(windowSize.X(), m_pDeselectedFont->GetScaleY()), m_Skin.m_SelctionColour);
				Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);

			}
		}
		else
		{
			Render::Font::C_FontRenderer *pFontRenderer = m_pDeselectedFont;
		}
		
		pFontRenderer->Print(itemPosition, m_FileList[i].m_Name.Chars());
		if (m_FileList[i].m_Directory)
			pFontRenderer->Print(Maths::HVec2(itemPosition.X() + windowSize.X(), itemPosition.Y()), directoryMark, Render::Font::E_Justification_Right);


		itemPosition.Y(itemPosition.Y() + m_pDeselectedFont->GetScaleY());
	}
	if (MoreItems)
	{
		m_pSelectedFont->Print(itemPosition, "<more>...");
	}
	
}

void C_FileDialog::Up()
{
	if (m_FileListPosition > 0)
		m_FileListPosition--;
}

void C_FileDialog::Down()
{
	if (m_FileListPosition < m_FileList.Size() - 1)
		m_FileListPosition++;
}

void C_FileDialog::Confirm()
{
	//Check if it is a folder or a file
	if (m_FileList[m_FileListPosition].m_Directory)
	{
		if (m_FileList[m_FileListPosition].m_Name == "..")
		{
			BackDirectory();	
		}
		else
		{
			m_CurrentFolder = m_CurrentFolder + m_FileList[m_FileListPosition].m_Name + "\\";
			UpdateFolder("");
		}
		
	}
	else
	{
		//File
		m_SelectedFile = m_CurrentFolder + m_FileList[m_FileListPosition].m_Name;
		m_Active = false;
	}


}

void C_FileDialog::Cancel()
{
	//Exit
	BackDirectory();
}

bool C_FileDialog::SplitFolder( Util::C_String folder, Util::C_String& parentFolder, Util::C_String& outfolder)
{
	//Remove last character
	Util::C_String folderAux = folder(0, folder.Length() - 1);

    S32 slashPos = folderAux.RevSearch('\\');
    if(slashPos < 0)
    {
        parentFolder = "";
		outfolder = "";
		return false;
    }
    else
    {
        parentFolder = folderAux(0, slashPos + 1);
		outfolder = folderAux(slashPos + 1, folderAux.Length());

		return true;
    }
}

bool C_FileDialog::BackDirectory()
{
	Util::C_String parentFolder;
	Util::C_String folder;

	if (!SplitFolder(m_CurrentFolder, parentFolder, folder))
	{
		m_CurrentFolder = ".\\";
		//Last, EXIT
		m_SelectedFile = "";
		m_Active = false;

		return false;
	}
	else
	{
		m_CurrentFolder = parentFolder;
		UpdateFolder(folder);
		return true;
	}
}

bool C_FileDialog::Active()
{
	return m_Active;
}
void C_FileDialog::Active( bool active )
{
	m_Active = active;
}

bool C_FileDialog::CheckState( Util::C_String& selectedFile )
{
	if (!m_Active && m_SelectedFile != "")
	{
		selectedFile = m_SelectedFile;
		return true;
	}
	else
		return false;
}
}


