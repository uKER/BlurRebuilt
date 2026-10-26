#ifndef EDITOR_TOOL_HANDLER_H
#define EDITOR_TOOL_HANDLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>

namespace EditorLib
{
	class I_UpdateHint;
}

namespace Editor
{
	class C_RenderViewport;
	class I_Manipulator;
	class C_ManipulatorPropertyPanel;

	class C_ManipulatorManager
	{
	public:
		C_ManipulatorManager();
		~C_ManipulatorManager();

		bool AddManipulator(const Btl::String& name, I_Manipulator *pManipulator);
		bool ManipulatorExists(const Btl::String& name) const;
		
		I_Manipulator* GetManipulator(const Btl::String& name);
		const I_Manipulator* GetManipulator(const Btl::String& name) const;

		I_Manipulator* GetActiveManipulator();
		const I_Manipulator* GetActiveManipulator() const;
		const char* GetActiveManipulatorName() const;

		I_Manipulator* SetActiveManipulator(const Btl::String& name);
		void SetActiveManipulator(I_Manipulator* pManipulator);

		void AttachPropertyPanel(C_ManipulatorPropertyPanel* pPropertyPanel);
		C_ManipulatorPropertyPanel* GetAttachedPropertyPanel();

		void Dirty();

		bool ToggleMode(E_ManipulatorMode mode);
		bool IsModeEnabled(E_ManipulatorMode mode) const;
		bool IsNormalMode() const;

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:

			C_Slots(C_ManipulatorManager* pManipulatorManager);
			void Connect();
			void Disconnect();

			void OnCoreContextChanged(const Btl::String& context, const void* contextData);
			void OnDgNodeUpdated(const EditorLib::I_DgNode& node, const EditorLib::C_UpdateHintCollection* pHc);

		private:
			C_ManipulatorManager		*m_pManipulatorManager;
		};

		Ut::UtMap<Btl::String, I_Manipulator*>	m_Manipulators;
		I_Manipulator								*m_pActiveManipulator;
		C_ManipulatorPropertyPanel					*m_pPropertyPanel;
		U32											m_mode;

		C_Slots							m_Slots;
		mutable bool					m_dirty;

		void OnCoreContextChanged(const Btl::String& context, const void* contextData);
	};

	class I_Manipulator : public wxEvtHandler
	{
	public:
		I_Manipulator();
		virtual ~I_Manipulator();

		virtual E_ManipulatorType GetType() const = 0;

		virtual bool IsA(E_ManipulatorType type) const;

		static void		BindRender3D();
		static U32		GetScreenSize();
		static void		SetScreenSize(U32 size);
		static F32		GetRenderScale(
			const C_RenderViewport& viewport, 
			const Maths::HVec3& position);
		bool			GetInitialMatrix(
			Maths::HMat44& matrix, 
			E_ManipulatorSpace space);

		virtual void			AttachToViewport(C_RenderViewport *pViewport);
		C_RenderViewport*		GetAttachedViewport();
		const C_RenderViewport* GetAttachedViewport() const;

		void SetToolManager(C_ManipulatorManager *pManager);

		void SetCoordinateSpace(E_ManipulatorSpace space);
		E_ManipulatorSpace GetCoordinateSpace() const;

		virtual void OnMouseEvent(wxMouseEvent &e);
		virtual void OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e);
		virtual void OnKeyDown(wxKeyEvent &e);
		virtual void OnKeyUp(wxKeyEvent &e);
		virtual void OnSelectionChanged();

		virtual void Render(const C_RenderViewport *pViewport) const;

		virtual void SetData(const void* userData);

		Maths::C_Line CalculateRay(S32 clickX, S32 clickY);

		I_ManipulatorContext* GetContext();

	protected:
		C_RenderViewport*			m_pViewport;
		C_ManipulatorManager*		m_pToolManager;
		I_ManipulatorContext*		m_pContext;
		E_ManipulatorContextType	m_previousContext;
		E_ManipulatorSpace			m_space;

		static U32					m_screenSize;
		DECLARE_EVENT_TABLE()
	};
}

#endif