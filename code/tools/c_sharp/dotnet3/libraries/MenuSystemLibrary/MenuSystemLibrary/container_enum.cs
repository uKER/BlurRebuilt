using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Collections;
using Global;
using System.Diagnostics;

namespace MenuSystemLibrary
{
	public partial class Container_Enum : MenuSystemLibrary.Container_Base
	{
		public Container_Enum()
		{
			InitializeComponent();
		}

		public Container_Enum(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			string strValues = XmlHelper.GetKeyValue(xContainer, "values");
			if (strValues != null)
			{
				List<string> strEntries;
				if (owner.m_enumTable.TryGetValue(strValues, out strEntries) == true)
				{

					foreach (string strEntry in strEntries)
						this.comboBox.Items.Add(strEntry);
				}
				else
				{
					this.comboBox.Items.Add("***INVALID***");

				}
			}
			else
			{
				this.comboBox.Items.Add("***INVALID***");
			}
			string strValue = XmlHelper.GetKeyValue(xContainer, "value");

//			this.comboBox.SelectedItem = strValue;
//			
//			if(this.comboBox.SelectedIndex == -1)
//				this.comboBox.SelectedIndex = 0;

			ObjectValue = strValue;

			if(ObjectValue == null)
				this.ObjectValue = this.comboBox.Items[0].ToString();

//			XmlHelper.AddAttribute(m_xObject, "value", strSelectedItem);
		}

		internal override Control ChildContainer { get { return null; } }

		internal override string ObjectName
		{
//			get { return this.label.Text; }
			set { this.label.Text = value; }
		}

		
		internal override string ObjectValue
		{
			get{return this.comboBox.SelectedItem == null ? null : this.comboBox.SelectedItem.ToString();}
			set
			{
				if(value == MenuSystem.CONFLICTED_MARKER)
				{
					CreateConflictedButton("CONFLICTED", this.comboBox.Left, this.comboBox.Top, this.comboBox.Width, this.comboBox.Height);
				}
				else
				{
					this.comboBox.SelectedItem = value;
				}
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.comboBox;
			}
		}

		private void comboBox_SelectedIndexChanged(object sender, EventArgs e)
		{
			//if(this.comboBox.Items.Contains(MenuSystem.CONFLICTED_MARKER) == true)
			//{
			//    if(this.comboBox.SelectedIndex == 0)
			//        return;
			//    this.comboBox.Items.RemoveAt(0);

			//}
			
			ValueChanged("value", this.comboBox.SelectedItem.ToString(), false);

		}
	}

}

