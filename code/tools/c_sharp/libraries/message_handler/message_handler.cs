using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Xml;
using System.IO;

namespace message_handler
{
	public static class MessageHandler
	{
		public enum MessageRedirect
		{
			None = 0,
			ToNull,
			ToConsole,
			ToLogfile,
			//EchoToNull,
			EchoToConsole,
			EchoToLogfile
		}

		static MessageRedirect m_memoryDialogRedirect = MessageRedirect.None;
		static MessageRedirect m_dialogRedirect = MessageRedirect.None;
		static MessageRedirect m_writeRedirect = MessageRedirect.None;

		static public MessageRedirect MemoryRedirect { get { return m_memoryDialogRedirect; } set { m_memoryDialogRedirect = value; } }
		static public MessageRedirect DialogRedirect { get { return m_dialogRedirect;       } set { m_dialogRedirect = value;       } }
		static public MessageRedirect WriteRedirect  { get { return m_writeRedirect;        } set { m_writeRedirect = value;        } }

		public static DialogResult ShowMemory(IWin32Window owner, string strId, string strManagerQuestion, string strDialogQuestion, string strCaption, MessageBoxButtons messageBoxButtons, MessageBoxIcon messageBoxIcon)
		{
			return ShowMemory(owner, strId, strManagerQuestion, strDialogQuestion, strCaption, messageBoxButtons, messageBoxIcon, false);
		}


		public static DialogResult ShowMemory(IWin32Window owner, string strId, string strManagerQuestion, string strDialogQuestion, string strCaption, MessageBoxButtons messageBoxButtons, MessageBoxIcon messageBoxIcon, bool bForce)
		{
			try
			{
				if(Redirect(m_memoryDialogRedirect, "MemoryID: " + strId + " Manager Question: " + strManagerQuestion + " Dialog Question: " + strDialogQuestion + " Buttons: " + messageBoxButtons.ToString() + "\n") == false)
				{
					DialogResult dr = MemoryDialog.Show(owner, strId, strManagerQuestion, strDialogQuestion, strCaption, messageBoxButtons, messageBoxIcon, bForce);
					return dr;
				}
				else
					return DialogResult.None;
			}
			catch
			{
				return DialogResult.None;
			}
		}


		public static DialogResult ManageMemory(IWin32Window owner)
		{
			return MemoryDialogForm.Manage(owner);
		}

		public static DialogResult GetAnswer(string strId)
		{
			return MemoryDialog.GetAnswer(strId);
		}

		public static bool SetAnswer(string strId, string strNewValue)
		{
			return MemoryDialog.SetAnswer(strId, strNewValue);
		}

		public static bool SetMemoryRoot(XmlElement xMemoryRoot)
		{
			return MemoryDialog.SetMemoryRoot(xMemoryRoot);
		}

		public static XmlElement GetMemoryRoot()
		{
			return MemoryDialog.GetMemoryRoot();
		}


		public static DialogResult GetDialogResultFromString(string strValue)
		{
			return MemoryDialog.GetDialogResultFromString(strValue);
		}


		/// /////////////////////////////////////////////////////////////////////

		public static DialogResult Show(IWin32Window owner, string text)
		{
			return Show(owner, text, "", MessageBoxButtons.OK, MessageBoxIcon.None, MessageBoxDefaultButton.Button1);
		}

		public static DialogResult Show(IWin32Window owner, string text, string caption)
		{
			return Show(owner, text, caption, MessageBoxButtons.OK, MessageBoxIcon.None, MessageBoxDefaultButton.Button1);
		}

		public static DialogResult Show(IWin32Window owner, string text, string caption, MessageBoxButtons buttons)
		{
			return Show(owner, text, caption, buttons, MessageBoxIcon.None, MessageBoxDefaultButton.Button1);
		}

		public static DialogResult Show(IWin32Window owner, string text, string caption, MessageBoxButtons buttons, MessageBoxIcon icon)
		{
			return Show(owner, text, caption, buttons, icon, MessageBoxDefaultButton.Button1);
		}

		public static DialogResult Show(IWin32Window owner, string text, string caption, MessageBoxButtons buttons, MessageBoxIcon icon, MessageBoxDefaultButton defaultButton)
		{
			if(Redirect(m_dialogRedirect, "DialogText: " + text + " Caption: " + caption + " Buttons: " + buttons.ToString()+"\n") == false)
				return MessageBox.Show(owner, text, caption, buttons, icon, defaultButton);
			else
				return DialogResult.None;
		}

		/// /////////////////////////////////////////////////////////////////////

		public static void WriteLine(string strMessage)
		{
			string strFullMessage = strMessage + "\n";
			if(Redirect(m_dialogRedirect, strFullMessage) == false)
				Write(strFullMessage);

			return;
		}

		public static void Write(string strMessage)
		{
			if(Redirect(m_dialogRedirect, strMessage) == false)
				Console.Write(strMessage);

			return;
		}

		/// /////////////////////////////////////////////////////////////////////

		private static bool Redirect(MessageRedirect m_memoryDialogRedirect, string strMessage)
		{
			switch(m_memoryDialogRedirect)
			{
				case MessageRedirect.None:
					return false;

				case MessageRedirect.ToNull:
					return true;

				case MessageRedirect.ToConsole:
					Console.Write(strMessage);
					return true;

				case MessageRedirect.ToLogfile:
					WriteToLogFile(strMessage);
					return true;

				case MessageRedirect.EchoToConsole:
					Console.WriteLine(strMessage);
					return false;

				case MessageRedirect.EchoToLogfile:
					WriteToLogFile(strMessage);
					return false;

				default:
					Debug.Assert(false, "MessageHandler.Redirect problem - bad value.");
					return false;
			}
		}


		static bool m_newFile = true;

		private static void WriteToLogFile(string strMessage)
		{
			try
			{
				//if(m_newFile == true)
				//{
				//    using(StreamWriter sw = File.CreateText(Application.ExecutablePath + ".Logfile"))
				//    {
				//        sw.WriteLine("Logfile created " + DateTime.Now.ToLongDateString() + " at " + DateTime.Now.ToLongTimeString());
				//        sw.WriteLine("\n\n");
				//        sw.WriteLine("Welcome to my world.\n\n");
				//    }
				//    m_newFile = false;
				//}


				using(StreamWriter sw = File.AppendText(Application.ExecutablePath + ".Logfile"))
				{
					sw.WriteLine(strMessage);
				}
			}
			catch
			{
			}
		}

		/// /////////////////////////////////////////////////////////////////////
	}	
}
