#include "stdafx.h"

#include <BasicExcel/BasicExcel.h>

#include <Tools_Shared/Utility/MacroExpander.h>

#include <DialogueMan/ImportExport/ImportWizard.h>
#include <DialogueMan/ImportExport/Import.h>
#include <DialogueMan/ImportExport/ExcelReader.h>

#include <DialogueMan/Diff/Diff.h>
#include <DialogueMan/Diff/DiffViewer.h>

#include <DialogueMan/Document/Rowset.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Misc/ViewDisabler.h>

#include <DialogueMan/Actions/AddRows.h>
#include <DialogueMan/Actions/ChangeRows.h>
#include <DialogueMan/Actions/DeleteRows.h>

#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/App.h>

#include <wx/progdlg.h>

namespace DMan
{

bool Import::Do(C_Frame* pFrame)
{
	bool retVal = false;

	C_ViewDisabler viewDisabler;

	C_ImportWizard importWizard(pFrame);
	if( importWizard.RunWizard(&importWizard.GetPage1()) ) //user pressed Finish
	{
		wxBeginBusyCursor();
		C_Selection selection;
		wxGetApp().GetSelection(selection);

		C_RowSet* pRowSet = selection.GetCurrentRowSet();
		if (!pRowSet)
			return false;

		C_Reader& reader = importWizard.GetReader();
		retVal = reader.Read();
		Btl::Vector< Btl::Map<wxString, C_MutantValue> >& readRows = reader.GetReadRows();
				
		C_DiffViewer diffViewer(pFrame, reader);
		wxEndBusyCursor();
		
		//If we need to merge...
		if (pRowSet->GetNumRows() != 0 && diffViewer.AnyMatch())
		{
			if (!diffViewer.IsDifferent())
			{
				retVal = true;
				wxMessageDialog msg(
					pFrame, 
					wxT("No differences found between the File and Database data"),  
					wxT("Import Wizard"),
					wxOK | wxICON_INFORMATION);
				msg.ShowModal();
				return retVal;
			}
			
			//Build list of importable columns for this set.
			Btl::Vector<const C_Column*> importCols;
			C_ColumnSet* pColSet = wxGetApp().GetDbManager().GetTypeColumnSet(reader.GetItemType());
			if(pColSet)
			{
				U32 numCols = pColSet->GetNumColumns();
				for(U32 i = 0; i < numCols; ++i)
				{
					C_Column* pCol = pColSet->GetColumn(i);
					if(pCol && pCol->IsImportable())
					{
						importCols.PushBack(pCol);
					}
				}
			}

			//Make the frame behave like a Modal window
			wxModalEventLoop modalLoop(&diffViewer);
			diffViewer.SetModalLoop(&modalLoop);
			diffViewer.Show(true);
			modalLoop.Run();
				
			int returnCode = diffViewer.GetReturnCode();
			if(returnCode == wxID_OK)
			{
				std::vector< C_AlignedData > ad = diffViewer.GetAlignedData();

				Xt::DomRecord* pLocTable = reader.GetLocTable();
				Xt::DomArray* pStringArray = Xt::AsArray(Xt::At(&pLocTable->root.data, "strings"));

				//Look for all accepted changes
				for(size_t i = 0; i < ad.size(); ++i)
				{
					if(ad[i].accepted)
					{
						switch(ad[i].status)
						{
						case C_AlignedData::Status_Added:
							{
								//set the row to add
								Btl::Map<wxString, C_MutantValue> row;
								for(S32 key = 0; key < importCols.Size(); ++key)
								{
									
									row[importCols[key]->GetKey()] = ad[i].leftLine[key];
								}
									  
								//look for the previous proper index
								int prevIndex = -1;
								for(int j = i - 1; j >= 0; --j)
								{
									if(ad[j].leftIndex != -1)
									{
										prevIndex = ad[j].leftIndex;
										break;
									}
								}
								ad[i].leftIndex = prevIndex + 1;
			
								retVal = AddRows::Add(reader.GetItemType(), Xt::AsArray(pRowSet->GetXtData()), reader.GetLocTable(), row, ad[i].leftIndex); 

								//update the left index of the AlignedData below
								for(size_t k = i + 1; k < ad.size(); ++k)
								{
									if(ad[k].leftIndex != -1)
									{
										++ad[k].leftIndex;
									}
								}
								
								break;
							}
						case C_AlignedData::Status_Deleted:
							{
								//Delete the record
								if(retVal = DeleteRows::Delete(Xt::AsArray(pRowSet->GetXtData()), ad[i].leftIndex))
								{
									//invalidate left index
									ad[i].leftIndex = -1;

									//update the left index of the AlignedData below
									for(size_t k = i + 1; k < ad.size(); ++k)
									{
										if(ad[k].leftIndex != -1)
										{
											--ad[k].leftIndex;
										}
									}
								}
											
								break;
							}
						case C_AlignedData::Status_Changed:
							{
								//set the row to change
								Btl::Map<wxString, C_MutantValue> row;
								for(S32 key = 0; key < importCols.Size(); ++key)
								{
									
									row[importCols[key]->GetKey()] = ad[i].leftLine[key];
								}

								retVal = ChangeRows::Change(reader.GetItemType(), Xt::AsArray(pRowSet->GetXtData()), row, ad[i].leftIndex);
						
								break;
							}
						}
					}
				}
			} 
			else if(returnCode == wxID_CANCEL || returnCode == wxID_CLOSE)
			{
				return true;
			}
		}
		else //Append all read rows
		{
			Xt::DomRecord* pLocTable = reader.GetLocTable();
			Xt::DomArray* pStringArray = Xt::AsArray(Xt::At(&pLocTable->root.data, "strings"));
			int itemsToProcess = readRows.Size();
			int itemsProcessed = 0;

			wxProgressDialog progress(_T("Importing Dialogue Lines..."),
									_T("Importing Dialogue Lines"),
									itemsToProcess, // range
									NULL,  // parent
									wxPD_APP_MODAL |
									wxPD_ELAPSED_TIME |
									wxPD_ESTIMATED_TIME |
									wxPD_REMAINING_TIME
									);

			for(int i = 0; i < itemsToProcess; ++i)
			{
				progress.Update(itemsProcessed, wxString::Format("Line %d of %d", itemsProcessed, itemsToProcess));
				retVal = AddRows::Add(reader.GetItemType(), Xt::AsArray(pRowSet->GetXtData()), reader.GetLocTable(), readRows[i]);
				++itemsProcessed;
			}
		}
	}
	else //User pressed Cancel
	{
		return true;
	}

	if(retVal)
	{
		LOGINFO(wxGetApp().GetLogger(), "File  succesfully imported.\n");
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Error while trying to import file.\n");
	}

	return retVal;
}

} //namespace DMan
