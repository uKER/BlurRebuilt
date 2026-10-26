#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include "DebugSystems/Frame.h"
#include "Btl/Containers/vector.h"
#include "Core/containers/String.h"

namespace Render { namespace Font { class C_FontRenderer;}}

namespace DebugSystems
{

class C_Frame;

//Interface to control the FileDialog
class I_FileDialogController
{
public:
	// Debounced movement (preferably with delay & repeat)
	virtual const bool 		MoveUp( ) = 0;
	virtual const bool 		MoveDown( ) = 0;
	
	// Debounced selection
	virtual const bool 		Confirm( ) = 0;
	virtual const bool 		Cancel( ) = 0;
};

/// Skin for defining the render style of the DebugMenu (default is a kinda Windows Eggplant, yuk!)
class C_FileDialogSkin
{	
public:
	C_FileDialogSkin( ) { Reset( );}

private:
	void Reset( );

public:
	/// Note menu is centred on the X but place on the Y
	Maths::HVec2			m_Position;
	Maths::HVec2			m_TitleFontScale;
	Maths::HVec2			m_NormalFontScale;

	/// Note : Width = (largest 'item text' * font size) + (largest 'value text' * font size) + ((outline + border) * 2)
	F32						m_MaxHeight;

	
	Maths::RGBA				m_SelctionColour;
	Maths::RGBA				m_MarginColour;

	Maths::RGBA				m_SelectedFontColour;
	Maths::RGBA				m_DeselectedFontColour;
	Maths::RGBA				m_TitleFontColour;

	Util::C_String			m_FontName;

	C_Frame*				m_pFrame;
};

class C_FileItem
{
public:

	C_FileItem(Util::C_String name, bool directory)
	{
		m_Name = name;
		m_Directory = directory;
	}

	Util::C_String m_Name;
	bool m_Directory;
};

class C_FileDialog
{
public:
	C_FileDialog(Util::C_String tittle, Util::C_String fileExtension, Util::C_String initFolder, I_FileDialogController* pController, const C_FileDialogSkin PCREF fileDialogSkin);
	~C_FileDialog();

	void							Update( const Maths::C_TimeS& timeStep );
	void							Render( );
	bool							Active();
	void							Active(bool active);

	bool							CheckState(Util::C_String& selectedFile);
private:
	
	void 							Up();
	void 							Down();
	void 							Confirm();
	void							Cancel();

	void							UpdateFolder(Util::C_String selectedFolder);
	bool							SplitFolder( Util::C_String folder, Util::C_String& parentFolder, Util::C_String& outfolder);
	bool							BackDirectory();
private:

	C_FileDialogSkin				m_Skin;
	C_Frame*						m_pFrame;

	Render::Font::C_FontRenderer*	m_pSelectedFont;
	Render::Font::C_FontRenderer*	m_pDeselectedFont;
	Render::Font::C_FontRenderer*	m_pTitleFont;

	
	Maths::HVec2					m_Position;
	Maths::HVec2					m_NamePos;
	Maths::HVec2					m_ValuePos;
	Maths::HVec2					m_TitlePos;
	F32								m_NameWidth;
	F32								m_ValueWidth;
	bool							m_Active;

	bool						    m_MarginBlendMode;
	bool						    m_HighlightBlendMode;

	I_FileDialogController*			m_pController;

	Util::C_String					m_Tittle;
	Util::C_String					m_SelectedFile;
	Util::C_String					m_FileExtension;
	Util::C_String					m_CurrentFolder;
	S32								m_FileListPosition;
	Btl::Vector<C_FileItem>			m_FileList;
};
}

#endif
