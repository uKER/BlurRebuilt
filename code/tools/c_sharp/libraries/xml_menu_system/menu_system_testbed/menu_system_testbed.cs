using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using MenuSystem;
using MenuSystemLibrary;

namespace MenuSystemTestbed
{
    public partial class MenuWrapper : Form
    {
        public MenuWrapper()
        {
            InitializeComponent();

//			this.menuSystemLibrary.Initialise(@"MenuDefinition_test.xml", GetMenu);

			XmlDocument menuDefinitionDocument = new System.Xml.XmlDocument();

			try
			{
				menuDefinitionDocument.Load(@"MenuDefinition_test.xml");
			}
			catch(Exception ex)
			{
				menuDefinitionDocument = null;
				this.menuSystemLibrary.SetError("Error loading the XML definition document.\nError is:\n" + ex.Message);
			}

			this.menuSystemLibrary.Initialise(menuDefinitionDocument["menu_system"], GetMenu);

//			this.menuSystemLibrary.Initialise(@"..\..\XmlFile1.xml", GetMenu);
//			this.menuSystemLibrary.SetMenu(new XmlDocument());
        }

//		void GetMenu(XmlDocument xMenu)
		void GetMenu(XmlElement xMenu, Container_Base cChangedContainer, bool bFlagDocumentAsChanged)
		{
			this.menuStringTextBox.Text = xMenu.OuterXml;
		}

		private void setString_Click(object sender, EventArgs e)
		{
			XmlDocument xMenu = this.menuSystemLibrary.CreateXmlDocumentFromString("<values>"+this.menuStringTextBox.Text+"</values>");
			if(xMenu != null)
				this.menuSystemLibrary.SetMenu(xMenu.FirstChild as XmlElement, true);
		}
    }
}