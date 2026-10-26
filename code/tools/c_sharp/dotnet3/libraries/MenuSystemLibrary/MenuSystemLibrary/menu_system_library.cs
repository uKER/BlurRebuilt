using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Diagnostics;
using Global;
using Windows.Message;
using System.IO;

namespace MenuSystemLibrary
{
    public partial class MenuSystem : UserControl
    {
		public bool m_bReadOnly = false;

		public static string CONFLICTED_MARKER = "*CONFLICTED*";
		public static string UNINITIALISED_MARKER = "**UNINITIALISED**";

		Dictionary<string, XmlElement>m_containerNames = null;

		internal bool m_bChanged;

		public MenuSystem()
        {
            InitializeComponent();
			SetStyle(ControlStyles.Selectable, false);		// Disallow receiving focus.
        }

		public Dictionary<string, List<string>> m_enumTable = new Dictionary<string, List<string>>();

		/// <summary>
		/// This document defines the layout of the menu.
		/// The application may modify it during execution.
		/// </summary>
		internal XmlElement m_menuDefinitionRoot;

		/// <summary>
		/// This document defines the current values of the menu.
		/// It is passed to the MenuValueChanged delegate each time it is changed.
		/// </summary>
		XmlDocument m_xCurrentMenuDocument;

		/// <summary>
		/// This function is called each time a menu value is changed.
		/// It is called with (null) after the menu is first generated.
		/// </summary>
		/// <param name="control">Reference to the control that has changed.</param>
		/// <returns></returns>
		internal bool MenuValueChanged(Container_Base control)
		{
//			Debug.WriteLine("MenuValueChanged.");
			if(GetMenu != null)
			{
				XmlNode x = m_xCurrentMenuDocument as XmlNode;
				XmlElement root = m_xCurrentMenuDocument.DocumentElement;
				GetMenu(root, control, (control != null) ? control.m_bSaveValue : false);
			}
			return true;
		}


		/// <summary>
		/// This is called when a value in the menu changes.
		/// </summary>
		/// <param name="xMenu"></param>
		public delegate void GetMenuDelegate(XmlElement xMenuRoot, Container_Base cChangedElement, bool bFlagDocumentAsChanged);
		GetMenuDelegate GetMenu;

		/// <summary>
		/// Initialises the menu system with a template and an event to call when the menu changes.
		/// </summary>
		/// <param name="xRoot">The template for the menu. (The 'menu_system' definition document element)</param>
		/// <param name="menuDelegate">The delegate to call when the menu is changed by the user.</param>
		/// <returns>true</returns>
		public bool Initialise(XmlElement xRoot, GetMenuDelegate menuDelegate)
		{
			GetMenu = menuDelegate;

			m_menuDefinitionRoot = xRoot;
			PopulateEnumList(xRoot, m_enumTable);
			MacroExpand(m_menuDefinitionRoot);

			return true;
		}

		/// <summary>
		/// Quicker version of the above for applications that have to validate large numbers of menus.
		/// </summary>
		/// <param name="xRoot">The template for the menu. (The 'menu_system' definition document element)</param>
		/// <param name="menuDelegate">The delegate to call when the menu is changed by the user.</param>
		/// <param name="bProcessEnums">Set to false if there are no enums to processin the template.</param>
		/// <param name="bExpandMacros">Set to false if the template contains no macros. (... no 'inherit' tags.)</param>
		/// <returns></returns>
		public bool Initialise(XmlElement xRoot, GetMenuDelegate menuDelegate, bool bProcessEnums, bool bExpandMacros)
		{
			GetMenu = menuDelegate;

			m_menuDefinitionRoot = xRoot;

			if(bProcessEnums == true)
				PopulateEnumList(xRoot, m_enumTable);

			if(bExpandMacros == true)
				MacroExpand(m_menuDefinitionRoot);

			return true;
		}

		public bool CheckContainerNamesAreUnique()
		{
			try
			{
				if(m_menuDefinitionRoot != null)
				{
					m_containerNames = new Dictionary<string, XmlElement>();

					XmlNode menuDefinitionRootContainer = m_menuDefinitionRoot.SelectSingleNode("container");
					if(menuDefinitionRootContainer != null)
						return _CheckContainerNamesAreUnique(menuDefinitionRootContainer as XmlElement);
				}
				return true;
			}
			catch
			{
				return false;
			}
			finally
			{
				m_containerNames = null;
			}
		}

