using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;

using XDevkit;

using Bizarre.Artery;

namespace Bizarre.Test
{
	public class XBoxAutomationConnection : AutomationConnection
	{
		//protected const int Version = 2;

		XboxManagerClass xboxManager = new XboxManagerClass();
		XboxConsole xboxConsole;

		public XBoxAutomationConnection(string connectionPrefix, string arteryName)
		{
			previousButtonState.RightTrigger = 1;
			previousButtonState.LeftTrigger = 1;

			this.xboxConsole = null;
			this.connectionPrefix = connectionPrefix;
			this.arteryName = arteryName;
			this.connectionOpen = false;

			ArteryManager.Startup(false, arteryName);

			conn = new Bizarre.Artery.Connection();
		}

		public override void Shutdown()
		{
			if (xboxConsole != null)
			{
				SendCommand("Disconnect360Controllers");

				if (connectionOpen)
				{
					try
					{
						xboxConsole.CloseConnection(connectionID);
					}
					catch
					{
					}

					connectionOpen = false;
				}
			}
		}

		public override String DefaultConsole
		{
			get
			{
				return xboxManager.DefaultConsole;
			}
		}

		public override bool Connected()
		{
			return (xboxConsole != null);
		}

		static void MyStdNotifyHandler(XboxDebugEventType eventCode, IXboxEventInfo eventInfo)
		{
			if (eventCode == XboxDebugEventType.DebugString)
			{
				XBOX_EVENT_INFO xei = eventInfo.Info;

				//System.Diagnostics.Debug.WriteLine("Dbg> " + xei.Message);
				if (DebugOutputLog != null)
				{
					DebugOutputLog.WriteLine(xei.Message);
				}
			}
		}

		public override bool IsSystemHUDVisible()
		{
			CommandResult result = SendCommand("IsXboxBladeDisplayed");

			return result.SingleLine == "1";
		}

		public override bool EnsureConnected()
		{
			if (xboxManager != null && xboxConsole == null)
			{
				xboxConsole = xboxManager.OpenConsole(consoleName);

				//System.Diagnostics.Debug.WriteLine("Running: " + xboxConsole.DebugTarget.RunningProcessInfo.ProgramName);

				xboxConsole.OnTextNotify += new XboxEvents_OnTextNotifyEventHandler(MyTextNotifyHandler);
				xboxConsole.OnStdNotify += new XboxEvents_OnStdNotifyEventHandler(MyStdNotifyHandler);

				try
				{
					connectionID = xboxConsole.OpenConnection(connectionPrefix);
					connectionOpen = true;
				}
				catch
				{
					System.Diagnostics.Debug.WriteLine(String.Format("ERROR: xboxConsole.OpenConnection failed ({0})", connectionPrefix));
				}

				if (!conn.IsConnected())
				{
					Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(xboxConsole.IPAddressTitle, 6317);

					Bizarre.Artery.ArteryManager.Connect(conn, target, "Automation", ProcessArteryMessagesCallback, 0);
				}
			}

			return (xboxManager != null && xboxConsole != null);
		}

		public override void InjectButton(uint pad, ControllerButtonFlags buttons)
		{
			if (assertFail || !connectionOpen)
			{
				return;
			}

			if (ControlLog != null)
			{
				ControlLog.WriteLine("Pressing: " + buttons.ToString());
			}

			if (ControlLogReplay != null)
			{
				ControlLogReplay.WriteLine("Press : " + buttons);
			}

			if (EnsureConnected())
			{
				try
				{
					SendCommand("CreateVirtual360Pad", 1);

					SendCommand("360ResetControls");

					List<ControllerButtonFlags> controls = GetButtonOption(buttons);
					foreach (ControllerButtonFlags cont in controls)
					{
						SetControlOption(cont);
					}

					//xboxConsole.XboxAutomation.SetGamepadState(0, ref xag);

					//xboxConsole.XboxAutomation.QueueGamepadState(pad, ref xag, 80, 1);

					List<ControllerButtonFlags> lButtons = GetButtonOption(buttons);
					foreach (ControllerButtonFlags button in lButtons)
					{
						//For buttons with no pressure sensing just send the button pressed
						if (!SetControlOption(button))
						{
							SendCommand("360Button", GetVirtualPadValue(button));
						}
					}

					SendPreviousButtonState();

					//invoke the display callback
					if (padButtonPressEventHandler360 != null)
					{
						padButtonPressEventHandler360(lButtons, previousButtonState);
					}
				}
				catch
				{
					//Artery not replying
				}
			}
		}

