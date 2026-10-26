using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace MenuSystemLibrary
{
	public partial class ModalEditText : Form
	{
		public string mText;

		private ModalEditText()
		{
			InitializeComponent();
		}

		public ModalEditText(bool bReadOnly) : this()
		{
			if(bReadOnly == true)
			{
				this.richTextBox1.ReadOnly = true;
				this.Text = "View Text";
				this.buttonCancel.Visible = false;
			}
		}

		private void buttonOk_Click(object sender, EventArgs e)
		{
			mText = richTextBox1.Text;

			DialogResult = DialogResult.OK;
		}

		private void ModalEditText_Shown(object sender, EventArgs e)
		{
			richTextBox1.Text = mText;
			richTextBox1.Select(mText.Length, 0);
			richTextBox1.Select(0, mText.Length);
		}
	}
}