		public string CheckContainerNamesAreUniqueErrorString = null;

		private bool _CheckContainerNamesAreUnique(XmlElement xSourceElement)
		{
			string strType = xSourceElement.Name;
			if(strType == "container")
			{
				string strName = XmlHelper.GetKeyValue(xSourceElement, "name");

				if(m_containerNames.ContainsKey(strName))
				{
					CheckContainerNamesAreUniqueErrorString = "The name '" + strName + "' is multiply defined.";
					return false;
				}
				else
				{
					m_containerNames.Add(strName, xSourceElement);
				}

				XmlNodeList xSourceNodes = xSourceElement.ChildNodes;
				if(xSourceNodes != null)
				{
					// Use a loop rather than a foreach since _MacroExpand does a 'ReplaceChild' that buggers up 'foreach' iteration.
//					for(int c = 0; c < xSourceNodes.Count; c++)
					foreach(XmlNode xChildSourceNode in xSourceNodes)
					{
//						XmlNode xChildSourceNode = xSourceNodes[c];
						if(xChildSourceNode is XmlElement)
							if(_CheckContainerNamesAreUnique(xChildSourceNode as XmlElement) == false)
								return false;
					}
				}
			}
			return true;
		}


		/// <summary>
		/// Recursively substitutes any occurences of the 'inherit' tag with it's expansion.
		/// </summary>
		/// <param name="m_menuDefinitionDocument">The document to expand.</param>
		private void MacroExpand(XmlElement menuDefinitionRoot)
		{
			if(menuDefinitionRoot != null)
			{
				XmlNode menuDefinitionRootContainer = menuDefinitionRoot.SelectSingleNode("container");
				if(menuDefinitionRootContainer != null)
					_MacroExpand(menuDefinitionRootContainer);
			}
		}

		/// <summary>
		/// See MacroExpand.
		/// </summary>
		/// <param name="xSourceNode"></param>
		/// <returns></returns>
		bool _MacroExpand(XmlNode xSourceNode)
		{
			string strType = xSourceNode.Name;

			if(strType == "inherit")
			{
				string strPath = XmlHelper.GetKeyValue(xSourceNode, "path");
				string strName = XmlHelper.GetKeyValue(xSourceNode, "name");

				XmlNode xInherit = m_menuDefinitionRoot.SelectSingleNode("reference");
				
				XmlNode xInheritedNode = XmlHelper.GetNodeFromKeyPath(xInherit, "name", strPath);

				if(xInheritedNode != null)
				{
					XmlNode xNewChild = xInheritedNode.CloneNode(true);

					if(strName != null && strName.Length != 0)
						XmlHelper.AddAttribute(xNewChild, "name", strName);

					xSourceNode.ParentNode.ReplaceChild(xNewChild, xSourceNode);

					_MacroExpand(xNewChild);
				}
			}

			XmlNodeList xSourceNodes = xSourceNode.ChildNodes;
			if(xSourceNodes != null)
			{
				// Use a loop rather than a foreach since _MacroExpand does a 'ReplaceChild' that buggers up 'foreach' iteration.
				for(int c = 0; c < xSourceNodes.Count; c++)
				{
					XmlNode xChildSourceNode = xSourceNodes[c];
					_MacroExpand(xChildSourceNode);
				}
			}
			return true;
		}

		/// <summary>
		/// Clears the current menu and populates it with a textbox containing the given string.
		/// </summary>
		/// <param name="strError">String to display.</param>
		/// <returns></returns>
		public bool SetError(string strError)
		{
			// Delete any controls in the form.
			if(this.rootMenuContainer.HasChildren == true)
			{
				this.rootMenuContainer.Controls.Clear();
				//				GC.Collect();                       // XXX Debug.
			}

			XmlDocument xDoc = new XmlDocument();



			xDoc.Load(
				new StringReader(
						"<container name = 'Status' type = 'basic' display_name = 'Status'>" +
							"<container name='text'	display_name='' type='label'	default='No selection' />" +
						"</container>"));

			XmlHelper.AddAttribute(xDoc["container"]["container"], "display_name", strError);

			this.SetMenu(xDoc.FirstChild as XmlElement, false);

			return true;
		}

