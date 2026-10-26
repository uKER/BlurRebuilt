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
	public partial class Container_MergeConflict : MenuSystemLibrary.Container_Base
	{
		private Container_MergeConflict()
		{
			InitializeComponent();
		}

		public Container_MergeConflict(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, false)
		{
			InitializeComponent();
		}

		override internal string ObjectName
		{
//			get { return null; }
			set { }
		}

		override internal string ObjectValue
		{
			get { return null; }
			set { }
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.groupBox2;
			}
		}
	}
}

