using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using SuperControl;
using System.IO;
using System.Threading;
using System.Xml;
using System.Diagnostics;
using Global;
using MenuSystemLibrary;
using System.Collections;
using Alienbrain;
using System.Text.RegularExpressions;

namespace file_tag_manager_2
{
	public partial class ListViewManager : SuperListView
	{
		internal XmlDocument m_xDocument;

		internal bool bDocumentDirty = false;

		internal string m_strDocumentFilename;
		//static internal string TAG_FILENAME = "file_tag_manager_testdata.data";
		static internal string TAG_FILENAME = "file_tag_manager.data";
		MenuSystem ms = new MenuSystem();							// Used for validating xml menu documents.

		XmlElement m_tempValueNode;
		Thread m_populateThread;
		Queue<ListViewCommand> m_PopulateCommandQueue;
		EventWaitHandle m_populateThread_Wait;
		string m_strAlienbrainProject;
		string m_strExternalViewerProcess;
		XmlDocument m_xTemplate = null;

		StatusStrip m_statusStrip;
		//ImageViewer m_ImageViewer;
		MenuSystem m_menuSystem;
		ToolStripMenuItem m_tsmiExternalViewer;

		Dictionary<string, XmlElement> m_typeLookup;
		Dictionary<string, XmlElement> m_nodeLookup;
		public bool m_bMenuIsReadOnly = true;

		bool m_bFollowingTypeReference = false;

		//bool m_bDataFileIsNew = false;

		int m_itemCount = 0;
		private ListViewColumnSorter lvwColumnSorter;

		System.Threading.Timer m_listViewSelectionChangedTimer;

		public ListViewManager()
			: base()
		{
			InitializeComponent();

			// Create an instance of a ListView column sorter and assign it 
			// to the ListView control.
			lvwColumnSorter = new ListViewColumnSorter();//LVCS
			//this.ListViewItemSorter = lvwColumnSorter;//LVCS
			m_listViewSelectionChangedTimer = new System.Threading.Timer(SelectionChangedTimerCallback);
			this.Sorting = SortOrder.None;
		}
		// name, (type, index)
		Dictionary<string, KeyValuePair<string, int>>m_extraColumnLookup = null;

		public bool Initialise(StatusStrip statusStrip, MenuSystem menuSystem, fileTagManager_MainForm mainForm, ToolStripMenuItem tsmiExternalViewer)
		{
			m_statusStrip = statusStrip;
			//m_ImageViewer = ImageViewer;
			m_menuSystem = menuSystem;

			m_tsmiExternalViewer = tsmiExternalViewer;

			m_fileTagManager_MainForm = mainForm;
			
			this.Columns.Add("Name", 100);
			this.Columns.Add("Local", 60);
			this.Columns.Add("Remote", 60);
			this.Columns.Add("Checkout status", 120);

			this.Columns.Add("Pixel Area", 120);

			try
			{
				m_xTemplate = LoadTemplate("template.file_tag_manager");

				if(m_xTemplate != null)
				{	// Get the details for any extra columns that are required.
					m_extraColumnLookup = new Dictionary<string,KeyValuePair<string,int>>();

					XmlElement xTemplateRoot = m_xTemplate["template"];

					XmlElement xListViewColumns = xTemplateRoot["list_view_columns"];

					m_firstCustomSubItem = this.Columns.Count;

					if(xListViewColumns != null)
					{
						foreach(XmlElement xListViewColumn in xListViewColumns)
						{
							if(xListViewColumn != null && xListViewColumn.Name == "list_view_column")
							{
								m_extraColumnLookup.Add(XmlHelper.GetKeyValue(xListViewColumn, "name"),
														new KeyValuePair<string, int>(XmlHelper.GetKeyValue(xListViewColumn, "type"), this.Columns.Count));

								string strDisplayName = XmlHelper.GetKeyValue(xListViewColumn, "display_name");
								if(strDisplayName == null)
									strDisplayName = XmlHelper.GetKeyValue(xListViewColumn, "name");
								this.Columns.Add(strDisplayName, 120);
							}
						}
					}

					m_strAlienbrainProject = XmlHelper.GetKeyValue(xTemplateRoot, "alienbrain_project");
					m_strExternalViewerProcess = XmlHelper.GetKeyValue(xTemplateRoot, "texture_viewer");
					
					m_typeLookup = GetTypeLookupFromTemplate(m_xTemplate);

					m_populateThread_Wait = new EventWaitHandle(false, EventResetMode.AutoReset);

					m_populateThread = new Thread(new ThreadStart(PopulateThread));
					m_populateThread.Name = "List view manager.";

					//					m_populateThread.Priority = ThreadPriority.BelowNormal;
					m_populateThread.Priority = ThreadPriority.Normal;
					m_PopulateCommandQueue = new Queue<ListViewCommand>();
					m_populateThread.Start();
					return true;
				}
			}
			catch
			{
				return false;
			}
			return false;
		}

		private XmlDocument LoadTemplate(string strPathFilename)
		{
			XmlDocument xTemplate = new XmlDocument();

			try
			{
				xTemplate.Load(strPathFilename);
				return xTemplate;
			}
			catch(Exception ex)
			{
				MessageBox.Show("Cannot load the template file. System report:\n" + ex.Message);
				return null;
			}
		}

		void PopulateThread()// Depracated.
		{
			while(true)
			{
				m_populateThread_Wait.WaitOne();

				while(m_PopulateCommandQueue.Count != 0)
				{
					ListViewCommand lvc;

					lock(m_PopulateCommandQueue)
					{
						if(m_PopulateCommandQueue.Count != 0)
							lvc = m_PopulateCommandQueue.Dequeue();
						else
							lvc = null;
					}

					if(lvc != null)
						lvc.DoCommand(this);
				}
			}
		}

