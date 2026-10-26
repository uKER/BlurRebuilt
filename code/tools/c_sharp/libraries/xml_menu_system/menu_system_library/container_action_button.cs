using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using Global;
using System.Diagnostics;
using System.IO;

namespace MenuSystemLibrary
{
	public partial class Container_ActionButton : MenuSystemLibrary.Container_Base
	{
		//private Container_ActionButton()
		//{
		//    InitializeComponent();
		//}

		public Container_ActionButton(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();
		}

		internal override string ObjectName
		{
			set { this.button1.Text = value; }
		}


		internal override string ObjectValue
		{
			get { return null; }
			set { }
		}

		internal override Control ObjectTooltipTarget
		{
			get { return this.button1; }
		}

		private static bool Shift
		{
			get { return (System.Windows.Forms.Control.ModifierKeys & Keys.Shift) != 0; }
		}

		private void Container_ActionButton_Load(object sender, EventArgs e)
		{
			string strName = XmlHelper.GetKeyValue(m_xObject, "display_name");
			if(strName == null)
			{
				strName = XmlHelper.GetKeyValue(m_xObject, "name");
				if(strName == null)
					strName = "";
			}

			strName = strName.Replace("\\n", "\n");

			XmlHelper.AddAttribute(m_xObject, "display_name", strName);
			//			this.button1.Text = strName;


			Graphics g = this.button1.CreateGraphics();
			SizeF fs = new SizeF(g.MeasureString(strName, this.button1.Font));
			g.Dispose();
			int textWidth = (int) fs.Width;
			int textHeight = (int) fs.Height;

			string strBitmap = XmlHelper.GetKeyValue(m_xObject, "bitmap");

			if(strBitmap != null)
			{
				Bitmap bmp;
				try
				{
					this.button1.Dock = DockStyle.Fill;
					bmp = new Bitmap(strBitmap);
				}
				catch
				{
					bmp = MenuSystemLibrary.Properties.Resources.Bitmap1;
				}

				this.Width = System.Math.Max(bmp.Width, textWidth) + 12;
				this.Height = bmp.Height + 8 + (textHeight*(strName == "" ? 0 : 1));
				this.button1.BackgroundImage = bmp;
				this.button1.TextAlign = ContentAlignment.BottomCenter;
				this.button1.ImageAlign = ContentAlignment.TopCenter;
			}
			else
			{
				this.Width = System.Math.Max(this.Width, textWidth + 12);
			}
			// TODO: Make the button fill it's parent.
			// Size the parent to the text in it.
		}

		protected override void MakeReadonly(bool breadonly)
		{
			// Do nothing. A disabled button makes no sense.
		}

		private void button1_Click(object sender, EventArgs e)
		{


			string strConfirmationText = XmlHelper.GetKeyValue(m_xObject, "confirmation_text");
			string strExternalTextReference = XmlHelper.GetKeyValue(m_xObject, "external_text");
			string strExternalText = "";

			if(!string.IsNullOrEmpty(strConfirmationText) || !string.IsNullOrEmpty(strExternalTextReference))
			{
				if(strConfirmationText == null)
					strConfirmationText = "";

				if(strExternalTextReference != null)
				{
					string[] parts = strExternalTextReference.Split(new char[] {'|'}, StringSplitOptions.RemoveEmptyEntries);

					string strArg = "";

					if(parts.Length == 2)
					{
						try
						{
							string strFilename = parts[0];
							string strxPath = parts[1];

							XmlDocument docDataDocument = new XmlDocument();
							docDataDocument.Load(strFilename);

							XmlNode xNode = docDataDocument.SelectSingleNode(strxPath);

							if(xNode != null)
								strArg = xNode.Value;
							else
							{
								strArg = "Cannot resolve xpath.";

							}

						}
						catch
						{
							strArg = "Cannot parse XmlDataContainer.";
						}
						strConfirmationText = strConfirmationText.Replace("%arg%", strArg);
					}


				}
				DialogResult dr = MessageBox.Show(this, strConfirmationText, "Executer", MessageBoxButtons.OKCancel);

				if(dr != DialogResult.OK)
					return;
			}





















			string strRawCommand = XmlHelper.GetKeyValue(m_xObject, "command");
			string strWorkingDirectory = XmlHelper.GetKeyValue(m_xObject, "working_directory");
			if(strRawCommand != null)
			{
				//strRawCommand = MenuSystem.ContainerExpand(strRawCommand, false);
				//strWorkingDirectory = MenuSystem.ContainerExpand(strWorkingDirectory, false);
				strRawCommand = this.m_owner.ContainerExpand(strRawCommand, false);
				strWorkingDirectory = this.m_owner.ContainerExpand(strWorkingDirectory, false);

				// Seperate command line and arguments ...
				string strCommand, strArguments;

				int argsIndex = 0;

				if(strRawCommand.StartsWith("\""))
				{
					argsIndex = strRawCommand.IndexOf("\"", 1);
				}
				else
				{
					argsIndex = strRawCommand.IndexOf(" ", 1);
				}

				if(argsIndex != -1)
				{
					strCommand = strRawCommand.Substring(0, argsIndex);
					strArguments = strRawCommand.Substring(argsIndex + 1);
				}
				else
				{
					strCommand = strRawCommand;
					strArguments = "";
				}
				try
				{
					// TODO: Expand arguments based on container name.
					// TODO: Optionally write out the full xml menu document.
					// TODO: Optionally specify the path for the full xml menu document.
					// TODO: Optionally expand the full xml menu document in the command line.


					if(Shift)
					{
						MessageBox.Show
							(this,
							 "Command: '" + strCommand +
							 "'\nArgs: '" + strArguments +
							 "'\nWorking dir: '" + strWorkingDirectory + "'",
							 "Menu button debug.");
					}
					else
					{
						Process p = new Process();
						p.StartInfo = new ProcessStartInfo(strCommand, strArguments);

						if(strWorkingDirectory != null)
						{
							p.StartInfo.WorkingDirectory = strWorkingDirectory;
						}
						else
						{
							p.StartInfo.WorkingDirectory = Path.GetDirectoryName(strCommand);
						}
						p.Start();
						//						System.Diagnostics.Process.Start(strCommand, "\"" + strFilename + "\"");
						//				System.Diagnostics.Process.Start(strCommand);
					}
				}
				catch(Exception ex)
				{
					MessageBox.Show
						(this, "Command failed.\nThe command was:'" + strRawCommand + "'\nThe working directory was:'" + strWorkingDirectory + "'\nSystem reports: " + ex.Message, "Executer - Error", MessageBoxButtons.OK,
						 MessageBoxIcon.Error);
				}
			}
		}
	}
}