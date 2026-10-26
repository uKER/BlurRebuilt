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
	public partial class ContainerRoot : MenuSystemLibrary.Container_Base
	{
		public ContainerRoot()
		{
			InitializeComponent();
		}

		internal override string ObjectName
		{
			//			get { return this.groupBox.Text; }
			set { }
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
				return this;
			}
		}

		internal override Control ChildContainer { get { return this; } }

		internal override List<XmlElement> xChildren
		{
			get
			{
				XmlNodeList xAllChildren = m_xObject.ChildNodes;
				List<XmlElement> xRelevantChildren = new List<XmlElement>();
				foreach(XmlElement xChild in xAllChildren)
				{
					if(xChild != null)
						if(xChild.Name == "container")
							xRelevantChildren.Add(xChild as XmlElement);
				}

				return xRelevantChildren;
			}
		}

	}
}

