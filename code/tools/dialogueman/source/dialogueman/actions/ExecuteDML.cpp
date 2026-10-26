#include "stdafx.h"

#include <DialogueMan/Actions/ExecuteDML.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/ViewDisabler.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Misc/MacroExpander.h>

#include <DialogueMan/App.h>

#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>

#include <System/Misc/StringUtil.h>
#include <tools_shared/utility/MacroExpander.h>

#include <DialogueMan/Dml/Variable.h>
#include <DialogueMan/Dml/Task.h>
#include <DialogueMan/Dml/Parser.h>
#include <DialogueMan/Dml/Tasks/AcquireArrayInRecordTask.h>
#include <DialogueMan/Dml/Tasks/AcquireFieldInComponentTask.h>
#include <DialogueMan/Dml/Tasks/AppendTask.h>
#include <DialogueMan/Dml/Tasks/ForTask.h>
#include <DialogueMan/Dml/Tasks/IfTask.h>
#include <DialogueMan/Dml/Tasks/MainTask.h>
#include <DialogueMan/Dml/Tasks/NewComponentTask.h>
#include <DialogueMan/Dml/Tasks/NewFieldTask.h>
#include <DialogueMan/Dml/Tasks/NewRecordTask.h>
#include <DialogueMan/Dml/Tasks/SetComponentTypeTask.h>
#include <DialogueMan/Dml/Tasks/SetFieldValueTask.h>
#include <DialogueMan/Dml/Tasks/SetTask.h>
#include <DialogueMan/Dml/Tasks/AcquireArrayInComponentTask.h>

namespace DMan
{

namespace ExecuteDML
{

bool Do(const C_Options& options)
{
	LOGINFO(wxGetApp().GetLogger(), ">>> STARTING TO EXECUTE DML FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());
	
	Util::C_String loadedProject = "%BIZARRE_PROJECT%";
	Tools::C_MacroExpander::Instance().Expand(loadedProject);
	if(loadedProject != "Amax")
	{
		wxMessageDialog msg(NULL, 
			wxT("This tool is only available in Amax"), 
			wxT("Compose Amax Records"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();

		LOGINFO(wxGetApp().GetLogger(), "<<< DONE EXECUTING DML FOR RECORD \"%s\"", 
			options.GetRecord()->GetName().Text());

		return false;	
	}
	
	C_RowSet* pRowSet = options.GetRecord() ->GetRowSet(0);
	if (!pRowSet)
	{
		LOGINFO(wxGetApp().GetLogger(), "<<< DONE EXECUTING DML FOR RECORD \"%s\"", 
			options.GetRecord()->GetName().Text());
		return false;
	}
	
	Dml::C_Parser parser;
	Dml::C_Variable* pVars = NULL;
	Dml::C_Task* pTask = NULL;

	if(!parser.Parse(options.GetDMLFile(), &pTask, &pVars))
	{
		if(pTask)
			delete pTask;

		if(pVars)
			delete pVars;

		LOGINFO(wxGetApp().GetLogger(), "<<< DONE EXECUTING DML FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

		return false;
	}

	Dml::C_Variable& vars = *pVars;
		
	for(S32 rowIdx = 0; rowIdx < pRowSet->GetNumRows(); rowIdx++)
	{
		C_Row* pRow = pRowSet->GetRow(rowIdx);
		Dml::C_CompositeVariable* pRowVar = new Dml::C_CompositeVariable( wxString::Format("%i", rowIdx) );

		Xt::DomComponent* pRowComponent = Xt::AsComponent(pRow->GetXtData());
		Xt::DomBlock* pRowDataBlock = &pRowComponent->data;
		Xt::DomBlock* pTextIdBlock = Xt::AsBlock(Xt::At(pRowDataBlock, "TextId"));

		wxString textRecordVal = Xt::AsText(Xt::At(pTextIdBlock, "record")).Text();
		wxString textIdVal = Xt::AsText(Xt::At(pTextIdBlock, "id")).Text();

		C_ColumnSet* pColSet = pRowSet->GetAllColumnSet();
		for(U32 colIdx = 0; colIdx < pColSet->GetNumColumns(); colIdx++)
		{
			C_Column* pCol = pColSet->GetColumn(colIdx);
			if(pCol)
			{
				const C_Row::ColumnMap& colMap = pRow->GetColumns();
				C_Row::ColumnMap::ConstIterator it = colMap.Find(pCol->GetKey());
				if (it != colMap.End())
				{
					wxString label = pCol->GetLabel();
					wxString value = it->GetValue();
					pRowVar->Add(new Dml::C_SimpleVariable(label, value));
				}
			}
		}

		pRowVar->Add(new Dml::C_SimpleVariable("_componentPtr", (void*)pRowComponent));
		pRowVar->Add(new Dml::C_SimpleVariable("TextRecord", textRecordVal));
		pRowVar->Add(new Dml::C_SimpleVariable("TextId", textIdVal));

		vars["_rows"].Add(pRowVar);
	}  

	pTask->Execute();
	
	if(pTask)
		delete pTask;

	if(pVars)
		delete pVars;


	LOGINFO(wxGetApp().GetLogger(), "<<< DONE EXECUTING DML FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

	return true;
}

} //namespace ExecuteDML

} //namespace DMan