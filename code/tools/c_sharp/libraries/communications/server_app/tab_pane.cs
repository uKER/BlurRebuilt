using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using comms_tcp;
using System.Diagnostics;

namespace server_app
{
	public partial class TabPane : UserControl
	{
		delegate void InvokedText(String strText);
		InvokedText m_invokedText;

		public TabPane()
		{
			InitializeComponent();

			m_invokedText = InvokedWriteString;
		}

		public void WriteString(string str)
		{
			Invoke(m_invokedText, new object[] { str });
		}

		private delegate bool d_ChangeName(string str);
		private delegate bool d_Remove(TabPage tp);

		bool ChangeName(string strNewName)
		{
			this.Parent.Text = strNewName;
			return true;
		}

		bool Remove(TabPage tp)
		{
			(tp.Parent as TabControl).TabPages.Remove(tp);
			return true;
		}

		/*
				public bool DataCallback(comms_tcp.cGlobal.CallbackType callbackType, object data)
				{
					// TODO: Need a way for this tabPane to talk back to the client.
					if (callbackType == communications_tcp.cGlobal.CallbackType.ReceivedData)
					{
						string strMessage = data as string;

						if(strMessage.StartsWith(cGlobal.STR_COMMAND))
						{
							string strCommand = strMessage.Substring(cGlobal.STR_COMMAND.Length);

							string[] strParams = strCommand.Split(new char[] { ' ', '\t', '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries);

							switch(strParams[0])
							{
								case "NAME":
									{
										d_ChangeName app = new d_ChangeName(this.ChangeName);
										this.Invoke(app, new object[] { strParams[1] });

										return true;
									}
								default:
									{
										break;
									}
							}
						}
						else
						{
							WriteString(strMessage);

							if((strMessage).Contains("progress"))
							{
								m_commsListener.SendData("Server says: '" + strMessage + "'\n");
							}
						}

						return true;
					}
					else if(callbackType == cGlobal.CallbackType.Disconnected)
					{
						d_Remove app = new d_Remove(this.Remove);
						this.Invoke(app, new object[]{this.Parent});
						return true;
					}
					else
						return false;
				}
		*/
		void InvokedWriteString(string str)
		{
			this.richTextBox.AppendText(str + "\n");
		}

		CommsListener m_commsListener = null;

		internal void SetCommsListener(CommsListener commsListener)
		{
			m_commsListener = commsListener;

			commsListener.DataReceivedEvent += new DataCallbackDelegate(commsListener_DataReceivedEvent);
			commsListener.DataSentEvent += new DataCallbackDelegate(commsListener_DataSentEvent);
			commsListener.ConnectionStateChangedEvent += new ConnectionStateChangedDelegate(commsListener_ConnectionStateChangedEvent);
		}

		void commsListener_ConnectionStateChangedEvent(object sender, TcpConnectionCallbackEventArgs e)
		{
			if(e.CommsListener.Connected == false)
			{
				d_Remove app = new d_Remove(this.Remove);
				this.Invoke(app, new object[] { this.Parent });
			}
		}

		void commsListener_DataSentEvent(object sender, TcpDataCallbackEventArgs e)
		{
			//throw new ApplicationException("The method or operation is not implemented.");
		}

		void commsListener_DataReceivedEvent(object sender, TcpDataCallbackEventArgs e)
		{
			// TODO: Need a way for this tabPane to talk back to the client.

			//string strMessage = data as string;
			Debug.Assert(e.Data is string);
			string strMessage = e.Data as string;

			if(strMessage.StartsWith(cGlobal.STR_COMMAND))
			{
				string strCommand = strMessage.Substring(cGlobal.STR_COMMAND.Length);

				string[] strParams = strCommand.Split(new char[] { ' ', '\t', '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries);

				switch(strParams[0])
				{
					case "NAME":
						{
							d_ChangeName app = new d_ChangeName(this.ChangeName);
							this.Invoke(app, new object[] { strParams[1] });

							return;
						}
					default:
						{
							break;
						}
				}
			}
			else
			{
				WriteString(strMessage);

				if((strMessage).Contains("progress"))
				{
					m_commsListener.SendData("Server says: '" + strMessage + "'\n");
				}
			}
		}
	}
}