		//Send the previous state of the pressure buttons (To allow for "holding" buttons)
		public void SendPreviousButtonState()
		{
			SendCommand("360Axis", new object[] { GetVirtualPadValue(ControllerButtonFlags.LeftThumbX), previousButtonState.LeftThumbX });
			SendCommand("360Axis", new object[] { GetVirtualPadValue(ControllerButtonFlags.LeftThumbY), previousButtonState.LeftThumbY });
			SendCommand("360Axis", new object[] { GetVirtualPadValue(ControllerButtonFlags.RightThumbX), previousButtonState.RightThumbX });
			SendCommand("360Axis", new object[] { GetVirtualPadValue(ControllerButtonFlags.RightThumbY), previousButtonState.RightThumbY });
			SendCommand("360ButtonP", new object[] { GetVirtualPadValue(ControllerButtonFlags.LeftTrigger), previousButtonState.LeftTrigger });
			SendCommand("360ButtonP", new object[] { GetVirtualPadValue(ControllerButtonFlags.RightTrigger), previousButtonState.RightTrigger });

		}

		public int GetVirtualPadValue(ControllerButtonFlags button)
		{
			switch (button)
			{
				case ControllerButtonFlags.A_Button:
					return 1;
				case ControllerButtonFlags.B_Button:
					return 2;
				case ControllerButtonFlags.X_Button:
					return 3;
				case ControllerButtonFlags.Y_Button:
					return 4;
				case ControllerButtonFlags.DPadUp:
					return 5;
				case ControllerButtonFlags.DPadDown:
					return 6;
				case ControllerButtonFlags.DPadLeft:
					return 7;
				case ControllerButtonFlags.DPadRight:
					return 8;
				case ControllerButtonFlags.StartButton:
					return 9;
				case ControllerButtonFlags.BackButton:
					return 10;
				case ControllerButtonFlags.LS:
					return 11;
				case ControllerButtonFlags.RS:
					return 12;
				case ControllerButtonFlags.LeftTrigger:
					return 13;
				case ControllerButtonFlags.RightTrigger:
					return 14;
				case ControllerButtonFlags.LeftThumbstickButton:
					return 15;
				case ControllerButtonFlags.RightThumbstickButton:
					return 16;
				case ControllerButtonFlags.LeftThumbX:
					return 1;
				case ControllerButtonFlags.LeftThumbY:
					return 2;
				case ControllerButtonFlags.RightThumbX:
					return 3;
				case ControllerButtonFlags.RightThumbY:
					return 4;
			}
			return 0;
		}

		public bool SetControlOption(ControllerButtonFlags button)
		{
			//Support for non-button controls
			Random random = new Random();

			switch (button)
			{
				case ControllerButtonFlags.LeftThumbX:
					previousButtonState.LeftThumbX = 32767 - random.Next(65536);
					return true;
				case ControllerButtonFlags.LeftThumbY:
					previousButtonState.LeftThumbY = 32767 - random.Next(65536);
					return true;
				case ControllerButtonFlags.LeftTrigger:
					previousButtonState.LeftTrigger = (uint)(100 + random.Next(155));
					return true;
				case ControllerButtonFlags.RightThumbX:
					previousButtonState.RightThumbX = 32767 - random.Next(65536);
					return true;
				case ControllerButtonFlags.RightThumbY:
					previousButtonState.RightThumbY = 32767 - random.Next(65536);
					return true;
				case ControllerButtonFlags.RightTrigger:
					previousButtonState.RightTrigger = (uint)(100 + random.Next(155));
					return true;
				case ControllerButtonFlags.R_LeftThumbX:
					previousButtonState.LeftThumbX = 0;
					return true;
				case ControllerButtonFlags.R_LeftThumbY:
					previousButtonState.LeftThumbY = 0;
					return true;
				case ControllerButtonFlags.R_LeftTrigger:
					previousButtonState.LeftTrigger = 0;
					return true;
				case ControllerButtonFlags.R_RightThumbX:
					previousButtonState.RightThumbX = 0;
					return true;
				case ControllerButtonFlags.R_RightThumbY:
					previousButtonState.RightThumbY = 0;
					return true;
				case ControllerButtonFlags.R_RightTrigger:
					previousButtonState.RightTrigger = 0;
					return true;
			}
			return false;
		}

