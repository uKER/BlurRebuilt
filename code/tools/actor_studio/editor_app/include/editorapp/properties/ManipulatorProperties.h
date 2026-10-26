#ifndef EDITOR_TOOL_PROPERTIES
#define	EDITOR_TOOL_PROPERTIES

#ifdef _MSC_VER
#pragma once
#endif

class wxPropertyGridManager;
class wxPGProperty;
class wxPropertyGridEvent;

namespace Editor
{
	class C_ActorCreateManipulator;
	class C_SelectManipulator;
	class C_MoveManipulator;
	class C_RotateManipulator;
	class C_ScaleManipulator;

	class I_ManipulatorProperties
	{
	public:
		I_ManipulatorProperties(wxPropertyGridManager *pPgMgr);
		virtual ~I_ManipulatorProperties();

		virtual void Refresh();
		virtual void OnPropertyChange(wxPropertyGridEvent& e) = 0;

	protected:
		wxPropertyGridManager	*m_pPropertyGridMgr;
	};

	class C_CreateManipulatorProperties : public I_ManipulatorProperties
	{
		public:
		C_CreateManipulatorProperties(wxPropertyGridManager *pPgMgr, C_ActorCreateManipulator *pTool);
		~C_CreateManipulatorProperties();

		void Refresh();
		void OnPropertyChange(wxPropertyGridEvent& e);

	private:
		C_ActorCreateManipulator *m_pTool;
		wxPGProperty			 *m_pCreationModeProp;
		wxPGProperty			 *m_pOrientationModeProp;
	};

	class C_SelectManipulatorProperties : public I_ManipulatorProperties
	{
	public:
		C_SelectManipulatorProperties(wxPropertyGridManager *pPgMgr, C_SelectManipulator *pTool);
		~C_SelectManipulatorProperties();

		void Refresh();
		void OnPropertyChange(wxPropertyGridEvent& e);

	private:
		C_SelectManipulator		*m_pTool;
		wxPGProperty			*m_pSelectionModeProp;
	};

	class C_MoveManipulatorProperties : public C_SelectManipulatorProperties
	{
	public:
		C_MoveManipulatorProperties(wxPropertyGridManager *pPgMgr, C_MoveManipulator *pTool);
		~C_MoveManipulatorProperties();

		void Refresh();
		void OnPropertyChange(wxPropertyGridEvent& e);

	private:
		C_MoveManipulator		*m_pTool;
		wxPGProperty			*m_pCoordinateSpaceProp;
		wxPGProperty			*m_pSnapMoveProp;
		wxPGProperty			*m_pSnapMoveValueProp;
	};



	class C_RotateManipulatorProperties : public C_SelectManipulatorProperties
	{
	public:
		C_RotateManipulatorProperties(wxPropertyGridManager *pPgMgr, C_RotateManipulator *pTool);
		~C_RotateManipulatorProperties();

		void Refresh();
		void OnPropertyChange(wxPropertyGridEvent& e);

	private:
		C_RotateManipulator		*m_pTool;
		wxPGProperty			*m_pCoordinateSpaceProp;
		wxPGProperty			*m_pSnapRotateProp;
		wxPGProperty			*m_pSnapRotateValueProp;
	};



	class C_ScaleManipulatorProperties : public C_SelectManipulatorProperties
	{
	public:
		C_ScaleManipulatorProperties(wxPropertyGridManager *pPgMgr, C_ScaleManipulator *pTool);
		~C_ScaleManipulatorProperties();

		void Refresh();
		void OnPropertyChange(wxPropertyGridEvent& e);

	private:
		C_ScaleManipulator		*m_pTool;
		wxPGProperty			*m_pCoordinateSpaceProp;
		wxPGProperty			*m_pSnapScaleProp;
		wxPGProperty			*m_pSnapScaleValueProp;
	};

}

#endif