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
	public partial class Container_RadioButton : MenuSystemLibrary.Container_Base
	{
		private Container_RadioButton()
		{
			InitializeComponent();
		}

		internal Container_Base m_rbGroupOwner;

		public Container_RadioButton(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();
		}


		override internal string ObjectName
		{
			set { this.radioButton.Text = value; }
		}

		override internal string ObjectValue
		{
			get { return this.radioButton.Checked.ToString().ToLower(); }
			set
			{
				if(value == MenuSystem.CONFLICTED_MARKER)
				{
					CreateConflictedButton("CONFLICTED", this.radioButton);
				}
				else
				{
					if(value != null)
					{
						string strVal = (value as string).ToLower();

						if(strVal == "true" || strVal == "yes" || strVal == "1" || strVal == "ok")
						{
							this.radioButton.Checked = true;
							return;
						}
					}
					this.radioButton.Checked = false;
				}
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.radioButton;
			}
		}

		// Note that changing a radio button to 'on' (the only valid user action) has to switch off another radio button,
		// which in turn generates a callback to here.

	private void radioButton_CheckedChanged(object sender, EventArgs e)
		{
			Debug.Assert(sender as RadioButton == this.radioButton);

			Container_Base parent = this.m_rbGroupOwner;

			Debug.Assert(parent.m_containerRadioButtons != null);

			if(parent.m_containerRadioButtons != null)
			{
				if(this.radioButton.Checked == true)
				{
					foreach(Container_RadioButton crb in parent.m_containerRadioButtons)
					{
						this.m_owner.m_ignoreChangedEventHandler++;
						if(crb != this)
						{
							if(crb.radioButton.Checked == true)
								crb.ObjectValue = "false";
						}
						this.m_owner.m_ignoreChangedEventHandler--;
					}
				}
			}
			ValueChanged("value", this.radioButton.Checked.ToString().ToLower(), false);
		}
	}
}

