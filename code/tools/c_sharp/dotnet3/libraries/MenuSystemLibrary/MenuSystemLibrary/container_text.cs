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
using System.Diagnostics;
using System.Media;

namespace MenuSystemLibrary
{
	public partial class Container_Text : MenuSystemLibrary.Container_Base
	{
		private Container_Text()
		{
			InitializeComponent();
		}

		bool m_bReadonly = false;

		public Container_Text(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			// Set the 'Enabled' property of this control.
			//m_bReadonly = XmlHelper.GetKeyBool(m_xObject, "readonly");
			//if(m_bReadonly == true)
			//{
			//    this.Enabled = true;					// Undo what the base class will have done because if this flag.
			//    this.buttonEdit.Text = "View ...";			// Button now is to 'view' rather than to 'edit'.
			//    this.richTextBox.ReadOnly = true;
			//}
		}



//		protected override Control ChildContainer { get { return null; } }
//		protected override ArrayList Children { get { return null; } }
		//////

		override internal string ObjectName
		{
//			get { return this.label.Text; }
			set { this.label.Text = value; }
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			this.richTextBox.ReadOnly = bReadonly;
			if(bReadonly == true)
			{
				this.Enabled = true;						// Undo what the base class will have done because if this flag.
				this.buttonEdit.Text = "View ...";			// Button now is to 'view' rather than to 'edit'.
				this.richTextBox.ReadOnly = true;
				this.richTextBox.BackColor = SystemColors.Control;
				//this.richTextBox.BackColor = Color.Brown;
			}
			else
			{
				this.Enabled = true;						// Undo what the base class will have done because if this flag.
				this.buttonEdit.Text = "Edit ...";			// Button now is to 'view' rather than to 'edit'.
				this.richTextBox.ReadOnly = false;
				this.richTextBox.BackColor = SystemColors.Window;
			}
		}

		override internal string ObjectValue
		{
			get { return this.richTextBox.Text; }
			set	{ this.richTextBox.Text = value; richTextBox.Select(richTextBox.Text.Length, 0);}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.richTextBox;
			}
		}

		private void richTextBox_TextChanged(object sender, EventArgs e)
		{
			ValueChanged("value", this.richTextBox.Text, false);
//			richTextBox.Select(richTextBox.Text.Length, 0);
		}

		private void button1_Click(object sender, EventArgs e)
		{
			ModalEditText met = new ModalEditText(m_bReadonly);
			met.mText = this.richTextBox.Text;

			DialogResult dr = met.ShowDialog(this);
			if(dr == DialogResult.OK)
				this.richTextBox.Text = met.mText;
		}

		private void richTextBox_LinkClicked(object sender, LinkClickedEventArgs e)
		{
			string strLink = e.LinkText;
			try
			{
				Process p = new Process();

				p.StartInfo = new ProcessStartInfo(strLink);

				p.Start();
			}
			catch(Exception ex)
			{
				try
				{
					SystemSounds.Beep.Play();

				}
				catch
				{
				
				}
			}
		}
	}
}

