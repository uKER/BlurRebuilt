#ifndef EDITOR_CAMERA_CONTROLLER_H
#define EDITOR_CAMERA_CONTROLLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "sigslot/SigSlot.h"

namespace EditorLib
{
	class C_DgAnimationPropNode;
	class I_UpdateHint;
}

namespace Editor
{
	struct CameraMessage;
	class C_RenderViewport;

	class I_CameraController
	{
	public:
		virtual ~I_CameraController() { };
		
		//called whenever messages are recieved from the game
		virtual void Recieve(const CameraMessage* msg) = 0;

		//called on timer by camera connection & at the start of every frame by the viewport manager
		//returns value indicates whether a message broadcast is needed
		virtual bool Send(CameraMessage* msg) = 0;

		//called at the start of every frame
		virtual void Update() = 0;
	};


	class C_SyncCamera : public I_CameraController
	{
	public:
		C_SyncCamera(C_RenderViewport *pViewport);
		~C_SyncCamera();

		void Recieve(const CameraMessage* msg);
		bool Send(CameraMessage* msg);
		void Update();


	private:
		struct CameraState
		{
			AftkRender::C_Camera	camera;
			wxString				label;
		};

		C_RenderViewport*	m_pViewport;
		CameraState			m_camState;
		CameraMessage*		m_pLastMessage;
		Time::C_Time		m_lastMessageTime;
	};


	class C_DgNodeCamera : public I_CameraController
	{
	public:
		C_DgNodeCamera(C_RenderViewport *pViewport, const EditorLib::I_DgNode* pDgNode);
		~C_DgNodeCamera();

		void Recieve(const CameraMessage* msg);
		bool Send(CameraMessage* msg);
		void Update();

		virtual void DgNodeDeserialised(const EditorLib::I_DgNode& dgNode);
		virtual void DgNodeDestroy(const EditorLib::I_DgNode& dgNode);

	private:
		//slot callbacks
		void OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode);
		void OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode);

	protected:

		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_DgNodeCamera *pController);
			
			void Connect();
			void Disconnect();

			void OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc);
			void OnDgNodeDestroySlot(const EditorLib::I_DgNode& dgNode);

		private:
			C_DgNodeCamera	*m_pController;
		};

		struct CameraState
		{
			AftkRender::C_Camera	camera;
			wxString				label;
		};

		C_RenderViewport*	m_pViewport;
		CameraState			m_camState;
		CameraMessage*		m_pLastMessage;
		Time::C_Time		m_lastMessageTime;
		C_Slots				m_slots;

		const EditorLib::I_DgNode*	m_pDgNode;
	};

	class C_DgAnimationPropNodeCamera : public C_DgNodeCamera
	{
	public:
		C_DgAnimationPropNodeCamera(C_RenderViewport *pViewport, const EditorLib::C_DgAnimationPropNode *pDgNode, U32 component);

		void Update();

		void DgNodeDeserialised(const EditorLib::I_DgNode& dgNode);

	protected:
		U32 m_componentId;
	};
}

#endif