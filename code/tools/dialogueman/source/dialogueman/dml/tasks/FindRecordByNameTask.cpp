#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/FindRecordByNameTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <Dialogueman/Misc/Utils.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_FindRecordByNameTask::IN_PACK_NAME		= "inPackName";
const wxString C_FindRecordByNameTask::IN_RECORD_NAME	= "inRecordName";
const wxString C_FindRecordByNameTask::OUT_RECORD_PTR	= "outRecordPtr";
const wxString C_FindRecordByNameTask::OUT_RECORD_XTID	= "outRecordXtId";

C_FindRecordByNameTask::C_FindRecordByNameTask(const wxString& name) : 
	C_Task(name)
{}

C_FindRecordByNameTask::~C_FindRecordByNameTask()
{}

bool C_FindRecordByNameTask::Execute()
{
	if(!GetInput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No input assigned", GetName());
		return false;
	}
	if(!GetOutput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No output assigned", GetName());
		return false;
	}
	
	wxString packName   = GetInputVariable(IN_PACK_NAME);
	wxString recordName = GetInputVariable(IN_RECORD_NAME);

	if(packName == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid pack name", GetName());
		return false;	
	}
	if(recordName == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid record name", GetName());
		return false;	
	}

	Xt::DomPack* pPack = NULL;
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(wxGetApp().GetDb()->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pThePack = packIt;
		if (!packName.CmpNoCase((const char *)pThePack->name))
		{
			pPack = pThePack;
			break;
		}
	}

	if(!pPack)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Cannot find pack with name %s", GetName(), packName);
		return false;
	}

	Xt::DomRecord* pRecord = FindRecordByName(pPack, recordName);
			
	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());

	if(pRecord)
	{
		pOutVar->Add(new C_SimpleVariable(OUT_RECORD_PTR,  (void*)pRecord));
		pOutVar->Add(new C_SimpleVariable(OUT_RECORD_XTID, wxString::Format("%08X", pRecord->id) ));
	}
	else
	{
		pOutVar->Add(new C_SimpleVariable(OUT_RECORD_PTR,  NULL));
		pOutVar->Add(new C_SimpleVariable(OUT_RECORD_XTID, wxString("0")));
	}
																		
	GetOutput()->Add(pOutVar);

	return true;
}

} //namespace Dml
} //namespace DMan
