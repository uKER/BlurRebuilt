using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Global;
using System.Xml;
using System.Collections;

namespace MenuSystemLibrary
{
	
	public partial class Container_GetDefault360 : MenuSystemLibrary.Container_Base
	{

		//static XDevkit.XboxManagerClass xbmc;

		private Container_GetDefault360()
		{
			InitializeComponent();
		}

		public Container_GetDefault360(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			//try
			//{
			//    if(xbmc == null)
			//        xbmc = new XboxManagerClass();
			//    if(xbmc != null)
			//    {
			//                            XmlHelper.AddAttribute(m_xObject, "default", ((XboxManagerClass)xbmc).DefaultConsole);
			//                            XmlHelper.AddAttribute(m_xObject, "value", ((XboxManagerClass)xbmc).DefaultConsole);
			//        //					ValueChanged("value", ((XboxManagerClass)xbmc).DefaultConsole, false);

			//        this.textBox_360Name.Text = ((XboxManagerClass)xbmc).DefaultConsole;

			//    }
			//}
			//catch
			//{
			//    XmlHelper.AddAttribute(m_xObject, "value", MenuSystemLibrary.MenuSystem.UNINITIALISED_MARKER);
			//    this.textBox_360Name.Text = MenuSystemLibrary.MenuSystem.UNINITIALISED_MARKER;
			//}

		}

//		protected override Control ChildContainer { get { return null; } }
//		protected override ArrayList Children { get { return null; } }
		//////

		override internal string ObjectName
		{
//			get { return this.textBox_360Name.Text; }
			set { this.textBox_360Name.Name = value; }
		}

		override internal string ObjectValue
		{
			get { return this.textBox_360Name.Text; }
			set
			{
				this.textBox_360Name.Text = value; 
				textBox_360Name.Select(textBox_360Name.Text.Length, 0);
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.textBox_360Name;
			}
		}

		private void textBox_360Name_TextChanged(object sender, EventArgs e)
		{
			ValueChanged("value", this.textBox_360Name.Text, false);
//			richTextBox.Select(richTextBox.Text.Length, 0);

		}

	}
}