		/// <summary>
		/// Parse the enum definitions in the given document and add them to the given map.
		/// </summary>
		/// <param name="xDoc">Document to parse for enums.</param>
		/// <param name="enumTable">Map to add the enums to.</param>
		/// <returns></returns>
        bool PopulateEnumList(XmlElement xRoot, Dictionary<string, List<string>>enumTable)
        {
			bool bRetVal = false;
			enumTable.Clear();
			if (xRoot != null)
			{
				XmlNodeList xEnums = xRoot.SelectNodes("enums/enum");

				if (xEnums != null)
				{
					foreach (XmlNode n in xEnums)
					{
						if (n.NodeType == XmlNodeType.Element)
						{
							string strEnumName = XmlHelper.GetKeyValue(n, "name");
							Debug.WriteLine("Enum name: " + strEnumName);
							List<string>strEnumValues = new List<string>();
							string strEntries = n.InnerText;
							string strCurrentValue = "";
							bool instring = false;

							for (int c = 0; c < strEntries.Length; c++)
							{
								if (strEntries[c] == '\"')
								{
									if (instring == true)
									{
										strEnumValues.Add(strCurrentValue);
										strCurrentValue = "";
										instring = false;
									}
									else
									{
										instring = true;
									}
								}
								else
								{
									if (instring == true)
									{
										strCurrentValue += strEntries[c];
									}
									else
									{
									}
								}
							}
							enumTable.Add(strEnumName, strEnumValues);
							bRetVal = true;
						}
					}
				}
			}
			return bRetVal;
        }

		public XmlDocument XmlValidateMenu(XmlElement xCurrentMenu)
		{
			return BuildFullMenuDocument(m_menuDefinitionRoot, xCurrentMenu);
		}

		/// <summary>
		/// Creates a menu after optionally merging the menu definition document with the provided current menu state.
		/// </summary>
		/// <param name="xCurrentMenu">The current menu state.</param>
		/// <param name="bValidateAgainstTemplate">Whether or not to construct a menu in the image of the template.</param>
		/// <param name="bMergeWithExistingControls">Whether to clear the menu first or try and update an existing menu.</param>
		/// <returns></returns>
		public bool SetMenu(XmlElement xCurrentMenu, bool bValidateAgainstTemplate)
		{
			if(bValidateAgainstTemplate == true)
			{
				if(m_menuDefinitionRoot == null)
					return false;

				try
				{
					m_xCurrentMenuDocument = BuildFullMenuDocument(m_menuDefinitionRoot, xCurrentMenu);
				}
				catch(Exception ex)
				{
					SetError(ex.Message);
					return false;
				}
			}
			else
			{
				m_xCurrentMenuDocument = new XmlDocument();
				m_xCurrentMenuDocument.AppendChild(m_xCurrentMenuDocument.ImportNode(xCurrentMenu.SelectSingleNode("container"), true));
			}

			Control baseContainer = null;
			try
			{
				baseContainer = this.rootMenuContainer;
			}
			catch
			{ }


			List<XmlElement>ar = new List<XmlElement>();
			ar.Add(m_xCurrentMenuDocument.SelectSingleNode("/container") as XmlElement);
			UpdateMenuContainers(ar, baseContainer as Container_Base);
			//this.rootMenuContainer.Controls.Clear();
			//Container_Base retVal = CreateMenuContainer(null, m_xCurrentMenuDocument.SelectSingleNode("/container") as XmlElement, true);
			//this.rootMenuContainer.Controls.Add(retVal);




//			if(m_bChanged == true)
//			{
			MenuValueChanged(null);				// If the template changes the menu, the owner app. gets to know about it.
//			}

			if(rootMenuContainer.Controls.Count != 0)
			{
				Container_Base cb = rootMenuContainer.Controls[0] as Container_Base;
				cb.LayoutChildren();
			}

			return true;
		}


		public bool ReDrawMenu()
		{
			if(rootMenuContainer.Controls.Count != 0)
			{
				Container_Base cb = rootMenuContainer.Controls[0] as Container_Base;
				cb.LayoutChildren();
				return true;
			}
			return false;
		}

		public bool SetWidthToContents()
		{
			if(rootMenuContainer.Controls.Count != 0)
			{
				Container_Base cb = rootMenuContainer.Controls[0] as Container_Base;
				this.Width = cb.Width + 3;
				this.Height = cb.Height + 3;
			}
			return true;
		}

