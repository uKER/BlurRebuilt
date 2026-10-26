using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using Global;

namespace MenuSystemLibrary
{
	public partial class Container_CustomAnimation : MenuSystemLibrary.Container_Base
	{
		public Container_CustomAnimation()
		{
			InitializeComponent();
		}

		private XmlDocument _xLookup;
		private XmlElement _xCycleFilename;
		private XmlElement _xOutputLocation;
		private XmlElement _xOutputDirectory;

		private string _strErrorString = null;

		private Dictionary<string, string> _customLookup;

		public Container_CustomAnimation(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			string strXmlLookup = XmlHelper.GetKeyValue(m_xObject, "xml_lookup");

			if(!string.IsNullOrEmpty(strXmlLookup))
			{
				try
				{
					_xLookup = new XmlDocument();
					_xLookup.Load(strXmlLookup);

					XmlNodeList xKeys = _xLookup.SelectNodes("keys/key");

					_customLookup = new Dictionary<string, string>(xKeys.Count);

					foreach(XmlElement xKey in xKeys)
					{
						if(xKey != null)
						{
							string strName = XmlHelper.GetKeyValue(xKey, "name");
							string strValue = XmlHelper.GetKeyValue(xKey, "value");

							if(!string.IsNullOrEmpty(strName) && !string.IsNullOrEmpty(strValue))
							{
								_customLookup.Add(strName, strValue);
							}
						}
					}
				}
				catch(Exception ex)
				{
					_strErrorString = "Cannot load the Xml lookup document. The system reports:\n'"+ex.Message+"'";
				}

				if(_strErrorString == null)
				{
					string strCycleFilename = XmlHelper.GetKeyValue(m_xObject, "cycle_filename");

					if(!string.IsNullOrEmpty(strCycleFilename))
					{
						_xCycleFilename = XmlHelper.GetNodeFromKeyPath(m_xObject, "name", strCycleFilename) as XmlElement;
						XmlHelper.AddAttribute(_xCycleFilename, "is_target_of_custom_animation", "true");

						string strOutputLocation = XmlHelper.GetKeyValue(m_xObject, "output_location");

						if(!string.IsNullOrEmpty(strOutputLocation))
						{
							_xOutputLocation = XmlHelper.GetNodeFromKeyPath(m_xObject, "name", strOutputLocation) as XmlElement;
							XmlHelper.AddAttribute(_xOutputLocation, "is_target_of_custom_animation", "true");

							string strOutputDirectory = XmlHelper.GetKeyValue(m_xObject, "output_directory");

							if(!string.IsNullOrEmpty(strOutputDirectory))
							{
								_xOutputDirectory = XmlHelper.GetNodeFromKeyPath(m_xObject, "name", strOutputDirectory) as XmlElement;
								XmlHelper.AddAttribute(_xOutputDirectory, "is_target_of_custom_animation", "true");


								// At this point, all the information needed from the xml container has been successfully gathered.
								// the rest of the work is done in the owner of the menu. ???
								//m_xObject.OwnerDocument.NodeChanged += OwnerDocument_NodeChanged;


							}
							else
							{
								_strErrorString = "Attribute 'output_directory' not found on Container_CustomAnimation";
							}
						}
						else
						{
							_strErrorString = "Attribute 'output_location' not found on Container_CustomAnimation";
						}
					}
					else
					{
						_strErrorString = "Attribute 'cycle_filename' not found on Container_CustomAnimation";
					}
				}
			}


			/*
			 * TODO:
			 * Get xml lookup file.
			 * Put an event 'watch' on the external input field. UIse Xml events on the document?
			 * Hack in to the 'output directory external field so values can be set.
			 * 
			 * Toggle Output directory colour depending on validity of lookup.
			 */

		}

		void OwnerDocument_NodeChanged(object sender, XmlNodeChangedEventArgs e)
		{
			throw new NotImplementedException();
		}

		bool bLoadEventHasOccured = false;

		private void Container_CustomAnimation_Load(object sender, EventArgs e)
		{
			if(bLoadEventHasOccured == false)
			{
				string strValue = XmlHelper.GetKeyValue(m_xObject, "value");

				bLoadEventHasOccured = true;
			}
		}

		override internal string ObjectName
		{
			//			get { return this.checkBox.Text; }
			set { this.checkBoxDirectoryFromNamingConvention.Text = value; }
		}

		override internal string ObjectValue
		{
			get { return this.checkBoxDirectoryFromNamingConvention.Checked.ToString().ToLower(); }
			set
			{
				if(value == MenuSystem.CONFLICTED_MARKER)
				{
					//this.checkBox.ThreeState = true;
					this.checkBoxDirectoryFromNamingConvention.CheckState = CheckState.Indeterminate;
				}
				else
				{
					if(value != null)
					{
						string strVal = (value as string).ToLower();

						bool bCheckVal = this.checkBoxDirectoryFromNamingConvention.Checked;

						if(strVal == "true" || strVal == "yes" || strVal == "1" || strVal == "ok")
							this.checkBoxDirectoryFromNamingConvention.Checked = true;
						else
							this.checkBoxDirectoryFromNamingConvention.Checked = false;

						// Hack? Setting a checkbox to it's current value doesn't initiate an OnChanged event so we have to fire one ourselves.
						// DO NOT do this. It implies a change has been made so applications using the library may mark their data as dirty.
						//if(bCheckVal == this.checkBox.Checked)
						//    checkBox_CheckStateChanged(this.checkBox, new EventArgs());
					}
					else
					{
						// ATTENTION investigate why we get here. Is it when the template adds a new 
						// checkbox and we load it for the first time or is it something sinister?
						bool bCheckVal = this.checkBoxDirectoryFromNamingConvention.Checked;
						this.checkBoxDirectoryFromNamingConvention.Checked = false;
						// Hack? Setting a checkbox to it's current value doesn't initiate an OnChanged event so we have to fire one ourselves.
						if(bCheckVal == this.checkBoxDirectoryFromNamingConvention.Checked)
							checkBoxDirectoryFromNamingConvention_CheckStateChanged(this.checkBoxDirectoryFromNamingConvention, new EventArgs());
					}
				}
			}
		}


	

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this;
			}
		}

		private void checkBoxDirectoryFromNamingConvention_CheckStateChanged(object sender, EventArgs e)
		{
			switch(this.checkBoxDirectoryFromNamingConvention.CheckState)
			{
				case CheckState.Indeterminate:
					ValueChanged("value", MenuSystem.CONFLICTED_MARKER, false);
					break;
				default:
					ValueChanged("value", this.checkBoxDirectoryFromNamingConvention.Checked.ToString().ToLower(), false);
					break;
			}
		}
	}
}

