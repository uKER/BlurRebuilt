using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Xml;
using Global;
using SuperControl;

/// Provides a yes/no, abort/retry/cancel, OK (etc.) modal dialog box with an option to 'always perform this action'.
/// Functions are provided to set and retrieve the current table of remembered entries in Xml form.


namespace YesNoMemoryDialog
{
	internal partial class YesNoMemoryForm : Form
	{
		internal bool AlwaysIsChecked { get { return this.checkBox1.Checked; } }

		/// <summary>
		///  Used to hold the result of clicking one of the three buttons.
		/// Their value depends on what the button has ben configured to represent.
		/// </summary>
		private DialogResult m_button1Result;
		private DialogResult m_button2Result;
		private DialogResult m_button3Result;

		/// <summary>
		/// The basic form that is used to display the dialog. It is created, configured and shown modally by the class YesNoDialog.
		/// </summary>
		/// <param name="strText">The message to be displayed</param>
		/// <param name="strCaption">The title of the dialog</param>
		/// <param name="mbb">Enum to choose the button combination required</param>
		/// <param name="icon">Enum to choose the icon required</param>
		internal YesNoMemoryForm(string strText, string strCaption, MessageBoxButtons mbb, MessageBoxIcon icon)
		{
			InitializeComponent();
			this.richTextBox.Text = strText;
			this.Text = strCaption;

			/// Get the size that the text box needs to be to show the text without resorting to scroll bars.
			/// Use this to set the size of the form and the visibility of the buttons.
			Graphics g = this.richTextBox.CreateGraphics();
			Font font = this.richTextBox.Font;
			SizeF size = g.MeasureString(strText, font);

			/// BUG: This 'MeasureString' method doesn't seem to be very accurate. Perhaps I'm using it incorrectly.
			this.richTextBox.Width = (int)((float)size.Width*g.PageScale)+20;
			this.richTextBox.Height = (int)((float)size.Height*g.PageScale)+20;
			this.Width = (int)size.Width + richTextBox.Left * 2;
			this.Height = richTextBox.Top + richTextBox.Height + richTextBox.Top * 2 + button1.Height + 33;
			g.Dispose();

			button1.Visible = false;
			button2.Visible = false;
			button3.Visible = false;
			
			/// Configure the text on the buttons and the result they yield.
			switch(mbb)
			{
				case MessageBoxButtons.AbortRetryIgnore:
					{
						this.button1.Text = "Abort";
						this.m_button1Result = DialogResult.Abort;
						this.button1.Visible = true;

						this.button2.Text = "Retry";
						this.m_button2Result = DialogResult.Retry;
						this.button2.Visible = true;

						this.button3.Text = "Ignore";
						this.m_button3Result = DialogResult.Ignore;
						this.button3.Visible = true;
					}
					break;
				case MessageBoxButtons.OK:
					{
						this.button3.Text = "OK";
						this.m_button3Result = DialogResult.OK;
						this.button3.Visible = true;
						this.checkBox1.Text = "Don't ask again?";
					}
					break;
				case MessageBoxButtons.OKCancel:
					{
						this.button2.Text = "OK";
						this.m_button2Result = DialogResult.OK;
						this.button2.Visible = true;

						this.button3.Text = "Cancel";
						this.m_button3Result = DialogResult.Cancel;
						this.button3.Visible = true;
					}
					break;
				case MessageBoxButtons.RetryCancel:
					{
						this.button2.Text = "Retry";
						this.m_button2Result = DialogResult.Retry;
						this.button2.Visible = true;

						this.button3.Text = "Cancel";
						this.m_button3Result = DialogResult.Cancel;
						this.button3.Visible = true;
					}
					break;
				case MessageBoxButtons.YesNo:
					{
						this.button2.Text = "Yes";
						this.m_button2Result = DialogResult.Yes;
						this.button2.Visible = true;

						this.button3.Text = "No";
						this.m_button3Result = DialogResult.No;
						this.button3.Visible = true;
					}
					break;
				case MessageBoxButtons.YesNoCancel:
					{
						this.button1.Text = "Yes";
						this.m_button1Result = DialogResult.Yes;
						this.button1.Visible = true;

						this.button2.Text = "No";
						this.m_button2Result = DialogResult.No;
						this.button2.Visible = true;

						this.button3.Text = "Cancel";
						this.m_button3Result = DialogResult.Cancel;
						this.button3.Visible = true;
					}
					break;

				default:
					{
						Debug.Assert(false, "Bad value for MessageBoxButtons");
					}
					break;
			}
		}

		#region Return values
		/// <summary>
		/// Return the value that has been assigned to each button when it is clicked.
		/// </summary>
		private void button1_Click(object sender, EventArgs e)
		{
			this.DialogResult = this.m_button1Result;
		}

