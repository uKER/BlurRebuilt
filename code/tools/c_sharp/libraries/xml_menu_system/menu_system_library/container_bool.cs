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
					if(value != null)
					{
						string strVal = (value as string).ToLower();

						bool bCheckVal = this.checkBox.Checked;

						if(strVal == "true" || strVal == "yes" || strVal == "1" || strVal == "ok")
							this.checkBox.Checked = true;
						else
							this.checkBox.Checked = false;

						// Hack? Setting a checkbox to it's current value doesn't initiate an OnChanged event so we have to fire one ourselves.
						// DO NOT do this. It implies a change has been made so applications using the library may mark their data as dirty.
						//if(bCheckVal == this.checkBox.Checked)
						//    checkBox_CheckStateChanged(this.checkBox, new EventArgs());
					}
					else
					{
						// ATTENTION investigate why we get here. Is it when the template adds a new 
						// checkbox and we load it for the first time or is it something sinister?
						bool bCheckVal = this.checkBox.Checked;
						this.checkBox.Checked = false;
						// Hack? Setting a checkbox to it's current value doesn't initiate an OnChanged event so we have to fire one ourselves.
						if(bCheckVal == this.checkBox.Checked)
							checkBox_CheckStateChanged(this.checkBox, new EventArgs());
					}
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

