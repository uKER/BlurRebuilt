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
		#region MergeMode enum

		public enum MergeMode
		{
			MasterIsTemplate = 0, // Resulting document fragment structure matches the master document.
			MustExistInBoth, // Resulting document fragment has elements common to both master and slave. Terminated by a merge conflict.
			MayExistInEither // Resulting document fragment has elements from both master and slave. Terminated by a merge conflict.
		}

		#endregion

		public static string CONFLICTED_MARKER = "*CONFLICTED*";
		public static string UNINITIALISED_MARKER = "**UNINITIALISED**";
		private bool m_bMenuIsInErrorState = false;
		public bool m_bReadOnly = false;

		public Dictionary<string, List<string>> m_enumTable = new Dictionary<string, List<string>>();
		internal int m_ignoreChangedEventHandler = 0;

		/// <summary>
		/// This document defines the layout of the menu.
		/// The application may modify it during execution.
		/// </summary>
		internal XmlElement m_menuDefinitionRoot;

		/// <summary>
		/// This document defines the current values of the menu.
		/// It is passed to the MenuValueChanged event each time it is changed.
		/// </summary>
		private XmlDocument m_xCurrentMenuDocument;

		public MenuSystem()
		{
			InitializeComponent();
			SetStyle(ControlStyles.Selectable, false); // Disallow receiving focus.
		}

		public bool MenuIsInErrorState
		{
			get { return m_bMenuIsInErrorState; }
			//set { m_bMenuIsInErrorState = value; }
		}

		public event EventHandler<MenuSystemChangedEventArgs> MenuChangedEvent;

		protected void OnMenuChanged(XmlElement xChangedElement, bool bValueNeedsToBeSaved)
		{
			if(xChangedElement != null)
				Debug.Assert(xChangedElement.OwnerDocument == m_xCurrentMenuDocument);

			if(MenuChangedEvent != null)
			{
				MenuChangedEvent(this, new MenuSystemChangedEventArgs(m_xCurrentMenuDocument, xChangedElement, bValueNeedsToBeSaved));
			}
		}

		public event EventHandler<MenuComponentCreatedEventArgs> MenuComponentCreatedEvent;

		protected void OnMenuComponentCreated(Container_Base createdComponent)
		{
			if(MenuComponentCreatedEvent != null)
			{
				MenuComponentCreatedEvent(this, new MenuComponentCreatedEventArgs(createdComponent));
			}
		}


		/// <summary>
		/// This function is called each time a menu value is changed.
		/// It is called with (null) after the menu is first generated.
		/// </summary>
		/// <param name="control">Reference to the control that has changed.</param>
		/// <returns></returns>
		internal bool MenuValueChanged(Container_Base control)
		{
			if(m_bMenuIsInErrorState == false)
			{
//control.m_xObject
				//Debug.WriteLine("MenuValueChanged.");

				//Debug.Assert(control != null);

				if(control == null)
					OnMenuChanged(null, false);
				else
					OnMenuChanged(control.m_xObject, control.m_bSaveValue);

				return true;
			}
			return false;
		}

		internal bool MenuComponentCreated(Container_Base control)
		{
			OnMenuComponentCreated(control);
			return true;
		}
		/// <summary>
		/// Initialises the menu system with a template and an event to call when the menu changes.
		/// </summary>
		/// <param name="xRoot">The template for the menu. (The 'menu_system' definition document element)</param>
		/// <param name="menuDelegate">The delegate to call when the menu is changed by the user.</param>
		/// <returns>true</returns>
		public bool Initialise(XmlElement xRoot)
		{
			return Initialise(xRoot, true, true);
		}

		/// <summary>
		/// Quicker version of the above for applications that have to validate large numbers of menus.
		/// </summary>
		/// <param name="xRoot">The template for the menu. (The 'menu_system' definition document element)</param>
		/// <param name="menuDelegate">The delegate to call when the menu is changed by the user.</param>
		/// <param name="bProcessEnums">Set to false if there are no enums to processin the template.</param>
		/// <param name="bExpandMacros">Set to false if the template contains no macros. (... no 'inherit' tags.)</param>
		/// <returns></returns>
		public bool Initialise(XmlElement xRoot, bool bProcessEnums, bool bExpandMacros)
		{
			MenuChangedEvent = null;

			m_menuDefinitionRoot = xRoot;

			if(bProcessEnums == true)
				PopulateEnumList(xRoot, m_enumTable);

			if(bExpandMacros == true)
				MacroExpand(m_menuDefinitionRoot);

			return true;
		}

		/// <summary>
		/// Checks whether the 'value' of the 'name' keys in all elements of type 'container' are unique.
		/// </summary>
		/// <returns>Returns the first element that has a duplicated name, otherwise null</returns>
		public XmlElement CheckContainerNamesAreUnique()
		{
			if(m_menuDefinitionRoot != null)
			{
				XmlNodeList xList = m_menuDefinitionRoot.SelectNodes(".//container");
				//XmlNodeList xList = m_menuDefinitionRoot.SelectNodes("/container//container"); ?????

				Dictionary<string, XmlElement> containerNames = new Dictionary<string, XmlElement>(xList.Count);

				foreach(XmlElement xElement in xList)
				{
					string strName = XmlHelper.GetKeyValue(xElement, "name");

					if(containerNames.ContainsKey(strName) == true)
					{
						return xElement;
					}
					else
						containerNames.Add(strName, xElement);
				}
			}
			return null;
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
		private bool _MacroExpand(XmlNode xSourceNode)
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

			xDoc.Load
				(new StringReader
				 	(
				 	"<container name = 'Status' type = 'basic' display_name = 'Status'>" +
				 	"<container name='text'	display_name='' type='label'	default='No selection' />" +
				 	"</container>"));

			//XmlHelper.AddAttribute(xDoc["container"]["container"], "display_name", strError);
			XmlHelper.AddAttribute(xDoc["container"]["container"], "value", strError);

			this._SetMenu(xDoc.FirstChild as XmlElement, false, true);

			return true;
		}

		/// <summary>
		/// Parse the enum definitions in the given document and add them to the given map.
		/// </summary>
		/// <param name="xDoc">Document to parse for enums.</param>
		/// <param name="enumTable">Map to add the enums to.</param>
		/// <returns></returns>
		private bool PopulateEnumList(XmlElement xRoot, Dictionary<string, List<string>> enumTable)
		{
			bool bRetVal = false;
			enumTable.Clear();
			if(xRoot != null)
			{
				XmlNodeList xEnums = xRoot.SelectNodes("enums/enum");

				if(xEnums != null)
				{
					foreach(XmlNode n in xEnums)
					{
						if(n.NodeType == XmlNodeType.Element)
						{
							string strEnumName = XmlHelper.GetKeyValue(n, "name");
							Debug.WriteLine("Enum name: " + strEnumName);
							List<string> strEnumValues = new List<string>();
							string strEntries = n.InnerText;
							string strCurrentValue = "";
							bool instring = false;

							for(int c = 0; c < strEntries.Length; c++)
							{
								if(strEntries[c] == '\"')
								{
									if(instring == true)
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
								else if(instring == true)
								{
									strCurrentValue += strEntries[c];
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

		/// <summary>
		/// This is marked obsolete because I don't know if it is used anywhere, therefore i can't
		/// be sure the modification I have made works properly.
		/// 
		/// </summary>
		/// <param name="xCurrentMenu"></param>
		/// <returns></returns>
		[Obsolete]
		public XmlDocument XmlValidateMenu(XmlElement xCurrentMenu)
		{
			return MergeMenuDocuments(m_menuDefinitionRoot["container"], xCurrentMenu["container"], MergeMode.MasterIsTemplate);

			//return BuildFullMenuDocument(m_menuDefinitionRoot, xCurrentMenu);
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
			if(InvokeRequired)
			{
				bool retval = false;
				this.Invoke((MethodInvoker) delegate() { retval = SetMenu(xCurrentMenu, bValidateAgainstTemplate); });
				return retval;
			}

			return _SetMenu(xCurrentMenu, bValidateAgainstTemplate, false);
		}


		private bool _SetMenu(XmlElement xCurrentMenu, bool bValidateAgainstTemplate, bool bMenuIsInErrorState)
		{
			m_bMenuIsInErrorState = bMenuIsInErrorState;

			if(xCurrentMenu == null)
			{
				SetError("");
				return false;
			}

			if(bValidateAgainstTemplate == true)
			{
				if(m_menuDefinitionRoot == null)
					return false;

				try
				{
// LOOKY
					//m_xCurrentMenuDocument = MergeMenuDocuments(m_menuDefinitionRoot["container"], xCurrentMenu["container"], true);
					m_xCurrentMenuDocument = MergeMenuDocuments(m_menuDefinitionRoot["container"], xCurrentMenu, MergeMode.MasterIsTemplate);
				}
				catch(Exception ex)
				{
					m_xCurrentMenuDocument = null;
					SetError(ex.Message);
					return false;
				}
			}
			else
			{
				m_xCurrentMenuDocument = new XmlDocument();
// LOOKY
				//m_xCurrentMenuDocument.AppendChild(m_xCurrentMenuDocument.ImportNode(xCurrentMenu.SelectSingleNode("container"), true));
				m_xCurrentMenuDocument.AppendChild(m_xCurrentMenuDocument.ImportNode(xCurrentMenu, true));
			}

			Control baseContainer = null;
			try
			{
				baseContainer = this.rootMenuContainer;
			}
			catch
			{
				MessageBox.Show("What?");
			}

			List<XmlElement> ar = new List<XmlElement>();
			ar.Add(m_xCurrentMenuDocument.SelectSingleNode("/container") as XmlElement);
			UpdateMenuContainers(ar, baseContainer as Container_Base);

			MenuValueChanged(null); // If the template changes the menu, the owner app. gets to know about it.

			if(rootMenuContainer.Controls.Count != 0)
			{
				Container_Base cb = rootMenuContainer.Controls[0] as Container_Base;
				cb.LayoutChildren();
			}
			return true;
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

		private Container_Base UpdateMenuContainers(List<XmlElement> xContainerList, Container_Base containerParent)
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

								Container_Base container = FindAndTagMatchingControl(containerParent, xContainer); // Matches name and type. Only considers unclaimed controls and claims them if a match is found.

								if(container == null)
								{
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

									container.m_xObject = xContainer; // Do this before setting the value, so the treeView can work.

									string strValue = XmlHelper.GetKeyValue(xContainer, "value");
									if(strValue == null)
										strValue = XmlHelper.GetKeyValue(xContainer, "default");
									if(strValue == null)
										strValue = UNINITIALISED_MARKER;

									string strDisplayName = XmlHelper.GetKeyValue(xContainer, "display_name");

									#region Set container name and value.

									if(strValue != "ignore me")
									{
										if(container.ObjectValue != strValue)		// RED ALERT this fixes the dirty flag being erroneously set but may break other things.
											container.ObjectValue = strValue;
									}

									if(strDisplayName != "ignore me")
									{
										container.ObjectName = strDisplayName;
									}

									#endregion
								}
								UpdateMenuContainers(container.xChildren, container);
							}
						}


					if(containerParent.ChildContainer != null)
					{
						List<Container_Base> removeList = new List<Container_Base>(containerParent.Controls.Count);
						foreach(Container_Base cb in containerParent.ChildContainer.Controls)
						{
							if((bool) cb.Tag == false)
							{
								removeList.Add(cb);
							}
						}
						foreach(Container_Base cb in removeList) /// Some will have been removed already by FindMatchingControl.
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
				foreach(Container_Base cb in containerParent.ChildContainer.Controls)
				{
					if((bool) cb.Tag == false) // Only consider unclaimed controls.
					{
						if(cb.Name == strName)
						{
							if(cb.strType == strType)
							{
								retVal = cb;
								cb.Tag = true; // Mark this control as in use.
								break;
							}
							else
							{
								retVal = null;
								break;
							}
						}
						containerParent.ChildContainer.Controls.Remove(cb); // Remove unclaimed any controls that have been skipped over. This ensures we maintain order,
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

		public bool SetSingleContainer(string strName, string strValue)
		{
			Container_Base cb = FindMatchingControlRecursively(this.rootMenuContainer, strName, null);

			if(cb != null)
			{
				if(strValue != null)
				{
					cb.ObjectValue = strValue;
					return true;
				}
			}
			return false;
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

		private static Container_Base FindMatchingControlRecursively(Control control, string strName, string strType)
		{
			if(control != null)
			{
				foreach(Control childControl in control.Controls)
				{
					if(childControl is Container_Base)
					{
						Container_Base cb = childControl as Container_Base;
						if(cb.Name == strName)
						{
// ATTENTION WTF???
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
		/// This function creates a new Xml menu by merging a master and slave XmlElement hierarchy.
		/// If xMasterMenu is a template then the resulting menu looks like xMasterMenu and 'value' keys are merged where possible,
		/// otherwise the menu looks like a full merge of the two and 'value' keys WILL be marked with CONFLICTED_MARKER if neither is null
		/// and they have different values.
		/// 
		/// Essentially it's a merge between the two elements passed in and when xMasterMenu is a template, it defines the
		/// contents of the result and xSlaveMenu defines the values of the contents.
		/// </summary>
		/// <param name="xMasterMenu">The primary menu for the merge.</param>
		/// <param name="xSlaveMenu">The secondary menu for the merge. Note that it can be null.</param>
		/// <returns>A new Xml menu document.</returns>
		public static XmlDocument MergeMenuDocuments(XmlElement xMasterMenu, XmlElement xSlaveMenu, MergeMode mergeMode)
		{
			// TODO: This is cack because the return value is a document. It should be an xmlelement?

			XmlDocument xNewMenu = new XmlDocument();
			XmlNode nodeDest = xNewMenu.ImportNode(xMasterMenu, true);
			xNewMenu.AppendChild(nodeDest);
			_MergeMenuDocuments(xNewMenu["container"], xSlaveMenu, mergeMode);

			return xNewMenu;
		}


		/// <summary>
		/// See 'MergeMenuDocuments'
		/// </summary>
		private static bool _MergeMenuDocuments(XmlElement xMasterElement, XmlElement xSlaveElement, MergeMode mergeMode)
		{
			// Merge these two elements.
			Debug.Assert(xMasterElement != null);

			if(xSlaveElement != null)
			{
				if(XmlHelper.GetKeyValue(xMasterElement, "type") == XmlHelper.GetKeyValue(xSlaveElement, "type") && // If the 'types' are the same ...
				   XmlHelper.GetKeyValue(xMasterElement, "name") == XmlHelper.GetKeyValue(xSlaveElement, "name"))
				{
					string strValue1 = XmlHelper.GetKeyValue(xMasterElement, "value");
					string strValue2 = XmlHelper.GetKeyValue(xSlaveElement, "value");

					if(strValue1 == null) strValue1 = strValue2;
					if(strValue2 == null) strValue2 = strValue1;

					string strNewValue;

					if(strValue1 == null)
					{
						// They are both null.
						strNewValue = null;
					}
					else if(strValue1 == strValue2)
						// Neither of them are null. They are the same.
						strNewValue = strValue1;
					else
					{
						// Neither of them are null. They are different. TODO: Pick 'master' value if Master is template?
						strNewValue = CONFLICTED_MARKER;
					}

					if(strNewValue != null) // Don't add a value when there isn't one to base it on.
						XmlHelper.AddAttribute(xMasterElement, "value", strNewValue);

					// TODO: Do the same as above for the 'default' attribute.
				}
				else // There is a merge conflict.
				{
					if(mergeMode == MergeMode.MasterIsTemplate)
					{
						// Do nothing. Keep the master element.
					}
					else
					{
						string strName = XmlHelper.GetKeyValue(xMasterElement, "name");
						xMasterElement.RemoveAll(); // We're not interested in the children of a conflicted merge container.
						XmlHelper.AddAttribute(xMasterElement, "name", strName);
						XmlHelper.AddAttribute(xMasterElement, "type", "merge_conflict");
					}
				}
			}
			else // Slave element is null.
			{
				switch(mergeMode)
				{
					case MergeMode.MasterIsTemplate:
						// Do nothing. Keep the master element.
						break;
					case MergeMode.MustExistInBoth:
						// Remove the element from the master.
						Debug.Assert(xMasterElement.ParentNode.NodeType == XmlNodeType.Element || xMasterElement.ParentNode.NodeType == XmlNodeType.Document);
						xMasterElement.ParentNode.RemoveChild(xMasterElement);
						break;
					case MergeMode.MayExistInEither:
						// Do nothing. Keep the master element.
						break;
					default:
						break;
				}
			}

			// The merge of the two passed elements is done. Now deal with the children.

			/// Build a double-key dictionary. The 'values' of the dictionary represent a list of the 'name' attribute value of all
			/// container elements in both the master and slave documents.
			/// key1 represents the master xml element associated with the name, if one exists.
			/// key2 represents the slave xml  element associated with the name, if one exists.

			SuperCollections.DoubleKeyDictionary<XmlElement, XmlElement, string> d = new SuperCollections.DoubleKeyDictionary<XmlElement, XmlElement, string>();


			// Put all of the master children into our double key dictionary.

			XmlNodeList xMasterChildren = xMasterElement.SelectNodes("container");

			foreach(XmlElement xMasterChild in xMasterChildren)
			{
				string strChildName = XmlHelper.GetKeyValue(xMasterChild, "name");
				d.AddKey1(xMasterChild, strChildName);
			}

			// Put all of the slave children into our double key dictionary.
			if(xSlaveElement != null)
			{
				XmlNodeList xSlaveChildren = xSlaveElement.SelectNodes("container");

				foreach(XmlElement xSlaveChild in xSlaveChildren)
				{
					string strChildName = XmlHelper.GetKeyValue(xSlaveChild, "name");
					d.AddKey2(xSlaveChild, strChildName);
				}
			}

			//foreach(string strContainerName in d._valuesToKeysLookup.Keys)
			foreach(string strContainerName in d.Values)
			{
				SuperCollections.DoubleKeyDictionary<XmlElement, XmlElement, string>.DictionaryKeys keys;

				keys = d.GetKeysFromValue(strContainerName);

				Debug.Assert(keys != null);

				switch(mergeMode)
				{
					case MergeMode.MasterIsTemplate: // (Must exist in master)
						if(keys._key1 != null) // Resulting document fragment must match the master fragment. Slave may be null.
						{
							_MergeMenuDocuments(keys._key1, keys._key2, mergeMode);
						}
						break;
					case MergeMode.MustExistInBoth:

						if((keys._key1 != null) && (keys._key2 != null)) // Resulting document fragment must match the master and slave fragment.
						{
							_MergeMenuDocuments(keys._key1, keys._key2, mergeMode);
						}
						else if(keys._key1 != null)
						{
							// It is in the master but not the slave. Remove it.
							Debug.Assert((keys._key1.NodeType == XmlNodeType.Element) || (keys._key1.NodeType == XmlNodeType.Document));
							keys._key1.ParentNode.RemoveChild(keys._key1);
						}
						break;
					case MergeMode.MayExistInEither:
						if(keys._key1 != null) // Resulting document fragment must match the master fragment if it is not null.
						{
							_MergeMenuDocuments(keys._key1, keys._key2, mergeMode);
						}
						else if(keys._key2 != null)
						{
							// Master is null, slave is not null.
							// slave must be copied into master.
							// slave becomes null.
							// Master becomes template.

							// keys._key2 is the slave child element.

							xMasterElement.AppendChild(xMasterElement.OwnerDocument.ImportNode(keys._key2, true));

							_MergeMenuDocuments(keys._key2, null, MergeMode.MasterIsTemplate);
						}
						break;
					default:
						break;
				}
			}

			return true;
		}


		//////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////

		/// <summary>
		/// Creates a menu container (a user control) from an Xml container definition, and calls it's PostConstructor.
		/// </summary>
		/// <param name="xContainer">Xml container definition.</param>
		/// <param name="bCreateChildren">Have the container recursively create it's children?</param>
		/// <returns></returns>
		internal Container_Base CreateMenuContainer(Container_Base parent, XmlElement xContainer, bool bCreateChildren)
		{
			m_ignoreChangedEventHandler++; // Disable events on the containers for the duration of it's creation.

			Container_Base menuContainer = null;

			string strContainerType = XmlHelper.GetKeyValue(xContainer, "type");
			switch(strContainerType)
			{
				case "action_button": // container_action_button.cs
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

				case "dropdown": // container_dropdown.cs
					{
						menuContainer = new Container_Dropdown(this, xContainer, bCreateChildren);
					}
					break;

				case "enum":
					{
						menuContainer = new Container_Enum(this, xContainer, bCreateChildren);
					}
					break;

				case "filename": // container_filename.cs
					{
						menuContainer = new Container_Filename(this, xContainer, bCreateChildren);
					}
					break;
				case "folder": // container_filename.cs
					{
						menuContainer = new Container_FolderBrowser(this, xContainer, bCreateChildren);
					}
					break;

				case "network_filename": // Depracate?
					{
						menuContainer = new Container_Filename(this, xContainer, bCreateChildren);
					}
					break;

				case "x360name": // container_get_default_360_console.cs
					{
						menuContainer = new Container_GetDefault360(this, xContainer, bCreateChildren);
					}
					break;

				case "label":
					{
						menuContainer = new Container_Label(this, xContainer, bCreateChildren);
					}
					break;

				case "int": // container_number.cs
					{
						menuContainer = new Container_Number(this, xContainer, bCreateChildren);
					}
					break;

				case "float": // container_number.cs
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
				case "xml_value":
					{
						menuContainer = new Container_GetXmlData(this, xContainer, bCreateChildren);
					}
					break;

				case "custom_animation":
					{
						menuContainer = new Container_CustomAnimation(this, xContainer, bCreateChildren);
					}
					break;

				default:
					{
						menuContainer = new Container_Unknown(this, xContainer, bCreateChildren); // Rock on!
					}
					break;
			}

			menuContainer.CreateControl(); // This causes the 'Load' event to fire. (I think).
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

		/// <summary>
		/// Some controls have dependencies, i.e. their 'readonly' state depends on the value of another control.
		/// This function recursively sets the 'readonly' state of all containers if they are dependent on
		/// other containers.
		/// </summary>
		internal void ProcessDependencies()
		{
			if(rootMenuContainer.Controls.Count > 0)
			{
				Container_Base cb = rootMenuContainer.Controls[0] as Container_Base;
				//cb.SuspendLayout();
				cb.ProcessDependencies();
				//cb.ResumeLayout(true);
			}
		}

		public string ContainerExpand(string strRawCommand, bool bSurroundExpansionWithQuotes)
		{
			MacroExpander.MacroExpander mx = new MacroExpander.MacroExpander();
			XmlElement xmlElement = this.m_xCurrentMenuDocument["container"];

			XmlNodeList xContainerList = xmlElement.SelectNodes("//container");

			if(xContainerList != null)
			{
				foreach(XmlElement xContainer in xContainerList)
				{
					if(bSurroundExpansionWithQuotes == true)
						mx.AddPair("%" + XmlHelper.GetKeyValue(xContainer, "name") + "%", "\"" + XmlHelper.GetKeyValue(xContainer, "value") + "\"");
					else
						mx.AddPair("%" + XmlHelper.GetKeyValue(xContainer, "name") + "%", XmlHelper.GetKeyValue(xContainer, "value"));
				}
			}
			string strResult = mx.Expand(strRawCommand, true);

			return strResult;
		}
	}


	public class MenuSystemChangedEventArgs : EventArgs
	{
		public MenuSystemChangedEventArgs(XmlDocument xOwnerDocument, XmlElement xChangedElement, bool bFlagDocumentAsChanged)
		{
			XOwnerDocument = xOwnerDocument;
			XChangedElement = xChangedElement;
			FlagDocumentAsChanged = bFlagDocumentAsChanged;
		}

		public XmlDocument XOwnerDocument { get; private set; }

		public XmlElement XChangedElement { get; private set; }

		public bool FlagDocumentAsChanged { get; private set; }
	}

	public class MenuComponentCreatedEventArgs : EventArgs
	{
		public MenuComponentCreatedEventArgs(Container_Base createdComponent)
		{
			_createdComponent = createdComponent;
		}

		public Container_Base _createdComponent { get; private set; }
	}

}

#if false


action_button
basic
bitmap
bool
colour
dropdown
enum
filename
folder
network_filename
x360name
label
int
float
progress_bar
radio_button
string
text
tree
webpage
custom_list

#endif