#ifndef EDITOR_LIB_ACTOR_HELPERS_H
#define EDITOR_LIB_ACTOR_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "SceneXtActorNode.h"

namespace EditorLib
{
	namespace XtNodeHelpers
	{
		struct C_XtPropertyDesc
		{
			LarvaField*		pLarvaField;
			Xt::DomElement*	pData;
		};

		C_Status InitialiseXtId(Xt::DomComponent* pData);

		I_DgNode* FindNodeByXtId(XtId xtId);

		bool AssumeXtId(C_XtLevel* pLevel, C_NodeId& nodeId, I_DgNode* pNode, XtId xtId, U32 childId, Xt::DomComponent* pComponent = 0);

		void ReleaseXtId(C_NodeId& nodeId, I_DgNode* pNode);

		//Set name, acquiring new field.
		void XtStoreName(
			const Btl::String& proposedName, 
			I_DgNode& dgNode,
			LtStruct* pType,
			Xt::DomBlock* pData);

		void XtInitialiseName(
			Btl::String& existingName, 
			I_DgNode* pDgNode,
			LtStruct* pType,
			Xt::DomBlock* pData);

		//Deserialise name.
		bool XtDeserialiseName(
			Btl::String& existingName, 
			I_DgNode* pDgNode,
			Xt::DomElement* pFrom);

		//initialise local transformation
		//sets plug values if block exists
		//otherwise will initialise block to plug values
		void XtInitialiseTransform(
			C_NullPlug& scale,
			C_NullPlug& shear,
			C_NullPlug& rotate,
			C_NullPlug& translate,
			bool triggerTopLevelParentOnChanged,
			Xt::DomBlock* pFrom);

		//deserialise local transformation
		bool XtDeserialiseTransform(
			C_NullPlug& scale,
			C_NullPlug& shear,
			C_NullPlug& rotate,
			C_NullPlug& translate,
			bool triggerTopLevelParentOnChanged,
			Xt::DomBlock* pFrom);

		void BuildXtActorPropertyList(
			Btl::Vector<C_XtPropertyDesc>& properties,
			LarvaType* pLarvaType, 
			Xt::DomBlock* pRoot,
			bool acquire);

		S32 GetPropertyIndex(const C_DgXtCompNode* pParent, C_DgXtPropNodeBase* pProp);

		bool		IsExternalActor(Xt::DomBlock* pRoot);
		void		SetIsExternalActor(Xt::DomBlock* pRoot, bool isExternalActor);
	}
}

#endif