		public void SetControlOption(ref XBOX_AUTOMATION_GAMEPAD xag, ControllerButtonFlags button)
		{
			//Support for non-button controls
			Random random = new Random();

			switch (button)
			{
				case ControllerButtonFlags.LeftThumbX:
					previousButtonState.LeftThumbX = 32767 - random.Next(65536);
					xag.LeftThumbX = previousButtonState.LeftThumbX;
					break;
				case ControllerButtonFlags.LeftThumbY:
					previousButtonState.LeftThumbY = 32767 - random.Next(65536);
					xag.LeftThumbY = previousButtonState.LeftThumbY;
					break;
				case ControllerButtonFlags.LeftTrigger:
					previousButtonState.LeftTrigger = (uint)(100 + random.Next(155));
					xag.LeftTrigger = previousButtonState.LeftTrigger;
					break;
				case ControllerButtonFlags.RightThumbX:
					previousButtonState.RightThumbX = 32767 - random.Next(65536);
					xag.RightThumbX = previousButtonState.RightThumbX;
					break;
				case ControllerButtonFlags.RightThumbY:
					previousButtonState.RightThumbY = 32767 - random.Next(65536);
					xag.RightThumbY = previousButtonState.RightThumbY;
					break;
				case ControllerButtonFlags.RightTrigger:
					previousButtonState.RightTrigger = (uint)(100 + random.Next(155));
					xag.RightTrigger = previousButtonState.RightTrigger;
					break;
				case ControllerButtonFlags.R_LeftThumbX:
					xag.LeftThumbX = 0;
					previousButtonState.LeftThumbX = 0;
					break;
				case ControllerButtonFlags.R_LeftThumbY:
					xag.LeftThumbY = 0;
					previousButtonState.LeftThumbY = 0;
					break;
				case ControllerButtonFlags.R_LeftTrigger:
					xag.LeftTrigger = 0;
					previousButtonState.LeftTrigger = 0;
					break;
				case ControllerButtonFlags.R_RightThumbX:
					xag.RightThumbX = 0;
					previousButtonState.RightThumbX = 0;
					break;
				case ControllerButtonFlags.R_RightThumbY:
					xag.RightThumbY = 0;
					previousButtonState.RightThumbY = 0;
					break;
				case ControllerButtonFlags.R_RightTrigger:
					xag.RightTrigger = 1;
					previousButtonState.RightTrigger = 0;
					break;
			}

		}

		// Check to see what's running, if the exe we want is then use that, else launch a new copy
		public override bool LaunchOrConnect(bool forceRestart, string console, string exePath)
		{
			// Do this in stages...
			// 1. Check if exe is running (and if a forced restart is required), if not, make it so
			// 2. Try and establish a connection
			// 3. If we can't communicate, go to Stage 1 again. Do this a couple times

			this.consoleName = console;

			XboxConsole xboxConsole = xboxManager.OpenConsole(console);

			try
			{
				string currentExe = "x" + xboxConsole.RunningProcessInfo.ProgramName.ToLower();

				//If the game is already running and not attempting to force a restart
				if (!forceRestart && currentExe == exePath)
				{
					if (MessageBox.Show("The game is already running. Do you want to restart it?", "Game already running", MessageBoxButtons.YesNo) == DialogResult.No)
					{
						if (!conn.IsConnected())
						{
							Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(xboxConsole.IPAddressTitle, 6317);

							if (Bizarre.Artery.ArteryManager.Connect(conn, target, "Automation", ProcessArteryMessagesCallback, 0))
							{
								connectionOpen = true;
								return CheckVersion();
							}
						}
						else
						{
							connectionOpen = true;
							return CheckVersion();
						}
					}
				}

				// We need to start up the exe
				xboxConsole.Reboot(exePath, System.IO.Path.GetDirectoryName(exePath), "", XboxRebootFlags.Warm);

				// We try every 5 seconds (for 10 goes) to connect to Artery
				for (int iLoop = 0; iLoop < 10; iLoop++)
				{
					System.Threading.WaitHandle.WaitAll(new System.Threading.ManualResetEvent[] { new System.Threading.ManualResetEvent(false) }, 5000);

					bool success = false;

					try
					{
						Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(xboxConsole.IPAddressTitle, 6317);

						success = Bizarre.Artery.ArteryManager.Connect(conn, target, "Automation", ProcessArteryMessagesCallback, 0);
					}
					catch
					{
					}

					if (success)
					{
						connectionOpen = true;
						return CheckVersion();
					}
				}

				System.Diagnostics.Debug.WriteLine("FAIL: Could not connect to Artery");
			}
			catch
			{
				System.Diagnostics.Debug.WriteLine("FAIL: Something bad happened trying to connect to kit");
			}

			return false;
		}

		public override void Reset()
		{
			ProcessStartInfo reboot = new ProcessStartInfo("C:\\Program Files\\Microsoft Xbox 360 SDK\\bin\\win32\\xbreboot.exe");
			Process.Start(reboot);
		}
	}
}
