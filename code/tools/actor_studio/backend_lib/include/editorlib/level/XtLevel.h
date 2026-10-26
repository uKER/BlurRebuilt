#ifndef EDITOR_LIB_XT_LEVEL_H
#define EDITOR_LIB_XT_LEVEL_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Level/LevelBase.h>
#include <EditorLib/Xt/XtObject.h>

namespace EditorLib
{

class C_AftkRef;

class C_XtLevel : public C_LevelBase, public C_XtObject
{
public:
	C_XtLevel(C_LevelPack* pack);
	virtual ~C_XtLevel();

	virtual E_LevelType GetType() const;

protected:
	virtual C_Status LoadData();
	virtual C_Status UnloadData();

public:
	C_Status SetXtRecord(XtId recordId);

	virtual const C_AftkLevelRef* AddAftkRef(const Btl::String& filepath);
	virtual C_Status RemoveAftkRef(const Btl::String& filepath);
	void UpdateAftkRef(const Btl::String& filePath, bool isExtActor);

	XtId GetXtPackId() const;
	XtId GetXtRecordId() const;

	virtual void SetName(const Btl::String& name);

	virtual void BuildActorNodeArray(Btl::Vector<const I_DgNode*>& nodes) const;

	virtual void OnDetachDgNode(I_DgNode* pNode);

	void OnNodeChangeXtType(const C_NodeId& id);
	C_XtObject* AsXtObject();

protected:
	C_Status XtInitialise();

	virtual C_Status XtInitialise(
		C_XtLevel* pLevel, 
		LarvaType* pLarvaType, 
		Xt::DomElement* pData);

	virtual void OnXtDomMonitorEvent(
		Xt::DomMonitor* pMonitor, 
		Xt::DomEvent* pEvent,
		bool typeChanged);

	void XtInitialiseDgNodes(Xt::DomArray* pArray, E_ObjType nullType);

	void XtInitialiseDgNodes(Xt::DomRecord* pLevelRecord);

	void XtInitialiseAftkRefs(Xt::DomRecord* pLevelRecord);

	bool XtInitialiseAftkRef(C_AftkLevelRef& aftkLevelRef, Xt::DomBlock* pDomBlock);

	Xt::DomBlock* GetAftkRefData(const Btl::String& _filePath);

	void OnXtArrayAdd(Xt::DomArray* pArray, E_ObjType nullType, S32 index);

	void OnXtArrayRemove(Xt::DomArray* pArray, E_ObjType baseFilterType);

	XtId				m_packId;
	XtId				m_recordId;
	Xt::DomMonitor		m_xtActorArrayMonitor;
	Xt::DomMonitor		m_xtNodeArrayMonitor;
	Xt::DomMonitor		m_xtGameDataArrayMonitor;
	Xt::DomMonitor		m_xtAftkRefMonitor;

	C_LevelPack*		m_pPack;

	FRIEND_CLASS		I_DgNode;
};

}

#endif
