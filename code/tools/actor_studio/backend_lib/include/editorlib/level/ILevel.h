#ifndef EDITOR_LIB_ILEVEL_H
#define EDITOR_LIB_ILEVEL_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

class I_Level
{
public:
	virtual ~I_Level() {};

	struct C_Flags
	{
		bool isHidden : 1;
		bool isReadOnly : 1;
		bool isOpen : 1;
		bool pendingSync : 1;
	};

	virtual E_LevelType GetType() const = 0;

	virtual C_Status Load() = 0;
	virtual void Unload() = 0;
	
protected:
	virtual C_Status Initialise() = 0;
	virtual C_Status LoadData() = 0;
	virtual C_Status UnloadData() = 0;

public:
	virtual const C_AftkLevelRef* AddAftkRef(const Btl::String& filepath) = 0;
	virtual C_Status RemoveAftkRef(const Btl::String& filepath) = 0;
	virtual const C_AftkLevelRef* GetAftkRef(const Btl::String& filepath) const = 0;
	virtual const AftkLevelRefList& GetAftkRefs() const = 0;

	virtual C_SceneNode* GetRootSceneNode() = 0;
	virtual const C_SceneNode* GetRootSceneNode() const = 0;

	virtual void SetName(const Btl::String& name) = 0;
	virtual const Btl::String& GetName() const = 0;

	virtual void BuildActorNodeArray(Btl::Vector<const I_DgNode*>& nodes) const = 0;

	virtual bool ContainsNode(const C_NodeId& id) const = 0;

	virtual const C_Flags& GetFlags() const = 0;

	virtual void OnAttachDgNode(I_DgNode* pNode) = 0;
	virtual void OnDetachDgNode(I_DgNode* pNode) = 0;

	virtual bool IsHidden() const = 0;
	virtual void SetHidden(bool hidden) = 0;

	virtual bool IsReadOnly() const = 0;
	virtual void SetReadOnly(bool readOnly) = 0;
};

}

#endif