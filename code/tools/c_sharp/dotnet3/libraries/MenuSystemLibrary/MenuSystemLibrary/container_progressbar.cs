using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Collections;
using System.Xml;
using Global;
using System.Diagnostics;

namespace MenuSystemLibrary
{
	public partial class Container_Progressbar : MenuSystemLibrary.Container_Base
	{
		public Container_Progressbar()
		{
			InitializeComponent();
		}

		public Container_Progressbar(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();
		}

//		protected override Control ChildContainer {get{return null;}}
//		protected override ArrayList Children{get{return null;}}

		override internal string ObjectName
		{
//			set { this.progressBar.Text = value; }
			set { this.labelProgress.Text = value; }
		}

		override internal string ObjectValue
		{
			get { return this.progressBar.Value.ToString(); }
			set
			{
				int myvalue;
				int.TryParse(value, out myvalue);
				this.progressBar.Value = myvalue;

//				try
//				{
//					this.progressBar.Value = int.Parse(value);
//				}
//				catch(Exception)
//				{
//					this.progressBar.Value = 0;
//				}
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.progressBar;
			}
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			// Do nothing.
		}

//		protected override string GetExtraStatusText()
//		{
//			return strTooltip;
//		}
	}
}


