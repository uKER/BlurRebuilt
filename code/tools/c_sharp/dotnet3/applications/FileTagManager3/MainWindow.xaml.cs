using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Alienbrain;
using System.Collections.ObjectModel;
using System.Collections;
using System.Threading;
using System.Diagnostics;
using System.Windows.Forms.Integration;
using System.Xml;
using DataLib.DataModel;
using MenuSystemLibrary;
using Global;
using FileTagManager3.LoadOnDemand;
using System.ComponentModel;

namespace FileTagManager3
{
	/// <summary>
	/// Interaction logic MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindow()
		{
			InitializeComponent();
		}

		private static XmlDocument m_xFullTemplate;

		/// <summary>
		/// This is a lookup that matches type names to their Xml type definitions in the template.
		/// It respects 'type aliases', so for example if a '.jpg' maps to a '.picture' m_typeLookup["jpg"] you get the "picture" type.
		/// </summary>
		private static Dictionary<string, XmlElement> m_typeLookup;



		/// <summary>
		/// This is a quick reference to the menu system used by the application.
		/// </summary>
		MenuSystemLibrary.MenuSystem m_menuSystem;



		private void abTreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
		{
			// Bind the listview datasource to the treeview file list.
			this.abListView.DataContext = this.abTreeView;
			this.abListView.ItemsSource = this.abTreeView.AbObservableListFiles;
			Debug.WriteLine("Bound to AbObservableListFiles in LoadOnDemandUserControl.");

			// Get the template.
			// TODO: Make this configurable.
			//m_xTemplate = abTreeView.GetTemplate("FileTagManager.user1.template");
		}

		private void Window_Loaded(object sender, RoutedEventArgs e)
		{
			m_menuSystem = windowsFormsHost1.Child as MenuSystemLibrary.MenuSystem;

			m_menuSystem.m_bReadOnly = true;
			m_menuSystem.SetError("This is a MenuSystem.");

			// TODO: Temporary.
			// Load a default template.
			m_xFullTemplate = new XmlDocument();

			try
			{
				m_xFullTemplate.Load("FileTagManager3.ftm3_template");

				m_typeLookup = GetTypeLookupFromTemplate(m_xFullTemplate);
			}
			catch(Exception ex)
			{
				m_menuSystem.SetError("Error with the template 'FileTagManager3.ftm3_template'\nThe system reports:"+ex.Message);
				this.abTreeView.IsEnabled = false;
			}

		}

