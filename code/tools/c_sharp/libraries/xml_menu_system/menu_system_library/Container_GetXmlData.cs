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
	public partial class Container_GetXmlData : MenuSystemLibrary.Container_Base
	{
		private Container_GetXmlData()
		{
			InitializeComponent();
		}

		public Container_GetXmlData(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			string strDataPath = XmlHelper.GetKeyValue(m_xObject, "default");

			if(strDataPath != null)
			{
				string[] parts = strDataPath.Split(new char[] { '|' }, StringSplitOptions.RemoveEmptyEntries);

				if(parts.Length == 3)
				{
					try
					{
						string strHeading = parts[0];
						string strFilename = parts[1];
						string strxPath = parts[2];

						this.groupBox1.Text = strHeading;

						XmlDocument docDataDocument = new XmlDocument();
						docDataDocument.Load(strFilename);

						XmlNode xNode = docDataDocument.SelectSingleNode(strxPath);
						XmlHelper.AddAttribute(m_xObject, "value", xNode.Value);
					}
					catch
					{
						XmlHelper.AddAttribute(m_xObject, "value", "Cannot parse XmlDataContainer.");
					}
				}
			}
		}

		override internal string ObjectName
		{
			set { /*this.label1.Text = value;*/ }
		}

		override internal string ObjectValue
		{
			get { return this.label1.Text; }
			set {
				if(value == MenuSystem.CONFLICTED_MARKER)
				{
					CreateConflictedButton("CONFLICTED", this.panel1);
				} 
				else
					this.label1.Text = value;
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.label1;
			}
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			// Do nothing.
		}
	}
}