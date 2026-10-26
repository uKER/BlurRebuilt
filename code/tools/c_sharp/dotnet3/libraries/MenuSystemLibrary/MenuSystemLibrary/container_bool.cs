using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Diagnostics;

namespace MenuSystemLibrary
{
	public partial class Container_Bool : MenuSystemLibrary.Container_Base
	{
		private Container_Bool()
		{
			InitializeComponent();
		}

		public Container_Bool(MenuSystem owner, XmlElement xContainer, bool bCreateChildren) : base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();
//			string strValue = XmlHelper.GetKeyValue(m_xObject, "value");
//
//			this.toolTip.SetToolTip(this.textBox, "String:\n\"" + strValue + "\"");
		}

		override internal string ObjectName
		{
//			get { return this.checkBox.Text; }
			set { this.checkBox.Text = value; }
		}

		override internal string ObjectValue
		{
			get { return this.checkBox.Checked.ToString().ToLower(); }
			set
			{
				if(value == MenuSystem.CONFLICTED_MARKER)
				{
					//this.checkBox.ThreeState = true;
					this.checkBox.CheckState = CheckState.Indeterminate;
				}
				else
				{
					string strVal = (value as string).ToLower();

					if(strVal == "true" || strVal == "yes" || strVal == "1" || strVal == "ok")
						this.checkBox.Checked = true;
					else
						this.checkBox.Checked = false;
				}
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.checkBox;
			}
		}


		private void checkBox_CheckStateChanged(object sender, EventArgs e)
		{
			switch(this.checkBox.CheckState)
			{
				case CheckState.Indeterminate:
					ValueChanged("value", MenuSystem.CONFLICTED_MARKER, false);
					break;
				default:
					ValueChanged("value", this.checkBox.Checked.ToString().ToLower(), false);
					break;
			}

		}
	}
}

