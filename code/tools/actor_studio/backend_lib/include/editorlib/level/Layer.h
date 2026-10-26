#ifndef EDITOR_LIB_LAYER_H
#define EDITOR_LIB_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Set.h>
#include <Btl/Utility/String.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Xt/XtObject.h>

namespace EditorLib
{
	/*class I_LayerOwner
	{
		E_LayerOwner GetType();
	};*/

	class I_Layer
	{
	public:
		typedef Btl::Set<I_DgNode*> DgNodeSet;

		struct Flags
		{
			union
			{
				struct
				{
					U32 visible		: 1;
					U32 selectable	: 1;
				};
				U32 value;
			};
		};

	public:
		I_Layer(I_LayerSet* layerSet, I_Level* level);
		virtual ~I_Layer();

		I_LayerSet* GetLayerSet();

		const I_Level* GetLevel() const;
		I_Level* GetLevel();

		virtual E_LayerType GetType() const;

		virtual void SetName(const Btl::String& name);
		const Btl::String& GetName() const;

		const DgNodeSet& GetNodes() const;

		virtual void SetDisplayColour(const Maths::C_RGBA_U8& colour) = 0;
		Maths::C_RGBA_U8 GetDisplayColour() const;

		virtual void SetLayerFlags(const Flags& flags);
		const Flags& GetLayerFlags() const;

		virtual bool IsReadOnly() const = 0;

	protected:
		virtual void AddNode(I_DgNode *pNode);
		virtual void RemoveNode(const I_DgNode *pNode);

		I_LayerSet*			m_LayerSet;
		I_Level*			m_Level;
		Btl::String			m_Name;
		DgNodeSet			m_DgNodes;
		Maths::C_RGBA_U8	m_DisplayColour;
		Flags				m_Flags;

		FRIEND_CLASS I_LayerSet;
		FRIEND_CLASS C_LayerManager;
	};

	class C_Layer : public I_Layer
	{
	public:
		C_Layer(I_LayerSet* layerSet, I_Level* level);
		virtual ~C_Layer();

		// TODO : Move implementation from I_Layer into here
	};

	class C_RefLayer : public C_Layer
	{
	public:
		C_RefLayer(I_LayerSet* layerSet, I_Level* level);
		virtual ~C_RefLayer();

		virtual E_LayerType GetType() const;

		virtual void SetDisplayColour(const Maths::C_RGBA_U8& colour);

		virtual bool IsReadOnly() const { return true; }

	protected:
		virtual void AddNode(I_DgNode *pNode);
	};



	class C_XtActorLayer : public C_Layer, public C_XtObject
	{
	public:

	public:
		C_XtActorLayer(I_LayerSet* layerSet, I_Level* level);
		virtual ~C_XtActorLayer();

		virtual E_LayerType GetType() const;

		virtual void SetName(const Btl::String& name);

		virtual void SetDisplayColour(const Maths::C_RGBA_U8& colour);

		C_Status XtInitialise(
			C_XtLevel* pLevel,
			LarvaType* pLarvaType,
			Xt::DomElement* pData);

		void XtDeserialise(Xt::DomElement* pFrom = 0);

		virtual bool IsReadOnly() const;

	protected:
		virtual void AddNode(I_DgNode *pNode);
		virtual void RemoveNode(const I_DgNode *pNode);

		bool XtDeserialiseName(Xt::DomElement* pFrom = 0);
		bool XtDeserialiseColour(Xt::DomElement* pFrom = 0);

		void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor,
			Xt::DomEvent* pEvent,
			bool typeChanged);

		void OnXtActorArrayAdd(S32 index);
		void OnXtActorArrayRemove();
	
		Xt::DomMonitor		m_xtActorArrayMonitor;

	};

}

#endif