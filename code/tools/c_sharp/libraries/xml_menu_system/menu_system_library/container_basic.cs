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

namespace MenuSystemLibrary
{
	public partial class Container_Basic : MenuSystemLibrary.Container_Base
	{
		//private Container_Basic() : base()
		//{
		//    InitializeComponent();
		//}

		public Container_Basic(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();
			this.button1.Visible = false;
		}

		internal override string ObjectName
		{
//			get { return this.groupBox.Text; }
			set { this.groupBox.Text = value; }
		}

		internal override string ObjectValue { get { return "*** NO VALUE ***"; } set { } }

		override protected void MakeReadonly(bool breadonly)
		{
			// Do nothing.
		}
		

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.groupBox;
			}
		}

		internal override Control ChildContainer { get { return this.groupBox; } }

		internal override List<XmlElement>xChildren
		{
			get
			{
				XmlNodeList xAllChildren = m_xObject.ChildNodes;
				List<XmlElement> xRelevantChildren = new List<XmlElement>();
				foreach (XmlNode xChild in xAllChildren)
				{
					xRelevantChildren.Add(xChild as XmlElement);
				}

				return xRelevantChildren;
			}
			
		
		}

		private void button1_Click(object sender, EventArgs e)
		{
			if(this.m_height == 0)
				this.m_height = 22;
			else
				this.m_height = 0;

			XmlHelper.AddAttribute(this.m_xObject, "height", m_height.ToString());
			
			m_owner.LayoutAllChildren();

		}
	}
}

