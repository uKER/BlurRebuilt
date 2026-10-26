using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace MenuSystemLibrary
{
	public partial class Container_Webpage : MenuSystemLibrary.Container_Base
	{
		private Container_Webpage()
		{
			InitializeComponent();
		}

		public Container_Webpage(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			//			string strValue = XmlHelper.GetKeyValue(m_xObject, "value");
			//
			//			this.toolTip.SetToolTip(this.textBox, "String:\n\"" + strValue + "\"");
		}

		override internal string ObjectName
		{
			//			get { return this.checkBox.Text; }
			set { return; }
		}

		override internal string ObjectValue
		{
			get { return this.webBrowser.Url.AbsolutePath; }
			set
			{
				try { this.webBrowser.Navigate(new Uri(value)); }
				catch { }
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.webBrowser;
			}
		}

//
//		private void checkBox_CheckedChanged(object sender, EventArgs e)
//		{
//			ValueChanged("value", this.checkBox.Checked.ToString().ToLower(), false);
//		}
	}
}