		private void abListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			RefreshMenuView();
		}

		private void RefreshMenuView()
		{
			// Get a list of all the AbData objects that are selected.
			List<DataLib.DataModel.AbData> selectedItems = new List<DataLib.DataModel.AbData>(this.abListView.SelectedItems.Count);

			foreach(DataLib.DataModel.AbData abData in this.abListView.SelectedItems)
			{
				selectedItems.Add(abData);
			}

			if(selectedItems.Count == 0)
			{
				m_menuSystem.SetError("This is a MenuSystem. No selection.");
			}
			else
			{
				// Get the Xml menu data from the AbData object.
				// Merge them all together.
				// Present the final Xml to the menu system.
				bool bSuccess = SetFullMenuDocument(selectedItems, m_menuSystem);
			}
		}


		/// <summary>
		/// Given a list of selected items and a menu system, Merge the Xml menu data of
		/// each selected item into the menu system.
		/// </summary>
		/// <param name="selectedItems">A list of items to merge.</param>
		/// <param name="menuSystem">The menu system to display the merged items.</param>
		/// <returns></returns>
		private bool SetFullMenuDocument(List<AbData> selectedItems, MenuSystem menuSystem)
		{
			string strErrors = "";

			XmlDocument xFinalDoc = new XmlDocument();
			XmlDocument xCurrentDoc = null;

			foreach(AbData abData in selectedItems)
			{
				// Get the template for this type of object ...
				string strTypeName = System.IO.Path.GetExtension(abData.Name).ToLower();

				XmlElement xTemplate = null;

				m_typeLookup.TryGetValue(strTypeName, out xTemplate);
				if(xTemplate == null)
					m_typeLookup.TryGetValue(".default.", out xTemplate);

				if(abData.IsRemoteFile == true)
				{
					if(xTemplate != null)
					{
						if(abData.GetFileData() != null)
						{
							if(abData.Type == AlienbrainInfo.eNxNBasicNamespaceType.DISKFILE)
							{
								strErrors += "The file '" + abData.Name + "' is not managed by Alienbrain.\nYou must manually add it to the project\nthen refresh this view\nbefore setting it's properties here.";
							}
							else if(abData.IsLocked == true)
							{
								strErrors += "The file '" + abData.Name + "' is checked out by another user.\nYou cannot change it's properties while\nit is checked out.";
							}
							else if(strErrors.Length == 0)
							{
								menuSystem.Initialise(xTemplate["menu_system"], MenuDelegate);

								XmlDocument xDoc = menuSystem.BuildFullMenuDocument(xTemplate["menu_system"] as XmlElement, abData.xFileData["root"]["file_data"]["values"]);

								/////////////////////
								XmlHelper.AddAttribute(abData.xFileData["root"]["file_data"], "file_name", abData.Name.ToLower());
								XmlHelper.AddAttribute(abData.xFileData["root"]["file_data"], "type_name", strTypeName.ToLower());

								XmlElement xValues = abData.xFileData["root"]["file_data"]["values"];

								if(xValues == null)
								{
									xValues = abData.xFileData.CreateElement("values");
									abData.xFileData["root"]["file_data"].AppendChild(xValues);
								}
								xValues.RemoveAll();

								XmlElement xImportedChild = abData.xFileData.ImportNode(xDoc["container"], true) as XmlElement;
								xValues.AppendChild(xImportedChild);

								/////////////////////
								if(xCurrentDoc == null)
								{
									xCurrentDoc = xDoc;
								}
								else
								{
									xCurrentDoc = menuSystem.MergeMenuDocuments(xCurrentDoc["container"], xDoc["container"], true);
								}
							}
						}
						else
						{
							strErrors += "The file '" + abData.Name + "' has no texture data.\nHas it been removed from Alienbrain?\n";
						}
					}
					else
					{
						strErrors += "The file '" + abData.Name + "' does not have a template and there is no default template.";
					}
				}
				else
				{
					strErrors += "The file '" + abData.Name + "' is not managed by alienbrain.";
				}

			}

			if(strErrors.Length == 0)
			{
				xFinalDoc.LoadXml("<values/>");
				xFinalDoc["values"].AppendChild(xFinalDoc.ImportNode(xCurrentDoc["container"], true));

				//menuSystem.Initialise(xTemplate["menu_system"], MenuDelegate);

				XmlElement tempValueNode = xFinalDoc["values"];

				menuSystem.rootMenuContainer.Controls.Clear();
				menuSystem.SetMenu(tempValueNode, false);				// The file data will be updated by the template and replaced by an event fired from this.
				return true;
			}
			else
			{
				menuSystem.SetError(strErrors);
				return false;
			}

#if false
/*	
			if(selectedItems.Count > 0)
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
*/
#endif

		}


		void MenuDelegate(XmlElement xMenu, Container_Base cChangedContainer, bool bFlagDocumentAsChanged)
		{
			if(cChangedContainer == null)
				return;

			string strName = XmlHelper.GetKeyValue(cChangedContainer.m_xObject, "name");
			string strValue = XmlHelper.GetKeyValue(cChangedContainer.m_xObject, "value");

			if(strValue != MenuSystem.CONFLICTED_MARKER)
			{
				foreach(AbData abData in this.abListView.SelectedItems)
				{
					XmlElement xDest = XmlHelper.FindNodeByKeyValue(abData.xFileData.SelectSingleNode("/root/file_data/values") as XmlElement, "name", strName);

					if(xDest != null)
					{
						XmlHelper.AddAttribute(xDest, "value", strValue);
					}
#if ColumnsProcessingIsDefined
					KeyValuePair<string, int> kvp;

					if(m_extraColumnLookup.TryGetValue(strName, out kvp) == true)
					{
						strValue = GetValueByType(kvp.Key, xDest);
						lvo.SubItems[kvp.Value].Text = strValue;
					}
#endif
					// This should always be true since it makes no sense to have an 'always_enabled' container in the menu definition.
					if(bFlagDocumentAsChanged == true)
					{
						abData.bXmlDocumentDirty = true;
					}
					Debug.Assert(bFlagDocumentAsChanged == true);
				}
			}
		}

		bool m_bFollowingTypeReference = false;

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
								throw (new ApplicationException("Error. Template defines a type reference to a type reference.\n"));
								//return null;
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

		#region Button callbacks

		private void buttonEdit_Click(object sender, RoutedEventArgs e)
		{
			AbDataViewModel dataViewModel = this.abTreeView.SelectedItem as AbDataViewModel;

			if(dataViewModel != null)
			{
				AbData abFolderData = dataViewModel.m_abData;
				if(abFolderData.LockAbFileFolder() == true)
				{

					m_menuSystem.m_bReadOnly = false;
					m_menuSystem.ReDrawMenu();

					//foreach(AbData abData in this.abListView.Items)
					//{
					//    abData.xFileData = null;
					//}
					this.buttonEdit.IsEnabled = false;
					this.buttonSubmit.IsEnabled = true;
					this.buttonRevert.IsEnabled = true;

					this.abTreeView.IsEnabled = false;

					//m_bMenuIsReadOnly = false;
					m_menuSystem.m_bReadOnly = false;
				}
				else
				{
					m_menuSystem.SetError("Cannot acquire lock for\nThis folder.\nIt is probably locked by\nsomeone else.\n\n\nIf it is not, you must manually remove\nlock.tagdata3lock\nfrom this folder in Alienbrain.");
				}
			}
		}

		private void buttonSubmit_Click(object sender, RoutedEventArgs e)
		{
			//this.abListView.SelectedItem = null;

			AbDataViewModel dataViewModel = this.abTreeView.SelectedItem as AbDataViewModel;
			AbData abFolderData = dataViewModel.m_abData;
			//if(abFolderData.UnLockAbFileFolder() == true)		// Not here, stupid. Unlock after comitting changes.
			{

				m_menuSystem.m_bReadOnly = true;
				m_menuSystem.ReDrawMenu();

				this.buttonEdit.IsEnabled = true;
				this.buttonSubmit.IsEnabled = false;
				this.buttonRevert.IsEnabled = false;

				this.abTreeView.IsEnabled = true;

				foreach(AbData abData in this.abListView.Items)
				{
					abData.SubmitDataFile(false);
				}
			}
			if(abFolderData.UnLockAbFileFolder() == false)
			{
				Debug.WriteLine("OH NO!");
			}

		}

		private void buttonRevert_Click(object sender, RoutedEventArgs e)
		{
			//this.abListView.SelectedItem = null;
			AbDataViewModel dataViewModel = this.abTreeView.SelectedItem as AbDataViewModel;
			AbData abFolderData = dataViewModel.m_abData;
			if(abFolderData.UnLockAbFileFolder() == true)
			{
				foreach(AbData abData in this.abListView.Items)
				{
					abData.xFileData = null;		/// Invalidate the file data so it has to be reloaded.
				}
				this.buttonEdit.IsEnabled = true;
				this.buttonSubmit.IsEnabled = false;
				this.buttonRevert.IsEnabled = false;

				this.abTreeView.IsEnabled = true;

				m_menuSystem.m_bReadOnly = true;
				RefreshMenuView();					/// The menu needs a full refresh because it's underlying data may have changed.
				//m_menuSystem.ReDrawMenu();
			}
			else
			{
				Debug.WriteLine("OH NO!");
			}
		}

		#endregion Button callbacks


		#region Sorting the list view.

		GridViewColumnHeader _lastHeaderClicked = null;

		ListSortDirection _lastDirection = ListSortDirection.Ascending;

		private void abListView_Click(object sender, RoutedEventArgs e)
		{
			GridViewColumnHeader headerClicked = e.OriginalSource as GridViewColumnHeader;
			ListSortDirection direction;

			if(headerClicked != null)
			{
				if(headerClicked != _lastHeaderClicked)
				{
					direction = ListSortDirection.Ascending;
				}
				else
				{
					if(_lastDirection == ListSortDirection.Ascending)
					{
						direction = ListSortDirection.Descending;
					}
					else
					{
						direction = ListSortDirection.Ascending;
					}
				}

				string header = headerClicked.Column.Header as string;

				// A dirty hack ...

				string strSortName = "";

				if(header == "Filename")
					strSortName = "Name";
				else if(header == "Local")
					strSortName = "IsLocalFile";
				else if(header == "Remote")
					strSortName = "IsRemoteFile";
				else if(header == "Locked")
					strSortName = "IsLocked";

				Sort(strSortName, direction);

				_lastHeaderClicked = headerClicked;
				_lastDirection = direction;
			}
		}

		private void Sort(string sortBy, ListSortDirection direction)
		{

			abListView.Items.SortDescriptions.Clear();

			SortDescription sd = new SortDescription(sortBy, direction);

			abListView.Items.SortDescriptions.Add(sd);
			
			abListView.Items.Refresh();

		}

		#endregion Sorting the list view.

	}





	#region If we need a datsource for the list view.

	static class AbDataSrc
	{
		static /*private*/ ObservableCollection<AlienbrainInfo> _results = new ObservableCollection<AlienbrainInfo>();
		static public ObservableCollection<AlienbrainInfo> Entries { get { return _results; } }

		static internal void LoadEntries(IList entries)
		{
			_results.Clear();

			foreach(AlienbrainInfo entry in entries)
			{
				_results.Add(entry);
			}
		}
	}
	#endregion
}