		//Container_Base UpdateMenuContainers(List<XmlElement>xContainerList, Control containerParent)
		Container_Base UpdateMenuContainers(List<XmlElement>xContainerList, Container_Base containerParent)
		{
			try
			{
				if(containerParent != null)
				{
					if(containerParent.ChildContainer != null)
						foreach(Container_Base containerBase in containerParent.ChildContainer.Controls)
						{
							if(containerBase is Container_Base)
								containerBase.Tag = false;
							else
								containerBase.Tag = true;
						}

					if(xContainerList != null)
						foreach(XmlElement xContainer in xContainerList)
						{
							if(xContainer != null)
							{
								string strType = XmlHelper.GetKeyValue(xContainer, "type");

								Container_Base container = FindAndTagMatchingControl(containerParent, xContainer);		// Matches name and type. Only considers unclaimed controls and claims them if a match is found.
								//Container_Base container = containerParent.FindAndTagMatchingControl(xContainer);		// Matches name and type. Only considers unclaimed controls and claims them if a match is found.

								if(container == null)
								{
									//Container_Base retVal = CreateMenuContainer(containerParent as Container_Base, xContainer, true);
									//containerParent.Controls.Add(retVal);
									//retVal.Tag = true;
									container = CreateMenuContainer(containerParent as Container_Base, xContainer, true);
									containerParent.ChildContainer.Controls.Add(container);
									container.Tag = true;
								}
								else
								{
									#region debug
									string strName = XmlHelper.GetKeyValue(xContainer, "name");
									//							string strType = XmlHelper.GetKeyValue(xContainer, "type");
									/// The following must be true since it has been confirmed in FindMatchingControl.
									Debug.Assert(strName == container.Name);
									Debug.Assert(strType == container.strType);
									#endregion debug

									container.m_xObject = xContainer;			// Do this before setting the value, so the treeView can work.

									string strValue = XmlHelper.GetKeyValue(xContainer, "value");
									string strDisplayName = XmlHelper.GetKeyValue(xContainer, "display_name");

									#region Set container name and value.
									if(strValue != "ignore me")
									{
										container.ObjectValue = strValue;
									}

									if(strDisplayName != "ignore me")
									{
										container.ObjectName = strDisplayName;
									}

									#endregion

								}
								//UpdateMenuContainers(container.xChildren, container.ChildContainer);
								UpdateMenuContainers(container.xChildren, container);
							}
						}


					//foreach(Container_Base cb in containerParent.Controls)
					if(containerParent.ChildContainer != null)
					{
						List<Container_Base> removeList = new List<Container_Base>(containerParent.Controls.Count);
						foreach(Container_Base cb in containerParent.ChildContainer.Controls)
						{
							if((bool)cb.Tag == false)
							{
								removeList.Add(cb);
							}
						}
						foreach(Container_Base cb in removeList)			/// Some will have been removed already by FindMatchingControl.
						{
							containerParent.Controls.Remove(cb);
						}
					}
				}
				else
				{
					Debug.Assert(xContainerList.Count == 0);
				}
			}
			catch(Exception ex)
			{
				Debug.Assert(false, "Exception : " + ex.Message);
				Debug.WriteLine("Exception : " + ex.Message);
			}
			return null;
		}

		private Container_Base FindAndTagMatchingControl(Container_Base containerParent, XmlElement xContainer)
		{
			string strName = XmlHelper.GetKeyValue(xContainer, "name");
			string strType = XmlHelper.GetKeyValue(xContainer, "type");

			Container_Base retVal = null;

			if(containerParent != null)
			{
				//foreach(Container_Base cb in control.Controls)
				foreach(Container_Base cb in containerParent.ChildContainer.Controls)
				{
					if((bool)cb.Tag == false)						// Only consider unclaimed controls.
					{
						if(cb.Name == strName)
						{
							if(cb.strType == strType)
							{
								retVal = cb;
								cb.Tag = true;						// Mark this control as in use.
								break;
							}
							else
							{
								retVal = null;
								break;
							}
						}
						containerParent.ChildContainer.Controls.Remove(cb);				// Remove unclaimed any controls that have been skipped over. This ensures we maintain order,
																	// since if the control has been 'moved', it will be recreated in the correct place.
					}
				}
			}
			return retVal;
		}

		public bool SetSingleContainer(XmlElement xContainer)
		{
			return UpdateSingleContainer(this.rootMenuContainer, xContainer);
		}