		private bool SetMenuSystemError(string strError)
		{
			m_menuSystem.Initialise(null);
			m_menuSystem.Invoke((MethodInvoker)delegate() { m_menuSystem.SetError(strError); });
			return true;
		}

		bool bMenuSystemIsInErrorState = false;

		internal XmlDocument LoadTagDataFile(string strDocumentFilename)
		{
			m_strDocumentFilename = strDocumentFilename;		// Store this away so we can later save the document.

			bMenuSystemIsInErrorState = false;

			XmlDocument xConfig = new XmlDocument();

			m_menuSystem.m_bReadOnly = true;

			try
			{
				string strPath = Path.GetDirectoryName(strDocumentFilename);

				bool bExistsInAb = fileTagManager_MainForm.m_alienbrainInterface.GetLatestFile(AlienbrainRequest.Priority.Normal, strDocumentFilename);	// This creates the local path if it needs to, asuming the file is in AB.

				if(bExistsInAb == false)
				{
					if(File.Exists(strDocumentFilename))
					{
						SetMenuSystemError("Conflicted state.\n" +
							"You have a local Texture Data file that is not managed by Alienbrain.\n" +
							"Edit this folder to see the options you have to correct this.");
						bMenuSystemIsInErrorState = true;

					}
					else
					{
						SetMenuSystemError("This folder does not have any texture data in Alienbrain.\n" +
							"Edit this folder to create a texture data file.");
						bMenuSystemIsInErrorState = true;
					}
					xConfig = null;
				}
				else
					xConfig.Load(strDocumentFilename);											// This will throw an exception if the file does not exist.
			}
			catch(Exception ex)
			{
				SetMenuSystemError("Cannot load '" + strDocumentFilename + "'\nSystem report:\n" + ex.Message + "\nPlease report this to the tools team.");
				Debug.WriteLine(ex.Message);
			}

			return xConfig;
		}