		private void button2_Click(object sender, EventArgs e)
		{
			this.DialogResult = this.m_button2Result;
		}

		private void button3_Click(object sender, EventArgs e)
		{
			this.DialogResult = this.m_button3Result;
		}
		#endregion Return values
	}

	/// <summary>
	/// The static class that is used to create, configure and show the YesNo form
	/// </summary>
	public class YesNoMemory
	{
		/// <summary>
		/// This is the Xml document that is used to remember the state of each dialog that has been handled in the past.
		/// </summary>
		static internal XmlDocument m_xml = null;

		/// <summary>
		/// Wrappers.
		/// </summary>
		/// 
		/// No force. No ManagerQuestion ...
		public static DialogResult Show(IWin32Window owner, string strId, string strText, string strCaption, MessageBoxButtons messageBoxButtons, MessageBoxIcon messageBoxIcon)
		{
			return Show(owner, strId, null, strText, strCaption, messageBoxButtons, messageBoxIcon, false);
		}

		/// Force. No ManagerQuestion ...
		public static DialogResult Show(IWin32Window owner, string strId, string strText, string strCaption, MessageBoxButtons messageBoxButtons, MessageBoxIcon messageBoxIcon, bool bForce)
		{
			return Show(owner, strId, null, strText, strCaption, messageBoxButtons, messageBoxIcon, false);
		}

		/// No force. ManagerQuestion ...
		public static DialogResult Show(IWin32Window owner, string strId, string strManagerQuestion, string strText, string strCaption, MessageBoxButtons messageBoxButtons, MessageBoxIcon messageBoxIcon)
		{
			return Show(owner, strId, strManagerQuestion, strText, strCaption, messageBoxButtons, messageBoxIcon, false);
		}
/**/
/*
		/// Force. ManagerQuestion ...
		public static DialogResult Show(IWin32Window owner, string strId, string strManagerQuestion, string strText, string strCaption, MessageBoxButtons messageBoxButtons, MessageBoxIcon messageBoxIcon, bool bForce)
		{
			return Show(owner, strId, strManagerQuestion, strText, strCaption, messageBoxButtons, messageBoxIcon, false);
		}
*/


