using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using Global;
using System.IO;
using System.Diagnostics;

namespace MenuSystemLibrary
{
	public partial class Container_FolderBrowser : MenuSystemLibrary.Container_Base
	{
		string m_strPrefix;
		string m_strFullPrefix;
		string m_strHostName;

		Color m_backColour;

		string m_strOpenWith;

		private Container_FolderBrowser()
		{
			InitializeComponent();
		}

		public Container_FolderBrowser(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			string strValue = XmlHelper.GetKeyValue(m_xObject, "value");

			this.toolTip.SetToolTip(this.textBox, "String:\n\"" + strValue + "\"");

			m_strPrefix = XmlHelper.GetKeyValue(m_xObject, "filepicker_share_name");

			m_strOpenWith = XmlHelper.GetKeyValue(m_xObject, "open_with");

			string strStyle = XmlHelper.GetKeyValue(m_xObject, "style");

			if(strStyle == "sparse")
			{
				this.label.Visible = false;

				this.Height = 28;
			}

			m_backColour = this.textBox.BackColor;

			m_strHostName = System.Net.Dns.GetHostName();

			m_strFullPrefix = m_strPrefix = "";
			this.labelPrefix.Visible = false;
			this.textBox.Left = this.labelPrefix.Left;
			this.textBox.Width = this.buttonBrowse.Left - 5 - this.textBox.Left;
		}

		override internal string ObjectName
		{
			set { this.label.Text = value; }
		}

		override internal string ObjectValue
		{
			get
			{
				return this.textBox.Text;
			}

			set
			{
				this.textBox.Text = value;
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.textBox;
			}
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			this.textBox.ReadOnly = bReadonly;
			this.buttonBrowse.Enabled = !bReadonly;

			if(bReadonly == true)
				this.textBox.BackColor = SystemColors.Control;
			else
				this.textBox.BackColor = SystemColors.Window;
		}


		private void button_Click(object sender, EventArgs e)
		{
			FolderBrowserDialog fbd = new FolderBrowserDialog();

			fbd.Description = "Hello";
			this.ObjectValue = this.ObjectValue.Replace('/', '\\');
			//fbd.RootFolder = Environment.SpecialFolder.InternetCache;
			fbd.SelectedPath = ObjectValue;
			fbd.ShowNewFolderButton = true;

			DialogResult dr = fbd.ShowDialog(this);

			if(dr == DialogResult.OK)
			{
				{
					this.ObjectValue = fbd.SelectedPath;
				}
				this.textBox.Select(this.textBox.Text.Length, 0);
			}
		}

		private void textBox_TextChanged(object sender, EventArgs e)
		{
			this.textBox.BackColor = m_backColour;

			if(Directory.Exists(this.ObjectValue) == false)
			{
				this.textBox.BackColor = Color.Aquamarine;
			}

			ValueChanged("value", this.ObjectValue, false);
		}

		private void buttonMakePath_Click(object sender, EventArgs e)
		{
			try
			{
				Directory.CreateDirectory(this.ObjectValue);
			}
			catch(Exception ex)
			{
				MessageBox.Show(this, "Error creating path. System reports:\n'" + ex.Message + "'");
			}
		}

		private void buttonCopyFilePath_Click(object sender, EventArgs e)
		{
			string strFilename = this.ObjectValue;

			if(strFilename != null)
				Clipboard.SetDataObject(strFilename);
		}

		private void textBox_DragDrop(object sender, DragEventArgs e)
		{
			string strNewValue = null;

			if(e.Data.GetDataPresent(DataFormats.Text))
			{

				string fileName = (string)e.Data.GetData(DataFormats.Text);
				strNewValue = fileName;
				//				this.ObjectValue = fileName;
			}
			else if(e.Data.GetDataPresent(DataFormats.FileDrop))
			{
				string[] fileNames;
				fileNames = (string[])e.Data.GetData(DataFormats.FileDrop);
				string fileName = fileNames[0];
				strNewValue = fileNames[0];
				//				this.ObjectValue = fileName;
			}

			if(strNewValue != null)
			{
				this.ObjectValue = strNewValue;
			}
		}

		private void textBox_DragEnter(object sender, DragEventArgs e)
		{
			if(e.Data.GetDataPresent(DataFormats.FileDrop))
				e.Effect = DragDropEffects.Copy;
			else
				e.Effect = DragDropEffects.None;
		}



		private void buttonView_Click(object sender, EventArgs e)
		{
			if(m_strOpenWith == null || m_strOpenWith == "")
			{
				LaunchJob(this.textBox.Text, null, null);
			}
			else
			{
				LaunchJob(m_strOpenWith, this.textBox.Text, null);
			}
		}

		public bool LaunchJob(string strCommand, string strClArguments, string strWorkingDirectory)
		{
			try
			{
				Process p = new Process();

				p.StartInfo = new ProcessStartInfo(strCommand, strClArguments);

				if(strWorkingDirectory != null)
				{
					p.StartInfo.WorkingDirectory = strWorkingDirectory;
				}
				else
				{
					p.StartInfo.WorkingDirectory = Path.GetDirectoryName(strCommand);
				}

				p.Start();
				return true;
			}
			catch(Exception ex)
			{
				MessageBox.Show(this, "Cannot open the specified file. System reports:\n" + ex.Message, "Executer", MessageBoxButtons.OK);
				return false;
				//throw (ex);
			}
		}

		private void buttonOptions_Click(object sender, EventArgs e)
		{
			ContextMenu cm = new ContextMenu();

			string strDirectory = this.ObjectValue;

			bool bDirectoryExists = Directory.Exists(strDirectory);

			cm.MenuItems.Add("View", buttonView_Click);
			cm.MenuItems.Add("Copy", buttonCopyFilePath_Click);

			cm.MenuItems.Add("Make Path", buttonMakePath_Click);

			if(Directory.Exists(strDirectory))
			{
				cm.MenuItems[0].Enabled = true;
				cm.MenuItems[2].Enabled = false;
			}
			else
			{
				cm.MenuItems[0].Enabled = false;
				cm.MenuItems[2].Enabled = true;
			}

			cm.Show(this.buttonOptions, new Point(0, buttonOptions.Height));
		}
	}
}

