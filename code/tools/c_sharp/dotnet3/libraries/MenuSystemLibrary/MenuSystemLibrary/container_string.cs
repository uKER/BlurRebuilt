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
	public partial class Container_String : MenuSystemLibrary.Container_Base
	{
		public Container_String()
		{
			InitializeComponent();
		}

		public Container_String(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();


		}

		private void Container_String_Load(object sender, EventArgs e)
		{
			string strValue = XmlHelper.GetKeyValue(m_xObject, "value");

			this.toolTip.SetToolTip(this.textBox, "String:\n\"" + strValue + "\"");

			string strStyle = XmlHelper.GetKeyValue(m_xObject, "style");

			if(strStyle == "sparse")
			{
				this.Height = 24;
				this.textBox.Location = new Point(3, 3);
				this.label.Visible = false;
			}
		}

		override internal string ObjectName
		{
//			get { return this.label.Text; }
			set { this.label.Text = value; }
		}

		override internal string ObjectValue
		{
			get { return this.textBox.Text; }
			set { this.textBox.Text = value; }
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.textBox;
			}
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			this.textBox.ReadOnly = bReadonly;
			if(bReadonly == true)
				this.textBox.BackColor = SystemColors.Control;
			else
				this.textBox.BackColor = SystemColors.Window;
		}


		private void textBox_TextChanged(object sender, EventArgs e)
		{
			ValueChanged("value", this.textBox.Text, false);
			this.toolTip.SetToolTip(this.textBox, "String:\n\"" + ObjectValue + "\"");
			this.toolTip.ShowAlways = true;
		}


	}
}

