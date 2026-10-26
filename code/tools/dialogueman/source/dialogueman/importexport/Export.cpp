#include "stdafx.h"

#include <BasicExcel/BasicExcel.h>
#include <DialogueMan/ImportExport/ExcelWriter.h>
#include <DialogueMan/ImportExport/ExportWizard.h>
#include <DialogueMan/ImportExport/Export.h>
#include <DialogueMan/Frame/Frame.h>

namespace DMan
{

bool Export::Do(C_Frame* pFrame)
{
	wxBusyCursor wait; //Display an HourGlass cursor
	bool retVal = true;
	
	C_ExportWizard exportWizard(pFrame);
	if( exportWizard.RunWizard(&exportWizard.GetPage1()) ) //user pressed Finish
	{
		//get the expected columns
		C_ExcelWriter& writer = exportWizard.GetExcelWriter();
		 		
		retVal &= writer.CreateExcelDoc();
		retVal &= writer.Write();
		retVal &= writer.SaveExcelDoc();
		retVal &= writer.CloseExcelDoc();
	}
	else //user pressed Cancel
	{
		return true;
	}

	if(retVal)
	{
		wxMessageDialog msg(pFrame, 
			wxT("Excel file succesfully written!\n"), 
			wxT("Export Wizard"),
			wxOK | wxICON_INFORMATION);
		msg.ShowModal();
	}
	else
	{
		wxMessageDialog msg(pFrame, 
			wxT("Error while trying to export to Excel file\n"), 
			wxT("Export Wizard"),
			wxOK | wxICON_ERROR);
		msg.ShowModal();
	}

	return retVal;
}

} //namespace DMan