		internal bool UpdateSingleContainer(Control containerParent, XmlElement xContainer)
		{
			string strName = XmlHelper.GetKeyValue(xContainer, "name");
			string strType = XmlHelper.GetKeyValue(xContainer, "type");

			Container_Base cb = FindMatchingControlRecursively(containerParent, strName, strType);

			if(cb != null)
			{
				string strValue = XmlHelper.GetKeyValue(xContainer, "value");
				if(strValue != null)
					cb.ObjectValue = strValue;

				string strDisplayName = XmlHelper.GetKeyValue(xContainer, "display_name");
				if(strDisplayName != null)
					cb.ObjectName = strDisplayName;

				return true;
			}

			return false;
		}

		private Container_Base FindMatchingControlRecursively(Control control, string strName, string strType)
		{
			if(control != null)
			{
//				foreach(Container_Base cb in control.Controls)
				foreach(Control childControl in control.Controls)
				{
					if(childControl is Container_Base)
					{
						Container_Base cb = childControl as Container_Base;
						if(cb.Name == strName)
						{
//							if(cb.strType == strType)		// Just key on the name.
//							{
								return cb;
//							}
						}
					}
					Container_Base retVal = FindMatchingControlRecursively(childControl, strName, strType);
					if(retVal != null)
						return retVal;

				}
			}
			return null;
		}

		/// <summary>
		/// Creates a menu WITHOUT merging the menu definition document with the provided current menu state.
		/// </summary>
		/// <param name="xCurrentMenu">The current menu state.</param>
		/// <returns></returns>
		public bool ForceMenu(XmlElement xCurrentMenu)
		{
			return SetMenu(xCurrentMenu, false);
		}

		/// <summary>
		/// This function creates a new Xml menu by copying xDefinitionRoot.
		/// The values of each container in the new menu are taken from matching containers in xCurrentDocument.
		/// If xCurrentDocument does not contain a corresponding container then the value of this new container
		/// is set to its default value as defined by xDefinitionRoot.
        /// Essentially it's a merge between the two documents passed in, where xDefinitionRoot defines the
		/// contents of the result and xCurrentDocument defines the values of the contents.
		/// </summary>
		/// <param name="xDefinitionRoot">Xml element defining the template for this menu.</param>
		/// <param name="xCurrentDocument">Xml element describing the current state of the menu.
		/// Note that it may be null.</param>
        /// <returns>A new Xml menu document.</returns>
        public XmlDocument BuildFullMenuDocument(XmlElement xDefinitionRoot, XmlElement xCurrentDocument)
        {
			XmlNode menuDefinitionRootContainer = xDefinitionRoot.SelectSingleNode("container");
			if(menuDefinitionRootContainer == null)
				throw new Exception("Cannot find the entry 'container' in the Xml definition document.");

			XmlNode currentMenuRootContainer = null;
			if(xCurrentDocument != null)			// HACK?
				currentMenuRootContainer = xCurrentDocument.SelectSingleNode("container");

			XmlDocument xNewMenu = new XmlDocument();
			XmlNode nodeDest = xNewMenu.ImportNode(menuDefinitionRootContainer, true);
			xNewMenu.AppendChild(nodeDest);

			m_bChanged = false;
            _BuildFullMenuDocument(xNewMenu["container"], currentMenuRootContainer);

			return xNewMenu;
        }