		/// <summary>
		/// 
		/// </summary>
		/// <param name="owner">The owner of the form</param>
		/// <param name="strId">An id used to reference the memory entry for this dialog</param>
		/// <param name="strText">The message to be displayed</param>
		/// <param name="strCaption">The title of the dialog</param>
		/// <param name="messageBoxButtons">Enum to choose the button combination required</param>
		/// <param name="messageBoxIcon">Enum to choose the icon required</param>
		/// <param name="bForce">Force the dialog to ask regardless of the memory state for this dialog</param>
		/// <returns>DialogResult holding the user selection</returns>
		public static DialogResult Show(IWin32Window owner, string strId, string strManagerQuestion, string strText, string strCaption, MessageBoxButtons messageBoxButtons, MessageBoxIcon messageBoxIcon, bool bForce)
		{
			if(strManagerQuestion == null)
				strManagerQuestion = strText;

			/// Ensure there is a valid Xml memory document.
			if(m_xml == null)
			{
				m_xml = CreateXmlDocument();
			}
			XmlElement xMemory = m_xml["yes_no_memory"];

			if(xMemory == null)											/// File may have been edited by the user so may be corrupt.
			{
				ResetMemory();
				xMemory = m_xml["yes_no_memory"];
			}

			strId = MungeString(strId);									/// Replace any undesireable characters with '_'.

			XmlElement xId = xMemory[strId];
			if(xId == null)
			{
				xId = m_xml.CreateElement(strId);
				xMemory.AppendChild(xId);
			}
			string strValue = XmlHelper.GetKeyValue(xId, "value");		/// May or may not exist.

			DialogResult dr = GetDialogResultFromString(strValue);		/// Returns DialogResult.None if value is unrecognised. (E.g. == "ask")

			if(dr == DialogResult.None || bForce == true)				/// Ask the user what to do.
			{
				YesNoMemoryForm yesNoMemoryForm = new YesNoMemoryForm(strText, strCaption, messageBoxButtons, messageBoxIcon);
				
				dr = yesNoMemoryForm.ShowDialog(owner);

				if(yesNoMemoryForm.AlwaysIsChecked == true)				/// If the user wants to remember their selection then store it.
				{
					XmlHelper.AddAttribute(xId, "type", messageBoxButtons.ToString());
					XmlHelper.AddAttribute(xId, "value", dr.ToString());
					XmlHelper.AddAttribute(xId, "question", strManagerQuestion);
				}
				else
				{
					XmlHelper.AddAttribute(xId, "type", messageBoxButtons.ToString());
					XmlHelper.AddAttribute(xId, "value", "Ask");
					XmlHelper.AddAttribute(xId, "question", strManagerQuestion);
				}
			}

			return dr;
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="strId"></param>
		/// <param name="strAnswer"></param>
		/// <param name="overwriteExisting"></param>
		/// <returns></returns>
		public static bool CreateDefaultEntry(string strId, string strQuestion, string strAnswer, bool b_overwriteExisting)
		{
			XmlElement xMemory = GetMemoryElement();

			strId = MungeString(strId);

			XmlElement xId = xMemory[strId];
			if(xId == null || b_overwriteExisting == true)
			{
				if(xId != null)
					xMemory.RemoveChild(xId);

				xId = m_xml.CreateElement(strId);
					
					
				XmlHelper.AddAttribute(xId, "value", strAnswer);
				XmlHelper.AddAttribute(xId, "question", strQuestion);

				xMemory.AppendChild(xId);

				return true;
			}
			return false;
		}



		/// <summary>
		/// Returns a new string that has any non-alphanumberic characters replaced with an underscore.
		/// </summary>
		/// <param name="strInput">The string that may have non-alphanumeric characters.</param>
		/// <returns>A new string consisting of only alphanumberic characters or underscores.</returns>
		private static string MungeString(string strInput)
		{
			char[]cha = strInput.ToCharArray();

			for(int c = 0; c < cha.Length; c++)
			{
				if(char.IsLetterOrDigit(cha[c]) == false)
				{
					cha[c] = '_';
				}
			}
			return new string(cha);
		}


		#region Get Dialog result from string.
		private static DialogResult GetDialogResultFromString(string strValue)
		{
			DialogResult dr;

			switch(strValue)
			{
				case "Abort":
					{
						dr = DialogResult.Abort;
					}
					break;
				case "Cancel":
					{
						dr = DialogResult.Cancel;
					}
					break;
				case "Ignore":
					{
						dr = DialogResult.Ignore;
					}
					break;
				case "No":
					{
						dr = DialogResult.No;
					}
					break;
				case "None":
					{
						dr = DialogResult.None;
					}
					break;
				case "OK":
					{
						dr = DialogResult.OK;
					}
					break;
				case "Retry":
					{
						dr = DialogResult.Retry;
					}
					break;
				case "Yes":
					{
						dr = DialogResult.Yes;
					}
					break;
				default:
					dr = DialogResult.None;
					break;
			}
			return dr;
		}
#endregion Get Dialog result from string.

		/// <summary>
		/// Reset the memory of previous selections by recreating the memory document.
		/// </summary>
		/// <returns>true</returns>
		public static bool ResetMemory()
		{
			m_xml = CreateXmlDocument();
			return true;
		}

		/// <summary>
		/// Create the memory document and add the root element.
		/// </summary>
		/// <returns>The newly created document</returns>
		static XmlDocument CreateXmlDocument()
		{
			XmlDocument xmlDocument = new XmlDocument();
			XmlElement xMemory = xmlDocument.CreateElement("yes_no_memory");
			xmlDocument.AppendChild(xMemory);
			return xmlDocument;
		}

		/// <summary>
		/// Returns the root entry of the memory document so it can be stored by the controlling application.
		/// If the memory document does not exist then one is created.
		/// </summary>
		/// <returns>Returns the root entry of the memory document so it can be stored by the controlling application.</returns>
		public static XmlElement GetMemoryElement()
		{
			if(m_xml == null)
				m_xml = CreateXmlDocument();

			return m_xml["yes_no_memory"];
		}

		/// <summary>
		/// Imports the settings for a memory document so states can be restored by the controlling application.
		/// </summary>
		/// <param name="xElement">An Xml element representing a copy of the root memory element</param>
		/// <returns>true</returns>
		public static bool SetMemoryElement(XmlElement xElement)
		{
			ResetMemory();
			XmlElement xMemory = m_xml.ImportNode(xElement, true) as XmlElement;

			XmlNode xNode = m_xml[xMemory.Name];

			if(xNode != null)
			{
				m_xml.ReplaceChild(xMemory, xNode);
			}
			else
			{
				m_xml.AppendChild(xMemory);
			}

			return true;
		}

		public static bool SetAlwaysFlag(string strId, string strNewValue)
		{
			try
			{
				strId = MungeString(strId);
				XmlHelper.AddAttribute(YesNoMemory.GetMemoryElement()[strId], "value", strNewValue);
				return true;
			}
			catch
			{
				return false;
			}
		}


		public static DialogResult GetValue(string strId)
		{
			XmlElement xMemory = GetMemoryElement();

			strId = MungeString(strId);
			string strValue = null;

			XmlElement xId = xMemory[strId];

			if(xId != null)
			{
				strValue = XmlHelper.GetKeyValue(xId, "value");	/// May or may not exist.
			}
			return GetDialogResultFromString(strValue);
		}
	}
}

