using System;
using System.Collections.Generic;
using System.Text;
using Alienbrain;
using System.Windows.Forms;
using System.Xml;
using Global;

namespace file_tag_manager_2
{
	public class ListViewCommand
	{
		private Command m_command = Command.Nothing;
		private object m_object = null;

		private ListViewCommand(Command command, object o)
		{
			m_command = command;
			m_object = o;
		}

		static public ListViewCommand CreateCommandListViewObjectAddFileData(ListViewObject lvo)
		{
		    return new ListViewCommand(Command.ListViewObjectAddFileData, lvo);
		}

		static public ListViewCommand CreateCommandPrepare(string strPath)
		{
		    return new ListViewCommand(Command.Prepare, strPath);
		}

		static public ListViewCommand CreateCommandEndPrepare()
		{
		    return new ListViewCommand(Command.EndPrepare, null);
		}

		private enum Command
		{
			Nothing = 0,
			Prepare,
			ListViewObjectAddFileData,
			EndPrepare
		}

		private enum ListViewObjectState
		{
			PotentiallyUnwanted = 0,
			Active
		};

		public void DoCommand(ListViewManager listViewManager)
		{
			//switch(m_command)
			//{
			//    case Command.Nothing:
			//        break;

			//    case Command.Prepare:
			//        string strPath = m_object as string;
			//        string strDocumentFilename = strPath + ListViewManager.TAG_FILENAME;

			//        listViewManager.m_xDocument = listViewManager.LoadTagDataFile(strDocumentFilename);

			//        listViewManager.DoPrepareToPopulateListView();

			//        break;

			//    case Command.ListViewObjectAddFileData:
			//        ListViewObject lvo = m_object as ListViewObject;
			//        listViewManager.ListViewObjectAddFileData(lvo);

			//        if(lvo.m_xFileData != null)
			//        {
			//            XmlAttribute attr = lvo.m_xFileData.OwnerDocument.CreateAttribute("keep");
			//            lvo.m_xFileData.Attributes.SetNamedItem(attr);
			//        }

			//        break;

			//    case Command.EndPrepare:
			//        listViewManager.DoEndPrepareListView();
			//        break;

			//    default:
			//        break;
			//}
		}
	}

}