		/// <summary>
		/// Recurse each 'container' in xSourceNode and create a 'value' key. Set the value of this new key to the
		/// corrsponding 'value' found in xCurrentNode if there is one, otherwise to the value of 'default' in xSourceNode.
		/// </summary>
		/// <param name="xSourceNode">The Xml node that needs it's 'value' key set.</param>
		/// <param name="xCurrentNode">The node from which the 'value' key is taken, or null.</param>
		/// <returns>true</returns>
        bool _BuildFullMenuDocument(XmlNode xSourceNode, XmlNode xCurrentNode)
        {
			string strValue = null;

            bool bSaveFlag = XmlHelper.GetKeyBool(xCurrentNode, "save", true);

			if(bSaveFlag == true)
			{
				if(xCurrentNode != null)
				{
					strValue = XmlHelper.GetKeyValue(xCurrentNode, "value");
				}
			}

			if(strValue == null)
			{
				strValue = XmlHelper.GetKeyValue(xSourceNode, "default");
				m_bChanged = true;
			}
			if(strValue == null)
			{
				strValue = UNINITIALISED_MARKER;
				m_bChanged = true;
			}

			XmlHelper.AddAttribute(xSourceNode, "value", strValue);

			Dictionary<string, int> containerNameCollision = new Dictionary<string, int>();

			foreach(XmlNode xChildSourceNode in xSourceNode)
			{
				if(xChildSourceNode.NodeType == XmlNodeType.Element)
				{
					if(xChildSourceNode.Name == "container")
					{
						XmlNode xChildCurrentNode = null;
						if(xCurrentNode != null)
						{
							string strName = XmlHelper.GetKeyValue(xChildSourceNode, "name");

							List<XmlElement> xChildList			= XmlHelper.FindElementsByKeyValue(xCurrentNode as XmlElement, "name", strName, false, false, true);
							List<XmlElement> xSourceChildList	= XmlHelper.FindElementsByKeyValue(xSourceNode as XmlElement, "name", strName, false, false, true);


							// If there is more than one 
							if(xSourceChildList.Count > 1 && xChildList.Count > 1)
							{
								if(containerNameCollision.ContainsKey(strName) == true)
								{
									containerNameCollision[strName]++;

									xChildCurrentNode = xChildList.Count > containerNameCollision[strName] ? xChildList[containerNameCollision[strName]] : null;
								}
								else
								{
									containerNameCollision.Add(strName, 0);
									xChildCurrentNode = xChildList[0];
									MessageBox.Show(this, "Your template is malformed. The container name\n'" + strName + "'\nis multiply defined at the same nest level.", "Menu System", MessageBoxButtons.OK);
								}

							}
							else
								xChildCurrentNode = xChildList.Count == 1 ? xChildList[0] : null;
						}

						//Debug.WriteLine(xChildSourceNode.Name);
						_BuildFullMenuDocument(xChildSourceNode, xChildCurrentNode);
					}
				}
			}
			return true;
        }


		public XmlDocument MergeMenuDocuments(XmlElement xMasterMenu, XmlElement xSlaveMenu, bool bMasterIsTemplate)
		{
			//XmlNode xMasterRootContainer = xMasterMenu.SelectSingleNode("container");
			//if(xMasterRootContainer == null)
			//    throw new Exception("Cannot find the entry 'container' in the Xml definition document.");

			//XmlNode xSlaveRootContainer = null;
			//if(xSlaveMenu != null)			// HACK?
			//    xSlaveRootContainer = xSlaveMenu.SelectSingleNode("container");

			XmlNode xMasterRootContainer = xMasterMenu;
			XmlElement xSlaveRootContainer = xSlaveMenu;


			XmlDocument xNewMenu = new XmlDocument();
			XmlNode nodeDest = xNewMenu.ImportNode(xMasterRootContainer, true);
			xNewMenu.AppendChild(nodeDest);

			m_bChanged = false;
			_MergeMenuDocuments(xNewMenu["container"], xSlaveRootContainer, false);

			return xNewMenu;
		}

