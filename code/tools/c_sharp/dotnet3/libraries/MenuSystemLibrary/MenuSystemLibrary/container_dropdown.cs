using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using Global;
using System.Collections;

namespace MenuSystemLibrary
{
	public partial class Container_Dropdown : Container_Base
	{
		string strOldSelectedItem = "";

		public Container_Dropdown(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			string strSelectedItem = null;
			string strValue = XmlHelper.GetKeyValue(xContainer, "value");

			// Add every child of type 'container' to this combobox.
			foreach(XmlNode n in xContainer.ChildNodes)
			{
				if(n.Name == "container")
				{
					string strName = XmlHelper.GetKeyValue(n, "name");
					this.comboBox.Items.Add(strName);

					if (strSelectedItem == null)
						strSelectedItem = strOldSelectedItem = strName;

					if(strName == strValue)
					{
						strSelectedItem = strOldSelectedItem = strValue;
					}
				}
			}

			// The 'value' of this container may have changed due to the validation above,
			// so it must be written back to the xml data for this container.
			XmlHelper.AddAttribute(m_xObject, "value", strSelectedItem);
		}

		internal override string ObjectName
		{
//			get { return this.groupBox.Text; }
			set { this.groupBox.Text = value; }
		}


		internal override string ObjectValue 
		{
			get{return this.comboBox.SelectedItem.ToString();}
			set { this.comboBox.SelectedItem = value; }
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.comboBox;
			}
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			// Do nothing. TODO: Is this how we want it to behave?
		}

		internal override Control ChildContainer {get{return groupChildren;}}

		internal override List<XmlElement> xChildren
		{
			get
			{
			    XmlNodeList xAllChildren = m_xObject.ChildNodes;

			    List<XmlElement> xRelevantChildren = new List<XmlElement>();

			    foreach (XmlNode xChild in xAllChildren)
			    {
			        if (xChild.Name == "container")
			        {
			            if (this.comboBox.SelectedItem != null)
			            {
			                if (XmlHelper.GetKeyValue(xChild, "name") == this.comboBox.SelectedItem.ToString())
			                {
			                    xRelevantChildren.Add(xChild as XmlElement);
			                }
			            }
			        }
			        else
			        {
			            xRelevantChildren.Add(xChild as XmlElement);
			        }
			    }
			    return xRelevantChildren;
			}			
	
		}

		private void comboBox_SelectedIndexChanged(object sender, EventArgs e)
		{
			ComboBox cd = sender as ComboBox;

			// Only refresh children if the selection has actually changed.
//			if (cd.SelectedItem.ToString() != strOldSelectedItem)
				ValueChanged("value", this.comboBox.SelectedItem.ToString(), true);

			strOldSelectedItem = cd.SelectedItem.ToString();
		}
	}
}

