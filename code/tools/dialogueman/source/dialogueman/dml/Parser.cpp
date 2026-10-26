#include "stdafx.h"

#include "system/xml/Xml.h"

#include <DialogueMan/Dml/Parser.h>
#include <DialogueMan/Dml/Variable.h>
#include <DialogueMan/Dml/Task.h>
#include <DialogueMan/Dml/Tasks/AcquireArrayInRecordTask.h>
#include <DialogueMan/Dml/Tasks/AcquireFieldInComponentTask.h>
#include <DialogueMan/Dml/Tasks/AcquireFieldInBlockTask.h>
#include <DialogueMan/Dml/Tasks/AcquireFieldInRecordTask.h>
#include <DialogueMan/Dml/Tasks/AcquireBlockInComponentTask.h>
#include <DialogueMan/Dml/Tasks/AcquireComponentInArrayTask.h>
#include <DialogueMan/Dml/Tasks/AcquireFieldInArrayTask.h>
#include <DialogueMan/Dml/Tasks/AppendTask.h>
#include <DialogueMan/Dml/Tasks/ForTask.h>
#include <DialogueMan/Dml/Tasks/IfTask.h>
#include <DialogueMan/Dml/Tasks/MainTask.h>
#include <DialogueMan/Dml/Tasks/NewComponentTask.h>
#include <DialogueMan/Dml/Tasks/NewFieldTask.h>
#include <DialogueMan/Dml/Tasks/NewRecordTask.h>
#include <DialogueMan/Dml/Tasks/FindRecordByNameTask.h>
#include <DialogueMan/Dml/Tasks/RenameRecordTask.h>
#include <DialogueMan/Dml/Tasks/RefTask.h>
#include <DialogueMan/Dml/Tasks/SetComponentTypeTask.h>
#include <DialogueMan/Dml/Tasks/SetFieldValueTask.h>
#include <DialogueMan/Dml/Tasks/SetTask.h>
#include <DialogueMan/Dml/Tasks/AddTask.h>
#include <DialogueMan/Dml/Tasks/SubTask.h>
#include <DialogueMan/Dml/Tasks/MulTask.h>
#include <DialogueMan/Dml/Tasks/DivTask.h>
#include <DialogueMan/Dml/Tasks/AcquireArrayInComponentTask.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

/////////////////////////// C_Parser \\\\\\\\\\\\\\\\\\\\\\\\\\\

const wxString C_Parser::GLOBALS_NAME	= "__GLOBALS";
const wxString C_Parser::MAIN_NAME		= "__MAIN";
	
C_Parser::C_Parser()
{}