		/// <summary>
		/// Creates a tag data file and imports it into Alienbrain.
		/// If you have a local data file and no Ab file it will ask whether to use the local one or create a new one. (inc. option to cancel)
		/// Fails for various reasons:
		/// 1) If there is already a data file in Ab.
		/// 
		/// </summary>
		/// <param name="strDocumentFilename"></param>
		/// <returns></returns>
		internal XmlDocument CreateTagDataFile(string strDocumentFilename)
		{
			m_strDocumentFilename = strDocumentFilename;		// Store this away so we can later save the document.
			m_tempValueNode = null;

			XmlDocument xConfig = new XmlDocument();

			m_menuSystem.m_bReadOnly = true;

			try
			{
				bool bExistsInAb = fileTagManager_MainForm.m_alienbrainInterface.GetLatestFile(AlienbrainRequest.Priority.Normal, strDocumentFilename);	// This creates the local path if it needs to, asuming the file is in AB.
				bool bExistsLocally = File.Exists(strDocumentFilename);

				if(bExistsInAb == true)
				{
					MessageBox.Show(this, "Atempting to create the tag data file when there is already one in Alienbrain.\nPlease inform the tools team.", "Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Error);
					return null;
				}

				if(bExistsLocally == true)
				{
					DialogResult dr = MessageBox.Show(this,
						"Creating texture tag data.\n" +
						"You are attempting to create new texture tag data in Alienbrain when you already have" +
						"local texture tag data. Would you like to use the local data instead?", "Texture Tag Manager", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning);

					if(dr == DialogResult.Cancel)
						return null;
					else if(dr == DialogResult.No)
					{
						// create the data
						string strPath = Path.GetDirectoryName(strDocumentFilename);
						if(Directory.Exists(strPath) == false)
						{
							DirectoryInfo di = Directory.CreateDirectory(strPath);
						}

						xConfig.Load(new StringReader("<root><files/></root>"));
						xConfig.Save(strDocumentFilename);
					}
					else
					{
						// use existing data.
						xConfig.Load(strDocumentFilename);											// This will throw an exception if the file does not exist.
					}
				}
				else
				{
					string strPath = Path.GetDirectoryName(strDocumentFilename);
					if(Directory.Exists(strPath) == false)
					{
						DirectoryInfo di = Directory.CreateDirectory(strPath);
					}
					xConfig.Load(new StringReader("<root><files/></root>"));
					xConfig.Save(strDocumentFilename);
				}

				bool temp = fileTagManager_MainForm.m_alienbrainInterface.Import(AlienbrainRequest.Priority.Normal, strDocumentFilename);
				if(temp == false)
				{
					MessageBox.Show(this, "Importing '" + strDocumentFilename + "' into Alienbrain failed.\nYou must use the Alienbrain client to get latest on this folder first.", "Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Error);
					return null;
				}
				return xConfig;
			}
			catch(Exception ex)
			{
				MessageBox.Show("Cannot load '" + strDocumentFilename + "'\nSystem report: " + ex.Message);
				Debug.WriteLine(ex.Message);
				return null;
			}
		}

		internal ListViewObject ListViewCreateObject(AlienbrainInfo abInfo, int itemCount)
		{
			ListViewObject lvo;
			lvo = new ListViewObject(abInfo);

			while(lvo.SubItems.Count < itemCount)
				lvo.SubItems.Add("N/A");
//			lvo.SubItems.Add("working");
//			lvo.SubItems.Add("working");
//			lvo.SubItems.Add("working");

			if(lvo.m_abInfo.bIsLocalfile == true)
				lvo.SubItems[1].Text = "YES";
			else
				lvo.SubItems[1].Text = "NO";

			if(lvo.m_abInfo.bIsRemoteFile == true)
				lvo.SubItems[2].Text = "YES";
			else
				lvo.SubItems[2].Text = "NO";

			if(lvo.m_abInfo.bIsCheckedOutByMe == true)
				lvo.SubItems[3].Text = "CHECKED OUT";
			else
			{
				if(lvo.m_abInfo.bIsLocked == true)
					lvo.SubItems[3].Text = "LOCKED";
				else
					lvo.SubItems[3].Text = "FREE";
			}
			return lvo;
		}

		//enum GroupAction
		//{
		//    SetState_PotentiallyUnwanted = 0,
		//    DeleteUnwanted
		//}

		private void SetStatusText(string strText)
		{
			m_statusStrip.Items["toolStripStatusLabel"].Text = strText;
		}

		#region Delegates for the list view
		delegate void dSetStatusText(string strText);
		#endregion Delegates for the list view


		private XmlElement CreateFileDataNode(XmlElement xTemplate, string strFilename)
		{
			if(xTemplate != null && m_xDocument != null)
			{
				XmlElement xParent = m_xDocument["root"]["files"];	// This is where the type must be added to the data document.

				string strTypeName = XmlHelper.GetKeyValue(xTemplate, "name");

				string strFilemask = XmlHelper.GetKeyValue(xTemplate, "filemask");

				XmlElement xFileData = xParent.OwnerDocument.CreateElement("file_data");

				XmlHelper.AddAttribute(xFileData, "type_name", strTypeName);

				XmlNode xValues = xParent.OwnerDocument.CreateElement("values");
				xFileData.AppendChild(xValues);

				xParent.AppendChild(xFileData);

				XmlHelper.AddAttribute(xFileData, "file_name", strFilename.ToLower());

				return xFileData;
			}
			return null;
		}

		private XmlElement GetNamedTypeFromTemplate(XmlDocument xTemplateDoc, string strType)
		{
			XmlElement xTypes = xTemplateDoc["template"]["types"];
			foreach(XmlNode xNode in xTypes)
			{
				if(xNode.NodeType == XmlNodeType.Element)
				{
					XmlElement xType = xNode as XmlElement;

					if(xType.Name == "type" && XmlHelper.GetKeyValue(xType, "name") == strType)
					{
						string strReference = XmlHelper.GetKeyValue(xType, "alias");
						if(strReference != null)
						{
							if(m_bFollowingTypeReference == true)
							{
								MessageBox.Show(this, "Error. Template defines a type reference to a type reference.\n", "File Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Error);
								return null;
							}
							m_bFollowingTypeReference = true;
							XmlElement xNamedType = GetNamedTypeFromTemplate(xTemplateDoc, strReference);
							m_bFollowingTypeReference = false;
							return xNamedType;
						}
						else
							return xType;
					}
				}
			}
			return null;
		}

		private Dictionary<string, XmlElement> GetTypeLookupFromTemplate(XmlDocument xTemplateDoc)
		{
			Dictionary<string, XmlElement> typeLookup = new Dictionary<string, XmlElement>();

			XmlElement xTypes = xTemplateDoc["template"]["types"];

			foreach(XmlNode xNode in xTypes)
			{
				if(xNode.NodeType == XmlNodeType.Element)
				{
					XmlElement xType = xNode as XmlElement;

					if(xType.Name == "type")
					{
						string strTypeName = XmlHelper.GetKeyValue(xType, "name");		// Get the type name before the type is aliased.

						string strReference = XmlHelper.GetKeyValue(xType, "alias");
						if(strReference != null)
						{
							m_bFollowingTypeReference = true;
							xType = GetNamedTypeFromTemplate(xTemplateDoc, strReference);
							m_bFollowingTypeReference = false;
						}
						if(xType != null)
							typeLookup.Add(strTypeName, xType);
					}
				}
			}
			return typeLookup;
		}

		public bool AddPopulateCommand(ListViewCommand populateThreadCommand)
		{
			lock(m_PopulateCommandQueue)
			{
				m_PopulateCommandQueue.Enqueue(populateThreadCommand);
			}
			m_populateThread_Wait.Set();

			return true;
		}

		internal void RefreshListViewMenu()
		{
			ListViewManager_SelectedIndexChanged(this, new EventArgs());
		}

		private void ListViewManager_SelectedIndexChanged(object sender, EventArgs e)
		{
			m_listViewSelectionChangedTimer.Change(555, Timeout.Infinite);
		}

		void SelectionChangedTimerCallback(object state)
		{
			this.Invoke((MethodInvoker)delegate() { SelectionChangedTimerCallback(); });
		}
/*		void SelectionChangedTimerCallback()
		{
			ListViewManager lvm = this;

			m_tempValueNode = null;

			if(lvm.SelectedItems.Count > 0)
			{
				ListViewObject lvo = lvm.SelectedItems[0] as ListViewObject;

				if(lvo.m_xFileData != null)
				{
					if(lvo.m_abInfo.Type == AlienbrainInfo.eNxNBasicNamespaceType.DISKFILE)
					{
						m_menuSystem.Initialise(lvo.m_xTemplate["menu_system"], null);
						SetMenuSystemError("This file is not managed by Alienbrain.\nYou must manually add it to the project\nthen refresh this view\nbefore setting it's properties here.");
					}
					else
					{
						if(fileTagManager_MainForm.m_alienbrainInterface.GetLatestFile(AlienbrainRequest.Priority.Normal, lvo.m_abInfo.strLocalPath) == false)
							m_ImageViewer.SetImage(null);
						else
							m_ImageViewer.SetImage(lvo.m_abInfo.strLocalPath);
						if(lvo.m_abInfo.bIsLocked == true)
						{
							SetMenuSystemError("This file is checked out by another user.\nYou cannot change it's properties while\nit is checked out.");
						}
						else
						{
							m_menuSystem.m_bReadOnly = m_bMenuIsReadOnly;

							XmlDocument xFinalDoc = new XmlDocument();
							XmlDocument xCurrentDoc = null;

							foreach(ListViewObject lvo2 in lvm.SelectedItems)
							{
								XmlDocument xDoc = m_menuSystem.BuildFullMenuDocument(lvo2.m_xTemplate["menu_system"], lvo2.m_xFileData["values"]);

								if(xCurrentDoc == null)
								{
									xCurrentDoc = xDoc;
								}
								else
								{
									xCurrentDoc = m_menuSystem.MergeMenuDocuments(xCurrentDoc["container"], xDoc["container"], true);
								}
							}

							xFinalDoc.LoadXml("<values/>");
							xFinalDoc["values"].AppendChild(xFinalDoc.ImportNode(xCurrentDoc["container"], true));

							m_menuSystem.Initialise(lvo.m_xTemplate["menu_system"], MenuDelegate);

							m_tempValueNode = xFinalDoc["values"];

							this.m_menuSystem.rootMenuContainer.Controls.Clear();
							m_menuSystem.SetMenu(m_tempValueNode, false);				// The file data will be updated by the template and replaced by an event fired from this.
						}
					}
				}
				else if(bMenuSystemIsInErrorState == false)
				{
					//SetMenuSystemError("This file is not stored locally. Do a 'Get Latest' to\nview the properties on this file.");
					SetMenuSystemError("This file has no texture data.\nPlease inform the tools team that there has been an error.");
				}
			}
			else if(bMenuSystemIsInErrorState == false)
			{
				SetMenuSystemError("No selection.");

				XmlDocument xDoc = new XmlDocument();
			}
		}
*/
		void SelectionChangedTimerCallback()
		{
			ListViewManager lvm = this;

			string strError = null;

			m_tempValueNode = null;

			if(lvm.SelectedItems.Count == 1)
			{
				ListViewObject lvo = lvm.SelectedItems[0] as ListViewObject;

				//AlienbrainInfo abInfo = fileTagManager_MainForm.m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, lvo.m_abInfo.strNamespacePath, true);

				////AlienbrainInfo abInfo = fileTagManager_MainForm.m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, lvo.m_abInfo.strLocalPath, false);

				//if(abInfo != null)
				//{
				//    lvo.m_abInfo = abInfo;
				//}

				//if(lvo.m_abInfo.bIsCheckedOutByMe == false)
				//{
				//    if(fileTagManager_MainForm.m_alienbrainInterface.GetLatestFile(AlienbrainRequest.Priority.Normal, lvo.m_abInfo.strLocalPath) == false)
				//    {
				//        //m_ImageViewer.SetImage(null);
				//    }
				//    else
				//    {
				//        //m_ImageViewer.SetImage(lvo.m_abInfo.strLocalPath);
				//    }
				//}
				ShowExternalImage(lvo);

			}
			else
			{
				//m_ImageViewer.SetImage(null);
			}

			if(lvm.SelectedItems.Count > 0)
			{
				bool bError = false;

				foreach(ListViewObject lvo in lvm.SelectedItems)
				{
					if(lvo.m_xFileData != null)
					{
						if(lvo.m_abInfo.Type == AlienbrainInfo.eNxNBasicNamespaceType.DISKFILE)
						{
							//m_menuSystem.Initialise(lvo.m_xTemplate["menu_system"], null);
							if(lvm.SelectedItems.Count == 1)
								strError = "This file is not managed by Alienbrain.\nYou must manually add it to the project\nthen refresh this view\nbefore setting it's properties here.";
							else
								strError = "One or more selected files are not managed by Alienbrain.\nYou must manually add them to the project\nthen refresh this view\nbefore setting their properties here.";
							bError = true;
							break;
						}
						else
						{
							//if(fileTagManager_MainForm.m_alienbrainInterface.GetLatestFile(AlienbrainRequest.Priority.Normal, lvo.m_abInfo.strLocalPath) == false)
							//    m_ImageViewer.SetImage(null);
							//else
							//    m_ImageViewer.SetImage(lvo.m_abInfo.strLocalPath);
							if(lvo.m_abInfo.bIsLocked == true)
							{
								if(lvm.SelectedItems.Count == 1)
									strError = "This file is checked out by another user.\nYou cannot change it's properties while\nit is checked out.";
								else
									strError = "One or more files are checked out by another user.\nYou cannot change their properties while\nthey are checked out.";
								bError = true;
								break;
							}

						}
					}
					else
					{
						bError = true;
						if(bMenuSystemIsInErrorState == false)
						{
							if(lvm.SelectedItems.Count == 1)
								SetMenuSystemError("This file has no texture data.\nPlease inform the tools team that there has been an error.");
							else
								SetMenuSystemError("One or more files have no texture data.\nPlease inform the tools team that there has been an error.");
						}
					}
				}
				if(bError == false)
				{
					m_menuSystem.m_bReadOnly = m_bMenuIsReadOnly;

					XmlDocument xFinalDoc = new XmlDocument();
					XmlDocument xCurrentDoc = null;

					foreach(ListViewObject lvo in lvm.SelectedItems)
					{
						// It is important to initialise the menu system with the current template so that macros are expanded and enums are relevant to the current item.
						// The 'on changed' delegate is also required, but that is persistent for the duration of this loop.


						m_menuSystem.Initialise(lvo.m_xTemplate["menu_system"], true, true);
						m_menuSystem.MenuChangedEvent += new EventHandler<MenuSystemChangedEventArgs>(m_menuSystem_MenuChangedEvent);
						//m_menuSystem.Initialise(lvo.m_xTemplate["menu_system"], MenuDelegate);

						//ATTENTION XmlDocument xDoc2 = m_menuSystem.BuildFullMenuDocument(lvo.m_xTemplate["menu_system"], lvo.m_xFileData["values"]);

						//ATTENTION ...
						XmlDocument xDoc = MenuSystem.MergeMenuDocuments(lvo.m_xTemplate["menu_system"]["container"], lvo.m_xFileData["values"]["container"], MenuSystemLibrary.MenuSystem.MergeMode.MasterIsTemplate);

						//ATTENTION Debug.Assert(string.Compare(xDoc.OuterXml, xDoc2.OuterXml) == 0);

						if(xCurrentDoc == null)
						{
							xCurrentDoc = xDoc;
						}
						else
						{
							//ATTENTION
							xCurrentDoc = MenuSystem.MergeMenuDocuments(xCurrentDoc["container"], xDoc["container"], MenuSystem.MergeMode.MasterIsTemplate);
						}
					}

					xFinalDoc.LoadXml("<values/>");
					xFinalDoc["values"].AppendChild(xFinalDoc.ImportNode(xCurrentDoc["container"], true));

					//m_menuSystem.Initialise(null, MenuDelegate);

					m_tempValueNode = xFinalDoc["values"];

					this.m_menuSystem.rootMenuContainer.Controls.Clear();
					m_menuSystem.SetMenu(m_tempValueNode["container"], false);				// The file data will be updated by the template and replaced by an event fired from this.
				}
				else
				{
					if(bMenuSystemIsInErrorState == false)
						SetMenuSystemError(strError);
				}
			}
			else if(bMenuSystemIsInErrorState == false)
			{
				SetMenuSystemError("No selection.");

				XmlDocument xDoc = new XmlDocument();
			}
		}

		void m_menuSystem_MenuChangedEvent(object sender, MenuSystemChangedEventArgs e)
		{
		//void MenuDelegate(XmlElement xMenu, Container_Base cChangedContainer, bool bFlagDocumentAsChanged)

			if(e.XChangedElement == null)
				return;

			//string strName = XmlHelper.GetKeyValue(cChangedContainer.m_xObject, "name");
			//string strValue = XmlHelper.GetKeyValue(cChangedContainer.m_xObject, "value");
			
			string strName = XmlHelper.GetKeyValue(e.XChangedElement, "name");
			string strValue = XmlHelper.GetKeyValue(e.XChangedElement, "value");

			if(strValue != MenuSystem.CONFLICTED_MARKER)
			{
				foreach(ListViewObject lvo in this.SelectedItems)
				{
					XmlElement xDest = XmlHelper.FindNodeByKeyValue(lvo.m_xFileData, "name", strName);

					if(xDest != null)
					{
						//Debug.Assert(xDest == cChangedContainer.m_xObject);

						XmlHelper.AddAttribute(xDest, "value", strValue);
					}

					KeyValuePair<string, int> kvp;
					
					if(m_extraColumnLookup.TryGetValue(strName, out kvp) == true)
					{
						strValue = GetValueByType(kvp.Key, xDest);
						lvo.SubItems[kvp.Value].Text = strValue;
					}

					bDocumentDirty = true;
				}
			}
		

		}

		private void ShowExternalImage(ListViewObject lvo)
		{
			if(m_tsmiExternalViewer.Checked == true)
			{
				if(lvo.m_xFileData != null)
				{
					XmlDocument xArgsDoc = new XmlDocument();
					XmlElement xRoot = xArgsDoc.AppendChild(xArgsDoc.ImportNode(lvo.m_xFileData, true)) as XmlElement;

					Debug.Assert(xRoot.Name == "file_data");

					XmlHelper.AddAttribute(xRoot, "file_name", lvo.m_abInfo.strLocalPath);
					XmlHelper.AddAttribute(xRoot, "alienbrain_project", m_strAlienbrainProject);

					string strTempFile = Path.GetTempFileName();
					xArgsDoc.Save(strTempFile);

					LaunchJob(m_strExternalViewerProcess, "\""+strTempFile+"\"", null);

				}
			}
		}

		public bool LaunchJob(string strCommand, string strClArguments, string strWorkingDirectory)
		{
			try
			{
				Process p = new Process();
				p.StartInfo.UseShellExecute = false;
				p.StartInfo = new ProcessStartInfo(strCommand, strClArguments);

				if(strWorkingDirectory != null)
				{
					p.StartInfo.WorkingDirectory = strWorkingDirectory;
				}
				else
				{
					p.StartInfo.WorkingDirectory = Path.GetDirectoryName(strCommand);
				}

				p.Start();
				return true;
			}
			catch(Exception ex)
			{
//				MessageBox.Show(this, "Cannot open the specified file. System reports:\n" + ex.Message, "Executer", MessageBoxButtons.OK);
				return false;
				//throw (ex);
			}
		}


		private void ListViewManager_ColumnClick(object sender, ColumnClickEventArgs e)
		{
			//LVCS
			// Determine if clicked column is already the column that is being sorted.
			if(e.Column == lvwColumnSorter.SortColumn)
			{
				// Reverse the current sort direction for this column.
				if(lvwColumnSorter.Order == SortOrder.Ascending)
				{
					lvwColumnSorter.Order = SortOrder.Descending;
				}
				else
				{
					lvwColumnSorter.Order = SortOrder.Ascending;
				}
			}
			else
			{
				// Set the column number that is to be sorted; default to ascending.
				lvwColumnSorter.SortColumn = e.Column;
				lvwColumnSorter.Order = SortOrder.Ascending;
			}

			// Perform the sort with these new sort options.
			this.Sort();
			// End LVCS*/
		}

		internal void KillThread()
		{
			this.m_populateThread.Abort();
		}

		internal bool PrepareToPopulateListView(string strPath)
		{
			lock(m_PopulateCommandQueue)
			{
				m_PopulateCommandQueue.Clear();
			}
			DoClear();

			return AddPopulateCommand(ListViewCommand.CreateCommandPrepare(strPath));
		}

		internal void DoClear()
		{
			if(this.InvokeRequired == true)
			{
				this.Invoke((MethodInvoker)delegate(){ClearListView();});
			}
			else
			{
				ClearListView();
			}
		}

		private bool ClearListView()
		{
			//m_ImageViewer.SetImage(null);
			this.m_menuSystem.rootMenuContainer.Controls.Clear();
			this.Items.Clear();

			return true;
		}

		int m_firstCustomSubItem = -1;

		internal bool ListViewObjectAddFileData(ListViewObject lvo)
		{
			// Find or create Xml element.
			string strTypeName = Path.GetExtension(lvo.m_abInfo.strName).ToLower();
			XmlElement xTemplate = null;

			// Get the template for this type of object ...
			m_typeLookup.TryGetValue(strTypeName, out xTemplate);
			if(xTemplate == null)
				m_typeLookup.TryGetValue(".default.", out xTemplate);

			if(xTemplate != null)
			{
				XmlElement xFileData = null;
				m_nodeLookup.TryGetValue(lvo.m_abInfo.strName.ToLower(), out xFileData);
				if(xFileData == null)
				{
					xFileData = CreateFileDataNode(xTemplate, lvo.m_abInfo.strName);
				}

				// Template can cause an update to the type name ...
				string strActualTypeName = XmlHelper.GetKeyValue(xTemplate, "name");
				XmlHelper.AddAttribute(xFileData, "type_name", strActualTypeName);

				lvo.m_xTemplate = xTemplate;
				lvo.m_xFileData = xFileData;

				// Validate the menu ...
				//ms.Initialise(lvo.m_xTemplate["menu_system"], MenuDelegate, false, false);
				ms.Initialise(lvo.m_xTemplate["menu_system"], false, false);


				if(lvo.m_xFileData != null)
				{
					XmlElement xValues = lvo.m_xFileData["values"];

					XmlDocument xNewDoc;
					xNewDoc = ms.XmlValidateMenu(xValues);

					//XmlNode xValues = lvo.m_xFileData["values"];

					if(xValues == null)
					{
						xValues = lvo.m_xFileData.OwnerDocument.CreateElement("values");
						lvo.m_xFileData.AppendChild(xValues);
					}
					xValues.RemoveAll();

					XmlElement xImportedChild = lvo.m_xFileData.OwnerDocument.ImportNode(xNewDoc["container"], true) as XmlElement;
					xValues.AppendChild(xImportedChild);


					if(lvo.m_xFileData != null)
					{
						if(m_extraColumnLookup != null)
						{
							int index = m_firstCustomSubItem;

							foreach(KeyValuePair<string, KeyValuePair<string, int>> kvp in m_extraColumnLookup)
							{
								//lvo.SubItems.Add(XmlHelper.FindElementsByKeyValue(lvo.m_xFileData, "name", kvp.Value, true, true, true);
								XmlElement xContainer = XmlHelper.FindNodeByKeyValue(lvo.m_xFileData, "name", kvp.Key);

								string strValue = "???";

								strValue = GetValueByType(kvp.Value.Key, xContainer);

								//ListViewItem.ListViewSubItem lvsu = lvo.SubItems.Add(strValue);

								lvo.SubItems[index++].Text = strValue;
								//ListViewItem.ListViewSubItem lvsu = lvo.SubItems.Add(strValue);
							}
						}
					}



					return true;
				}
			}
			return false;
		}


		void GetSizeThread()
		{
			int count;
			this.Invoke((MethodInvoker)delegate{count = this.Items.Count;});
			
			foreach(KeyValuePair<string, ListViewItem.ListViewSubItem> sizeLookup in m_sizeLookup)
			{
				string strTextureSize = GetTextureSize(sizeLookup.Key);
				
				//ListViewItem.ListViewSubItem lvsiSize = lvo.SubItems[4];

				this.Invoke((MethodInvoker)delegate { sizeLookup.Value.Text = strTextureSize; });
				//if(lvsiSize != null)
				//{
				//    lvsiSize.Text = strTextureSize;
				//}
			}
			GetSizeThreadEvent.Set();

		}




		private string GetTextureSize(string strFilename)
		{
			if(strFilename.ToLower().EndsWith(".tga"))
			{
				Size size = GetTgaWidthHeight(strFilename);
				if(size.Width != -1)
				{
					//return size.Width.ToString() + " " + size.Height.ToString();
					//int t = 0;
					//return t.ToString("D15");
					return (size.Width * size.Height).ToString().PadLeft(9);

					//return size.ToString();
				}
				else
				{
					return "Unknown";
				}
			}
			else if(strFilename.ToLower().EndsWith(".bmp"))
			{
				Size size = GetBmpWidthHeight(strFilename);
				if(size.Width != -1)
				{
					return (size.Width * size.Height).ToString().PadLeft(9);
					//return size.Width.ToString() + " " + size.Height.ToString();
					//return size.ToString();
				}
				else
				{
					return "Unknown";
				}
			}

			return "Unknown.";
		}

		private Size GetBmpWidthHeight(string strFilename)
		{
			FileStream fst = null;
			BinaryReader brd = null;

			try
			{
				fst = new FileStream(strFilename, FileMode.Open, FileAccess.Read);
				brd = new BinaryReader(fst);

				UInt16 BM = brd.ReadUInt16();
				UInt32 filesize = brd.ReadUInt32();
				UInt32 Always0 = brd.ReadUInt32();
				UInt32 pixelDataOffset = brd.ReadUInt32();
				UInt32 always40 = brd.ReadUInt32();
				UInt32 width = brd.ReadUInt32();
				UInt32 height = brd.ReadUInt32();

				// blah.

				Size size = new Size((int)width, (int)height);

				return size;
			}
			catch(Exception ex)
			{
				return new Size(-1, -1);
			}
			finally
			{
				if(fst != null)
					fst.Close();

				if(brd != null)
					brd.Close();
			}

		}

		private Size GetTgaWidthHeight(string strFilename)
		{
			FileStream fst = null;
			BinaryReader brd = null;
			try
			{
				fst = new FileStream(strFilename, FileMode.Open, FileAccess.Read);
				brd = new BinaryReader(fst);

				byte idLength = brd.ReadByte();
				byte colorMapType = brd.ReadByte();
				byte imageType = brd.ReadByte();
				short colorMapOrigin = brd.ReadInt16();
				short colorMapLength = brd.ReadInt16();
				byte colorMapEntrySize = brd.ReadByte();
				short originX = brd.ReadInt16();
				short originY = brd.ReadInt16();
				short width = brd.ReadInt16();
				short height = brd.ReadInt16();
				byte bits = brd.ReadByte();
				byte descriptor = brd.ReadByte();

				Size size = new Size(width, height);

				return size;
			}
			catch(Exception ex)
			{
				return new Size(-1, -1);
			}
			finally
			{
				if(fst != null)
					fst.Close();

				if(brd != null)
					brd.Close();
			}
		}



		private string GetValueByType(string strType, XmlElement xContainer)
		{
			string strRetval = "???";

			if(xContainer != null)
			{
				if(strType == "bool")
				{
					strRetval = XmlHelper.GetKeyValue(xContainer, "value");
				}
				else if(strType == "int")
				{
					strRetval = XmlHelper.GetKeyValue(xContainer, "value");
				}
				else if(strType == "enum")
				{
					strRetval = XmlHelper.GetKeyValue(xContainer, "value");
				}
			}
			return strRetval;
		}

		//internal void DoPrepareToPopulateListView()
		//{
		//    m_nodeLookup = new Dictionary<string, XmlElement>();

		//    m_itemCount = 0;

		//    if(m_xDocument != null)
		//    {
		//        XmlNodeList xList = m_xDocument["root"]["files"].GetElementsByTagName("file_data");

		//        foreach(XmlElement xElement in xList)
		//        {
		//            string strFilename = XmlHelper.GetKeyValue(xElement, "file_name");

		//            try
		//            {
		//                m_nodeLookup.Add(strFilename, xElement);
		//            }
		//            catch(Exception ex)
		//            {
		//                Debug.WriteLine("Error in DoPrepareToPopulateListView: '" + ex.Message + "'");
		//            }
		//        }
		//    }
		//}

		//internal void DoEndPrepareListView()
		//{
		//    string strstatusText = m_itemCount + " Object" + (m_itemCount == 1 ? "" : "s");
		//    m_statusStrip.Invoke(new dSetStatusText(SetStatusText), new object[] { strstatusText });

		//    if(m_xDocument != null)
		//    {
		//        XmlNodeList xList = m_xDocument["root"]["files"].GetElementsByTagName("file_data");

		//        if(xList != null)
		//        {
		//            List<XmlElement> xDeleteList = new List<XmlElement>(xList.Count);
		//            foreach(XmlElement xChild in xList)
		//            {
		//                XmlAttribute xAttr = xChild.Attributes.GetNamedItem("keep") as XmlAttribute;
		//                //if(XmlHelper.GetKeyValue(xChild, "keep") != "true")
		//                if(xAttr == null)
		//                    xDeleteList.Add(xChild);
		//                else
		//                    //	xChild.RemoveAttribute("keep");
		//                    xChild.Attributes.Remove(xAttr);

		//            }
		//            foreach(XmlElement xDeleteMe in xDeleteList)
		//            {
		//                m_xDocument["root"]["files"].RemoveChild(xDeleteMe);
		//            }
		//        }
		//    }
		//    this.Invoke((MethodInvoker)delegate() { this.RefreshListViewMenu(); });

		//    this.m_fileTagManager_MainForm.EnableToolstrip();
		//}

		//internal bool GetLatest()
		//{
		//    bool bRetval = true;

		//    foreach(ListViewObject lvo in this.Items)
		//    {
		//        if(lvo.m_abInfo.bIsRemoteFile == true)
		//        {
		//            if(lvo.m_abInfo.bIsCheckedOutByMe == false)							// Do not 'GetLatest' on files checked out by me.
		//            {
		//                string strFilename = lvo.m_abInfo.strLocalPath;

		//                if(fileTagManager_MainForm.m_alienbrainInterface.GetLatestFile(AlienbrainRequest.Priority.Normal, strFilename) == false)
		//                    bRetval = false;
		//            }
		//        }
		//    }
		//    return bRetval;
		//}
/*
		internal bool Revert()
		{
			bool bRetval = true;

			foreach(ListViewObject lvo in this.Items)
			{
				if(lvo.m_abInfo.bIsRemoteFile == true)
				{
					string strFilename = lvo.m_abInfo.strLocalPath;

					if(fileTagManager_MainForm.m_alienbrainInterface.GetLatestFile(AlienbrainRequest.Priority.Normal, strFilename) == false)
						bRetval = false;
				}
			}
			return bRetval;
		}
*/
		fileTagManager_MainForm m_fileTagManager_MainForm;
		internal void SetMainForm(fileTagManager_MainForm fileTagManager_MainForm)
		{
			m_fileTagManager_MainForm = fileTagManager_MainForm;
		}

		private void ListViewManager_ItemSelectionChanged(object sender, ListViewItemSelectionChangedEventArgs e)
		{
			Debug.WriteLine("SelectionChanged." + e.Item.ToString());

		}

		internal XmlElement GetMenuItems()
		{
			try
			{
				return m_xTemplate["template"]["menu_items"];
			}
			catch
			{
				return null;
			}
		}
		internal XmlElement GetRegularExpressions()
		{
			try
			{
				return m_xTemplate["template"]["regular_expressions"];
			}
			catch
			{
				return null;
			}
		}

		string m_strSelectedItem;

		internal void CustomBeginUpdate()
		{
			if(this.SelectedItems.Count > 0)
				m_strSelectedItem = this.SelectedItems[0].Text;
			else
				m_strSelectedItem = null;

			foreach(ListViewObject lvo in this.SelectedItems)
			{
				m_strSelectedItem = lvo.Text;
			}

			this.ListViewItemSorter = null;
			BeginUpdate();
		}

		internal void CustomEndUpdate()
		{
			if(m_strSelectedItem != null)
			{
				foreach(ListViewObject lvo in this.Items)
				{
					if(m_strSelectedItem == lvo.Text)
					{
						lvo.Selected = true;
						break;
					}
				}
			}

			this.ListViewItemSorter = lvwColumnSorter;
			this.Sort();
			EndUpdate();
		}

		TreeObject m_currentTreeParent = null;

		AutoResetEvent GetSizeThreadEvent = new AutoResetEvent(true);
		Thread SizeThread = null;


		Dictionary<string, ListViewItem.ListViewSubItem> m_sizeLookup;

		public void RefreshListView(TreeObject treeParent)
		{
			if(SizeThread != null && SizeThread.IsAlive)
			{
				SizeThread.Abort();
				GetSizeThreadEvent.WaitOne();
			}

			m_sizeLookup = new Dictionary<string, ListViewItem.ListViewSubItem>();

			m_currentTreeParent = treeParent;
			this.Invoke((MethodInvoker)delegate(){_RefreshListView(treeParent);});

			SizeThread = new Thread(new ThreadStart(GetSizeThread));
			SizeThread.Name = "GetSize thread.";
			SizeThread.Priority = ThreadPriority.Normal;
			SizeThread.Start();
		}

		private void _RefreshListView(TreeObject treeParent)
		{
			CustomBeginUpdate();

			DoClear();

			string strPath = treeParent.m_abInfo.strLocalPath;
			string strDocumentFilename = strPath + ListViewManager.TAG_FILENAME;

			m_xDocument = LoadTagDataFile(strDocumentFilename);

			//DoPrepareToPopulateListView();

			m_nodeLookup = new Dictionary<string, XmlElement>();

			m_itemCount = 0;

			if(m_xDocument != null)
			{
				XmlNodeList xList = m_xDocument["root"]["files"].GetElementsByTagName("file_data");

				foreach(XmlElement xElement in xList)
				{
					string strFilename = XmlHelper.GetKeyValue(xElement, "file_name");

					try
					{
						m_nodeLookup.Add(strFilename, xElement);
					}
					catch(Exception ex)
					{
						Debug.WriteLine("Error in DoPrepareToPopulateListView: '" + ex.Message + "'");
					}
				}
			}

			//int count = 0;

			foreach(AlienbrainInfo abInfo in treeParent.m_listAbFiles)
			{
				if(abInfo.strName.ToLower() != ListViewManager.TAG_FILENAME.ToLower())
				{

					ListViewObject lvo = ListViewCreateObject(abInfo, this.Columns.Count);
					if(m_regex != null)
					{
						if(m_regex.IsMatch(abInfo.strName) == true)
						{
							Items.Add(lvo);
							m_sizeLookup.Add(lvo.m_abInfo.strLocalPath, lvo.SubItems[4]);
						}
					}
					ListViewObjectAddFileData(lvo);

					if(lvo.m_xFileData != null)
					{
						XmlAttribute attr = lvo.m_xFileData.OwnerDocument.CreateAttribute("keep");
						lvo.m_xFileData.Attributes.SetNamedItem(attr);
					}

					m_itemCount++;
					if((m_itemCount % 100) == 0)
					{
						SetStatusText("Sending to list view :" + m_itemCount.ToString());
					}

				}
			}
			SetStatusText("Sending to list view :" + m_itemCount.ToString());

			string strstatusText = m_itemCount + " Object" + (m_itemCount == 1 ? "" : "s");
			m_statusStrip.Invoke(new dSetStatusText(SetStatusText), new object[] { strstatusText });

			if(m_xDocument != null)
			{
				XmlNodeList xList = m_xDocument["root"]["files"].GetElementsByTagName("file_data");

				if(xList != null)
				{
					List<XmlElement> xDeleteList = new List<XmlElement>(xList.Count);
					foreach(XmlElement xChild in xList)
					{
						XmlAttribute xAttr = xChild.Attributes.GetNamedItem("keep") as XmlAttribute;
						if(xAttr == null)
							xDeleteList.Add(xChild);
						else
							xChild.Attributes.Remove(xAttr);

					}
					foreach(XmlElement xDeleteMe in xDeleteList)
					{
						m_xDocument["root"]["files"].RemoveChild(xDeleteMe);
					}
				}
			}
			this.Invoke((MethodInvoker)delegate() { this.RefreshListViewMenu(); });

			this.m_fileTagManager_MainForm.EnableToolstrip();

			CustomEndUpdate();
		}

		Regex m_regex = null;

		internal void SetRegexp(Regex regex)
		{
			try
			{
				m_regex = regex;
				if(m_currentTreeParent != null)
					RefreshListView(m_currentTreeParent);
			}
			catch
			{
				m_regex = null;
				RefreshListView(m_currentTreeParent);
			}
		}
	}
}
