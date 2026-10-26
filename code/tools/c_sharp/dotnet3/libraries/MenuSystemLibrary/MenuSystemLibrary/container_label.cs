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
	public partial class Container_Label : MenuSystemLibrary.Container_Base
	{
		private Container_Label()
		{
			InitializeComponent();
		}

		public Container_Label(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();
		}

		override internal string ObjectName
		{
			set { this.label1.Text = value; }
		}

		override internal string ObjectValue
		{
			get { return this.label1.Text; }
			set	{ /*this.label1.Text = value;*/ }
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