bool C_Parser::Parse(const wxString& dmlFilePath, C_Task** pTask, C_Variable** pVariable)
{
	C_Variable* pResult = NULL;

	Sys::C_XmlDom dom;
	if (!dom.Read(dmlFilePath))
	{
		LOGERROR(wxGetApp().GetLogger(), "Unable to read file %s", dmlFilePath);
		return false;
	}

	if(dom.GetRootNodePtr()->GetChildren().Size() == 1)
	{
		const Sys::C_XmlDomBaseNode* pRootNode = dom.GetRootNodePtr()->GetChildren()[0];
		if ((pRootNode->GetName() == "dml") && (pRootNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pRootElement = (Sys::C_XmlDomElement*)pRootNode;
			
			for (int childIdx = 0; childIdx < pRootElement->GetChildren().Size(); childIdx++)
			{
				const Sys::C_XmlDomBaseNode* pChildNode = pRootElement->GetChildren()[childIdx];

				if((pChildNode->GetName() == "globals") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
				{
					Sys::C_XmlDomElement* pVarSetElement = (Sys::C_XmlDomElement*)pChildNode;
					
					*pVariable = new C_CompositeVariable(GLOBALS_NAME); 
					(*pVariable)->SetContext(*pVariable); 
					ProcessVariableChildren(pVarSetElement, *pVariable);

					//Add System Variables
					C_CompositeVariable* pRowsVar = new C_CompositeVariable("_rows");
					(*pVariable)->Add(pRowsVar);
				}
				else if ((pChildNode->GetName() == "main") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
				{
					if(!*pVariable)
					{
						LOGERROR(wxGetApp().GetLogger(), "Global variables not defined");
						return false;
					}

					Sys::C_XmlDomElement* pMainTaskElement = (Sys::C_XmlDomElement*)pChildNode;

					*pTask = new C_MainTask(MAIN_NAME);
					ProcessTaskChildren(pMainTaskElement, *pTask, *pVariable);
				}
				else
				{
					LOGERROR(wxGetApp().GetLogger(), "DML file not well formed");
					return false;	
				}
			}
		}
		else
		{
			LOGERROR(wxGetApp().GetLogger(), "DML file not well formed");
			return false;	
		}
	}

	if(!*pVariable || !*pTask)
	{
		LOGERROR(wxGetApp().GetLogger(), "DML file not well formed");
		return false;
	}
	
	return true;
}

void C_Parser::ProcessTaskChildren(Sys::C_XmlDomElement* pParent, C_Task* pParentTask, C_Variable* pGlobals)
{
	for (int childIdx = 0; childIdx < pParent->GetChildren().Size(); childIdx++)
	{
		const Sys::C_XmlDomBaseNode* pChildNode = pParent->GetChildren()[childIdx];

		//Simple Tasks
		if ((pChildNode->GetName() == "new_record") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName   = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString packName   = pChildElement->GetAttributes().GetValueByName("packName");
			wxString recordName = pChildElement->GetAttributes().GetValueByName("recordName");
			wxString recordType = pChildElement->GetAttributes().GetValueByName("recordType");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("new_record_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_NewRecordTask::IN_PACK_NAME, packName));
			pVars->Add(new C_SimpleVariable(C_NewRecordTask::IN_RECORD_NAME, recordName));
			pVars->Add(new C_SimpleVariable(C_NewRecordTask::IN_RECORD_TYPE, recordType));

			//Create Task
			C_NewRecordTask* pTask = new C_NewRecordTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "find_record_by_name") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName   = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString packName   = pChildElement->GetAttributes().GetValueByName("packName");
			wxString recordName = pChildElement->GetAttributes().GetValueByName("recordName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("find_record_by_name_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_FindRecordByNameTask::IN_PACK_NAME, packName));
			pVars->Add(new C_SimpleVariable(C_FindRecordByNameTask::IN_RECORD_NAME, recordName));

			//Create Task
			C_FindRecordByNameTask* pTask = new C_FindRecordByNameTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "rename_record") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName		= pChildElement->GetAttributes().GetValueByName("taskName");
			wxString recordPtr		= pChildElement->GetAttributes().GetValueByName("recordPtr");
			wxString newRecordName  = pChildElement->GetAttributes().GetValueByName("newRecordName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("rename_record_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_RenameRecordTask::IN_RECORD_PTR, recordPtr));
			pVars->Add(new C_SimpleVariable(C_RenameRecordTask::IN_NEW_RECORD_NAME, newRecordName));

			//Create Task
			C_RenameRecordTask* pTask = new C_RenameRecordTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "set") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString varName  = pChildElement->GetAttributes().GetValueByName("varName");
			wxString value	  = pChildElement->GetAttributes().GetValueByName("value");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("set_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_SetTask::IN_VAR_NAME, varName ));
			pVars->Add(new C_SimpleVariable(C_SetTask::IN_VALUE, value ));

			//Create Task
			C_SetTask* pTask = new C_SetTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "add") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString varName  = pChildElement->GetAttributes().GetValueByName("varName");
			wxString value	  = pChildElement->GetAttributes().GetValueByName("value");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("add_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AddTask::IN_VAR_NAME, varName ));
			pVars->Add(new C_SimpleVariable(C_AddTask::IN_VALUE, value ));

			//Create Task
			C_AddTask* pTask = new C_AddTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "sub") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString varName  = pChildElement->GetAttributes().GetValueByName("varName");
			wxString value	  = pChildElement->GetAttributes().GetValueByName("value");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("sub_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_SubTask::IN_VAR_NAME, varName ));
			pVars->Add(new C_SimpleVariable(C_SubTask::IN_VALUE, value ));

			//Create Task
			C_SubTask* pTask = new C_SubTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "mul") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString varName  = pChildElement->GetAttributes().GetValueByName("varName");
			wxString value	  = pChildElement->GetAttributes().GetValueByName("value");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("sub_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_MulTask::IN_VAR_NAME, varName ));
			pVars->Add(new C_SimpleVariable(C_MulTask::IN_VALUE, value ));

			//Create Task
			C_MulTask* pTask = new C_MulTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "div") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString varName  = pChildElement->GetAttributes().GetValueByName("varName");
			wxString value	  = pChildElement->GetAttributes().GetValueByName("value");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("sub_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_DivTask::IN_VAR_NAME, varName ));
			pVars->Add(new C_SimpleVariable(C_DivTask::IN_VALUE, value ));

			//Create Task
			C_DivTask* pTask = new C_DivTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "ref") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString varName  = pChildElement->GetAttributes().GetValueByName("varName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("ref_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_RefTask::IN_VAR_NAME, varName ));
			
			//Create Task
			C_RefTask* pTask = new C_RefTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "acquire_array_in_record") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName  = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString recordPtr = pChildElement->GetAttributes().GetValueByName("recordPtr");
			wxString arrayName = pChildElement->GetAttributes().GetValueByName("arrayName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("acquire_array_in_record_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AcquireArrayInRecordTask::IN_RECORD_PTR, recordPtr ));
			pVars->Add(new C_SimpleVariable(C_AcquireArrayInRecordTask::IN_ARRAY_NAME, arrayName ));

			//Create Task
			C_AcquireArrayInRecordTask* pTask = new C_AcquireArrayInRecordTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "acquire_array_in_component") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName  = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString componentPtr = pChildElement->GetAttributes().GetValueByName("componentPtr");
			wxString arrayName = pChildElement->GetAttributes().GetValueByName("arrayName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("acquire_array_in_component_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AcquireArrayInComponentTask::IN_COMPONENT_PTR, componentPtr ));
			pVars->Add(new C_SimpleVariable(C_AcquireArrayInComponentTask::IN_ARRAY_NAME, arrayName ));

			//Create Task
			C_AcquireArrayInComponentTask* pTask = new C_AcquireArrayInComponentTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "acquire_field_in_component") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName     = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString componentPtr = pChildElement->GetAttributes().GetValueByName("componentPtr");
			wxString fieldName	  = pChildElement->GetAttributes().GetValueByName("fieldName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("acquire_field_in_component_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AcquireFieldInComponentTask::IN_COMPONENT_PTR, componentPtr ));
			pVars->Add(new C_SimpleVariable(C_AcquireFieldInComponentTask::IN_FIELD_NAME, fieldName ));

			//Create Task
			C_AcquireFieldInComponentTask* pTask = new C_AcquireFieldInComponentTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "acquire_component_in_array") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName     = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString arrayPtr = pChildElement->GetAttributes().GetValueByName("arrayPtr");
			wxString index	  = pChildElement->GetAttributes().GetValueByName("index");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("acquire_component_in_array_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AcquireComponentInArrayTask::IN_ARRAY_PTR, arrayPtr ));
			pVars->Add(new C_SimpleVariable(C_AcquireComponentInArrayTask::IN_INDEX, index ));

			//Create Task
			C_AcquireComponentInArrayTask* pTask = new C_AcquireComponentInArrayTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "acquire_field_in_array") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName     = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString arrayPtr = pChildElement->GetAttributes().GetValueByName("arrayPtr");
			wxString index	  = pChildElement->GetAttributes().GetValueByName("index");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("acquire_field_in_array_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AcquireFieldInArrayTask::IN_ARRAY_PTR, arrayPtr ));
			pVars->Add(new C_SimpleVariable(C_AcquireFieldInArrayTask::IN_INDEX, index ));

			//Create Task
			C_AcquireFieldInArrayTask* pTask = new C_AcquireFieldInArrayTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "acquire_field_in_record") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName  = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString recordPtr = pChildElement->GetAttributes().GetValueByName("recordPtr");
			wxString fieldName = pChildElement->GetAttributes().GetValueByName("fieldName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("acquire_field_in_record_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AcquireFieldInRecordTask::IN_RECORD_PTR, recordPtr ));
			pVars->Add(new C_SimpleVariable(C_AcquireFieldInRecordTask::IN_FIELD_NAME, fieldName ));

			//Create Task
			C_AcquireFieldInRecordTask* pTask = new C_AcquireFieldInRecordTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "acquire_field_in_block") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName  = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString blockPtr  = pChildElement->GetAttributes().GetValueByName("blockPtr");
			wxString fieldName = pChildElement->GetAttributes().GetValueByName("fieldName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("acquire_field_in_block_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AcquireFieldInBlockTask::IN_BLOCK_PTR, blockPtr ));
			pVars->Add(new C_SimpleVariable(C_AcquireFieldInBlockTask::IN_FIELD_NAME, fieldName ));

			//Create Task
			C_AcquireFieldInBlockTask* pTask = new C_AcquireFieldInBlockTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "acquire_block_in_component") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName     = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString componentPtr = pChildElement->GetAttributes().GetValueByName("componentPtr");
			wxString blockName	  = pChildElement->GetAttributes().GetValueByName("blockName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("acquire_block_in_component_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AcquireBlockInComponentTask::IN_COMPONENT_PTR, componentPtr ));
			pVars->Add(new C_SimpleVariable(C_AcquireBlockInComponentTask::IN_BLOCK_NAME, blockName ));

			//Create Task
			C_AcquireBlockInComponentTask* pTask = new C_AcquireBlockInComponentTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "new_component") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName  = pChildElement->GetAttributes().GetValueByName("taskName");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("new_component_vars");
			pVars->SetContext(pVars);

			//Create Task
			C_NewComponentTask* pTask = new C_NewComponentTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "append") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName  = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString arrayPtr  = pChildElement->GetAttributes().GetValueByName("arrayPtr");
			wxString elementPtr  = pChildElement->GetAttributes().GetValueByName("elementPtr");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("append_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_AppendTask::IN_ARRAY_PTR, arrayPtr ));
			pVars->Add(new C_SimpleVariable(C_AppendTask::IN_ELEMENT_PTR, elementPtr ));

			//Create Task
			C_AppendTask* pTask = new C_AppendTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "set_component_type") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName		= pChildElement->GetAttributes().GetValueByName("taskName");
			wxString componentPtr   = pChildElement->GetAttributes().GetValueByName("componentPtr");
			wxString componentType  = pChildElement->GetAttributes().GetValueByName("componentType");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("set_component_type_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_SetComponentTypeTask::IN_COMPONENT_PTR, componentPtr ));
			pVars->Add(new C_SimpleVariable(C_SetComponentTypeTask::IN_COMPONENT_TYPE, componentType ));

			//Create Task
			C_SetComponentTypeTask* pTask = new C_SetComponentTypeTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "set_field_value") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName	= pChildElement->GetAttributes().GetValueByName("taskName");
			wxString fieldPtr   = pChildElement->GetAttributes().GetValueByName("fieldPtr");
			wxString fieldValue = pChildElement->GetAttributes().GetValueByName("fieldValue");
			
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("set_field_value_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_SetFieldValueTask::IN_FIELD_PTR, fieldPtr ));
			pVars->Add(new C_SimpleVariable(C_SetFieldValueTask::IN_FIELD_VALUE, fieldValue ));

			//Create Task
			C_SetFieldValueTask* pTask = new C_SetFieldValueTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}
		else if ((pChildNode->GetName() == "new_field") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName	= pChildElement->GetAttributes().GetValueByName("taskName");
				
			//Set Variables
			C_CompositeVariable* pVars =  new C_CompositeVariable("new_field_vars");
			pVars->SetContext(pVars);

			//Create Task
			C_NewFieldTask* pTask = new C_NewFieldTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
		}

		//Composite Tasks
		else if((pChildNode->GetName() == "for") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString items = pChildElement->GetAttributes().GetValueByName("items");
			wxString itemVarName = pChildElement->GetAttributes().GetValueByName("itemVarName");
						
			//Set Variables
			C_CompositeVariable* pVars = new C_CompositeVariable("for_vars");
			pVars->SetContext(pVars);

			pVars->Add(new C_SimpleVariable(C_ForTask::IN_ITEMS, items ));
			pVars->Add(new C_SimpleVariable(C_ForTask::IN_ITEM_VAR_NAME, itemVarName ));
						
			//Create Task
			C_ForTask* pTask = new C_ForTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
			ProcessTaskChildren(pChildElement, pTask, pGlobals); 
		}
		else if((pChildNode->GetName() == "if") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			//Get Attributes
			wxString taskName = pChildElement->GetAttributes().GetValueByName("taskName");
			wxString comp = pChildElement->GetAttributes().GetValueByName("comp");
			wxString arg1 = pChildElement->GetAttributes().GetValueByName("arg1");
			wxString arg2 = pChildElement->GetAttributes().GetValueByName("arg2");

			//Set Variables
			C_CompositeVariable* pVars = new C_CompositeVariable("if_vars");
			pVars->SetContext(pVars);
			
			pVars->Add(new C_SimpleVariable(C_IfTask::IN_COMP, comp));
			pVars->Add(new C_SimpleVariable(C_IfTask::IN_ARG1, arg1));
			pVars->Add(new C_SimpleVariable(C_IfTask::IN_ARG2, arg2));

			//Create Task
			C_IfTask* pTask = new C_IfTask(taskName);
			
			pTask->SetGlobals(pGlobals);
			pTask->SetInput(pVars, true);
			pTask->SetOutput(pGlobals, false);

			pParentTask->Add(pTask);
			ProcessTaskChildren(pChildElement, pTask, pGlobals);
		}
		else
		{
			LOGERROR(wxGetApp().GetLogger(), "DML file not well formed");
		}
	}
}