		bool _MergeMenuDocuments(XmlElement xMasterNode, XmlElement xSlaveNode, bool bMasterIsTemplate)
		{
			// Merge these two elements.

			if(xSlaveNode != null)
			{
				if(XmlHelper.GetKeyValue(xMasterNode, "type") == XmlHelper.GetKeyValue(xSlaveNode, "type"))
				{
					string strValue1 = XmlHelper.GetKeyValue(xMasterNode, "value");
					string strValue2 = XmlHelper.GetKeyValue(xSlaveNode, "value");

					if(strValue1 == strValue2)
					{
						// Leave the value in Master alone.
					}
					else if(strValue1 == null)
					{
						if(strValue2 != null)
							XmlHelper.AddAttribute(xMasterNode, "value", strValue2);
						else
							XmlHelper.AddAttribute(xMasterNode, "value", UNINITIALISED_MARKER);
					}
					else if(strValue2 == null)
					{
						// Leave the value in Master alone.
					}
					else
					{
						XmlHelper.AddAttribute(xMasterNode, "value", CONFLICTED_MARKER);		// Otherwise mark the entry as conflicted.
					}
				}
				else
				{
					string strName = XmlHelper.GetKeyValue(xMasterNode, "name");
					xMasterNode.RemoveAll();													// We're not interested in the children of a conflicted merge container.
					XmlHelper.AddAttribute(xMasterNode, "name", strName);
					XmlHelper.AddAttribute(xMasterNode, "type", "merge_conflict");
					// TODO: Fill in the 'value' key with details about the conflict.
				}
			}

			// then ...

			// Find any child containers that exist only in the slave and copy them to the master ...

			XmlNodeList masterChildNodeList = xMasterNode.SelectNodes("container");
			Dictionary<string, XmlElement>masterChildList = new Dictionary<string,XmlElement>(masterChildNodeList.Count);

			foreach(XmlElement xMasterChild in masterChildNodeList)
			{
				masterChildList.Add(XmlHelper.GetKeyValue(xMasterChild, "name"), xMasterChild);
			}

			if(xSlaveNode != null)
			{
				XmlNodeList slaveChildNodeList = xSlaveNode.SelectNodes("container");
				Dictionary<string, XmlElement> slaveChildList = new Dictionary<string, XmlElement>(slaveChildNodeList.Count);

				foreach(XmlElement xSlaveChild in slaveChildNodeList)
				{
					string strName = XmlHelper.GetKeyValue(xSlaveChild, "name");
					slaveChildList.Add(strName, xSlaveChild);									// Is this list needed?

					XmlElement xMasterChild;
					if(masterChildList.TryGetValue(strName, out xMasterChild) == true)
					{
						// The Master and Slave have a child in common. Let the next recurse deal with it.
					}
					else
					{
						// The slave has a child that the master does not.
						if(bMasterIsTemplate == false)													// If the master wants the slave children ...
						{																				// (... as any good master should ...)
							xMasterNode.AppendChild(xMasterNode.OwnerDocument.ImportNode(xSlaveChild, true));
						}
					}
				}

				foreach(XmlElement xMasterChild in masterChildNodeList)
				{
					XmlElement xSlaveChild = null;
					if(slaveChildList.TryGetValue(XmlHelper.GetKeyValue(xMasterChild, "name"), out xSlaveChild) == true)
						_MergeMenuDocuments(xMasterChild, xSlaveChild, bMasterIsTemplate);
					else
						_MergeMenuDocuments(xMasterChild, null, bMasterIsTemplate);
				}
			}
			else
			{
				foreach(XmlElement xMasterChild in masterChildNodeList)
				{
					_MergeMenuDocuments(xMasterChild, null, bMasterIsTemplate);
				}
			}

			return true;
		}


		//////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////
		internal int m_ignoreChangedEventHandler = 0;

		/// <summary>
		/// Creates a menu container (a user control) from an Xml container definition, and calls it's PostConstructor.
		/// </summary>
		/// <param name="xContainer">Xml container definition.</param>
		/// <param name="bCreateChildren">Have the container recursively create it's children?</param>
		/// <returns></returns>
		internal Container_Base CreateMenuContainer(Container_Base parent, XmlElement xContainer, bool bCreateChildren)
		{
			m_ignoreChangedEventHandler++;				// Disable events on the containers for the duration of it's creation.

			Container_Base menuContainer = null;

			string strContainerType = XmlHelper.GetKeyValue(xContainer, "type");
			switch(strContainerType)
			{
				case "action_button":		// container_action_button.cs
					{
						menuContainer = new Container_ActionButton(this, xContainer, bCreateChildren);
					}
					break;

				case "basic":
					{
						menuContainer = new Container_Basic(this, xContainer, bCreateChildren);
					}
					break;

				case "bitmap":
					{
						menuContainer = new Container_Bitmap(this, xContainer, bCreateChildren);
					}
					break;

				case "bool":
					{
						menuContainer = new Container_Bool(this, xContainer, bCreateChildren);
					}
					break;

				case "colour":
					{
						menuContainer = new Container_Colour(this, xContainer, bCreateChildren);
					}
					break;

				case "dropdown":			// container_dropdown.cs
					{
						menuContainer = new Container_Dropdown(this, xContainer, bCreateChildren);
					}
					break;

				case "enum":
					{
						menuContainer = new Container_Enum(this, xContainer, bCreateChildren);
					}
					break;

				case "filename":			// container_filename.cs
					{
						menuContainer = new Container_Filename(this, xContainer, bCreateChildren);
					}
					break;
				case "folder":			// container_filename.cs
					{
						menuContainer = new Container_FolderBrowser(this, xContainer, bCreateChildren);
					}
					break;

				case "network_filename":	// Depracate?
					{
						menuContainer = new Container_Filename(this, xContainer, bCreateChildren);
					}
					break;

				case "x360name":			// container_get_default_360_console.cs
					{
						menuContainer = new Container_GetDefault360(this, xContainer, bCreateChildren);
					}
					break;

				case "label":
					{
						menuContainer = new Container_Label(this, xContainer, bCreateChildren);
					}
					break;

				case "int":					// container_number.cs
					{
						menuContainer = new Container_Number(this, xContainer, bCreateChildren);
					}
					break;

				case "float":				// container_number.cs
					{
						//						menuContainer = new Container_Float(this, xContainer, bCreateChildren);
						menuContainer = new Container_Number(this, xContainer, bCreateChildren);
					}
					break;

				case "progress_bar":
					{
						menuContainer = new Container_Progressbar(this, xContainer, bCreateChildren);
					}
					break;

				case "radio_button":
					{
						Container_RadioButton crb = new Container_RadioButton(this, xContainer, bCreateChildren);
						parent.AddContainerRadioButton(crb);
						crb.m_rbGroupOwner = parent;
//						crb.m_radioParent = parent;
						menuContainer = crb;
					}
					break;

				case "string":
					{
						menuContainer = new Container_String(this, xContainer, bCreateChildren);
					}
					break;

				case "text":
					{
						menuContainer = new Container_Text(this, xContainer, bCreateChildren);
					}
					break;

				case "tree":
					{
						menuContainer = new Container_Tree(this, xContainer, bCreateChildren);
					}
					break;

				case "webpage":
					{
						menuContainer = new Container_Webpage(this, xContainer, bCreateChildren);
					}
					break;
				case "custom_list":
					{
						menuContainer = new Container_CustomList(this, xContainer, bCreateChildren);
					}
					break;

                default:
					{
						menuContainer = new Container_Unknown(this, xContainer, bCreateChildren);		// Rock on!
					}
					break;
			}

			menuContainer.CreateControl();					// This causes the 'Load' event to fire. (I think).
			menuContainer.PostConstructor();

			m_ignoreChangedEventHandler--;

			return menuContainer;
		}

