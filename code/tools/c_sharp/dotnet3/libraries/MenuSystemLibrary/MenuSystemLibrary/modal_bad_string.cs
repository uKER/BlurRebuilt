using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace MenuSystemLibrary
{
	public partial class ModalBadString : Form
	{
		public ModalBadString(string strHeading, string strMessage)
		{
			InitializeComponent();

			this.Text = strHeading;
			this.label.Text = strMessage;
		}

		public enum result
		{
			None = 0,
			EditString,
			CreateDefault,
			Ignore
		};

		public result Result
		{
			get
			{
				if(this.radioButton1.Checked == true)
					return result.EditString;
				else if(this.radioButton2.Checked == true)
					return result.CreateDefault;
				else if(this.radioButton3.Checked == true)
					return result.Ignore;
				else
					return result.None;
			}
		}
	}
}