void C_Parser::ProcessVariableChildren(Sys::C_XmlDomElement* pParent, C_Variable* pParentVariable)
{
	for (int childIdx = 0; childIdx < pParent->GetChildren().Size(); childIdx++)
	{
		const Sys::C_XmlDomBaseNode* pChildNode = pParent->GetChildren()[childIdx];

		//Simple Variables
		if ((pChildNode->GetName() == "var") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			wxString name  = pChildElement->GetAttributes().GetValueByName("name");
			wxString value = pChildElement->GetAttributes().GetValueByName("value");
			C_SimpleVariable* pSimpleVar = new C_SimpleVariable(name, value);

			pParentVariable->Add(pSimpleVar);
		}

		//Composite Variables
		else if((pChildNode->GetName() == "varset") && (pChildNode->GetType() == Sys::E_XmlDomNodeType_Element))
		{
			Sys::C_XmlDomElement* pChildElement = (Sys::C_XmlDomElement*)pChildNode;

			wxString name  = pChildElement->GetAttributes().GetValueByName("name");
			C_CompositeVariable* pCompositeVar = new C_CompositeVariable(name); 
			pParentVariable->Add(pCompositeVar);
			ProcessVariableChildren(pChildElement, pCompositeVar);
		}

		else
		{
			LOGERROR(wxGetApp().GetLogger(), "DML file not well formed");
		}
	}
}

} //namespace Dml
} //namespace DMan