		/// <summary>
		/// Have the menu lay itself out nicely and auto-size all it's children.
		/// </summary>
		internal void LayoutAllChildren()
		{
			Container_Base cb = rootMenuContainer.Controls[0] as Container_Base;
//			cb.SuspendLayout();
			cb.LayoutChildren();
//			cb.ResumeLayout(true);
		}

		//public bool SetStatus(string strText)
		//{
		//    this.toolStripStatusLabel.Text = strText;
		//    return true;
		//}


		internal void ProcessDependencies()
		{
			Container_Base cb = rootMenuContainer.Controls[0] as Container_Base;
			//			cb.SuspendLayout();
			cb.ProcessDependencies();
			//			cb.ResumeLayout(true);
		}

		/// <summary>
		/// Creates an Xml document from an Xml string.
		/// If the string is bad it gives the user options to correct the error.
		/// </summary>
		/// <param name="strMenuString">Xml string</param>
		/// <returns>Xml document created from the string.</returns>
		public XmlDocument CreateXmlDocumentFromString(string strMenuString)
		{
			XmlDocument docCurrentMenu = new XmlDocument();

			if(strMenuString != null && strMenuString.Length > 0)
			{
				bool bRetry;
				do
				{
					System.IO.StringReader sr = new System.IO.StringReader(strMenuString);

					bRetry = false;
					try
					{
						docCurrentMenu.Load(sr);
					}
					catch(Exception ex)
					{
						// TODO: Throw a useful exception to the caller.
						ModalBadString mbs = new ModalBadString("Error", "The string presented is bad.\n\nDetails:\n" + ex.ToString());
						mbs.Text = "Error";
						DialogResult dr = mbs.ShowDialog(this);
						//						if(dr == DialogResult.OK)
						switch(mbs.Result)
						{
							case ModalBadString.result.EditString:
								{
									ModalEditText met = new ModalEditText(true);
									met.mText = strMenuString;

									DialogResult dr2 = met.ShowDialog(this);
									if(dr2 == DialogResult.OK)
										strMenuString = met.mText;
									bRetry = true;
								}
								break;

							case ModalBadString.result.CreateDefault:
								{

								}
								break;

							case ModalBadString.result.Ignore:
								{
									SetError("The values for this entry cannot be shown.");
									return null;
								}

							default:
								{
								}
								break;
						}
					}
				} while(bRetry == true);
			}
			return docCurrentMenu;
		}


		//public void ShowStatusStrip(bool bShow)
		//{
		//    this.statusStrip.Visible = bShow;
		//}
	}
}
