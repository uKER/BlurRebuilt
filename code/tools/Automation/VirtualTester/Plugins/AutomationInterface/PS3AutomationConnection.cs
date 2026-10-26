using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;

using Bizarre.Artery;
//using Bizarre.PS3Interop;

using System.Runtime.InteropServices;

namespace Bizarre.Test
{

	public class PS3AutomationConnection : AutomationConnection
	{
		public int hTarget; //holds the target id of the PS3

		public TMAPI_HandleEventCallback ttyDelegate;

		public PS3AutomationConnection(string connectionPrefix, string arteryName)
		{
			this.hTarget = -1;
			this.connectionPrefix = connectionPrefix;
			this.arteryName = arteryName;
			this.connectionOpen = false;

			ArteryManager.Startup(false, arteryName);
			conn = new Bizarre.Artery.Connection();
		}

		public override void Shutdown()
		{
			SendCommand("DisconnectPS3Controllers");

			//if (hTarget!=-1) //if we have a target
			//{
			//    PS3Wrapper.StopPadCapture(hTarget);
			//    PS3Wrapper.StopPadPlayback(hTarget);
			//    PS3Wrapper.UnRegisterPadCaptureHandler(hTarget);
			//    PS3Wrapper.UnRegisterFileTraceHandler(hTarget);
			//    PS3Wrapper.UnRegisterPadPlaybackNotificationHandler(hTarget);
			//}

			connectionOpen = false;
		}

		public override string DefaultConsole
		{
			get
			{
				return ""; //TODO: find default console name
			}
		}

		public override bool IsSystemHUDVisible()
		{
			return false;
		}

		public override bool Connected()
		{
			return (PS3Wrapper.GetConnectStatus(hTarget) == ECONNECTSTATUS.CS_CONNECTED);
		}

		static void MyStdNotifyHandler(uint length, byte[] data)
		{
			if (length != 0)
			{
				if (DebugOutputLog != null)
				{
					string message = data.ToString();
					DebugOutputLog.WriteLine(message+"/n");
				}
			}
		}

		public override unsafe bool EnsureConnected()
		{
			PS3Wrapper.Kick();

			if (hTarget != -1)
			{
				//order Target Manager to connect to the supplied target
				PS3Wrapper.Connect(hTarget);

				connectionOpen = true;

				if (!conn.IsConnected())
				{
					SNPS3GamePortIPAddressData info = PS3Wrapper.GetGamePortDebugIPAddrData(hTarget, null);
					Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(info.uIPAddress, 6317);

					Bizarre.Artery.ArteryManager.Connect(conn, target, "Automation", ProcessArteryMessagesCallback, 0);
				}
			}

			return (hTarget != -1);
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
				ControlLogReplay.WriteLine("Press : " + " " + buttons);
			}

			if (EnsureConnected())
			{
				try
				{

					SendCommand("CreateVirtualPS3Pad", 1);

					//Start controller update
					SendCommand("PS3ResetControls");

					//Send the buttons that are pressed
					List<ControllerButtonFlags> lButtons = GetButtonOption(buttons);
					foreach (ControllerButtonFlags button in lButtons)
					{
						//For buttons with no pressure sensing just send the button pressed
						if (!SetControlOption(button))
						{
							SendCommand("PS3Button", GetVirtualPadValue(button));
						}
					}
					//Send previous state
					SendPreviousButtonState();

					//invoke the display callback
					if (padButtonPressEventHandlerPS3 != null)
					{
						padButtonPressEventHandlerPS3(lButtons, previousButtonState);
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
			SendCommand("PS3Axis", new object[] { GetVirtualPadValue(ControllerButtonFlags.LeftThumbX), previousButtonState.LeftThumbX });
			SendCommand("PS3Axis", new object[] { GetVirtualPadValue(ControllerButtonFlags.LeftThumbY), previousButtonState.LeftThumbY });
			SendCommand("PS3Axis", new object[] { GetVirtualPadValue(ControllerButtonFlags.RightThumbX), previousButtonState.RightThumbX });
			SendCommand("PS3Axis", new object[] { GetVirtualPadValue(ControllerButtonFlags.RightThumbY), previousButtonState.RightThumbY });
			SendCommand("PS3ButtonP", new object[] { GetVirtualPadValue(ControllerButtonFlags.LeftTrigger), previousButtonState.LeftTrigger });
			SendCommand("PS3ButtonP", new object[] { GetVirtualPadValue(ControllerButtonFlags.RightTrigger), previousButtonState.RightTrigger });

		}

		//Given a control, return its previous state value
		public int GetPreviousButtonState(ControllerButtonFlags button)
		{
			switch (button)
			{
				case ControllerButtonFlags.LeftThumbX:
					return previousButtonState.LeftThumbX;
				case ControllerButtonFlags.LeftThumbY:
					return previousButtonState.LeftThumbY;
				case ControllerButtonFlags.LeftTrigger:
					return (int)previousButtonState.LeftTrigger;
				case ControllerButtonFlags.RightThumbX:
					return previousButtonState.RightThumbX;
				case ControllerButtonFlags.RightThumbY:
					return previousButtonState.RightThumbY;
				case ControllerButtonFlags.RightTrigger:
					return (int)previousButtonState.RightTrigger;
			}

			return 0;
		}

		public int GetVirtualPadValue(ControllerButtonFlags button)
		{
			switch (button)
			{
				case ControllerButtonFlags.A_Button: //X
					return 1;
				case ControllerButtonFlags.B_Button: //O
					return 2;
				case ControllerButtonFlags.X_Button: //[]
					return 3;
				case ControllerButtonFlags.Y_Button: //^
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
					previousButtonState.LeftThumbX = 100 + random.Next(155);
					return true;
				case ControllerButtonFlags.LeftThumbY:
					previousButtonState.LeftThumbY = 100 + random.Next(155);
					return true;
				case ControllerButtonFlags.LeftTrigger:
					previousButtonState.LeftTrigger = (uint)(100 + random.Next(155));
					return true;
				case ControllerButtonFlags.RightThumbX:
					previousButtonState.RightThumbX = 100 + random.Next(155);
					return true;
				case ControllerButtonFlags.RightThumbY:
					previousButtonState.RightThumbY = 100 + random.Next(155);
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

		// Check to see what's running, if the exe we want is then use that, else launch a new copy
		public unsafe override bool LaunchOrConnect(bool forceRestart, string console, string exePath)
		{
			//initialises and launches Target Manager
			PS3Wrapper.InitTargetComms();

			// Do this in stages...
			// 1. Check if exe is running (and if a forced restart is required), if not, make it so
			// 2. Try and establish a connection
			// 3. If we can't communicate, go to Stage 1 again. Do this a couple times

			this.consoleName = console;

			ulong numTargets = PS3Wrapper.GetNumTargets();

			if (numTargets == 1)
			{
				hTarget = 0;
				//connect to that one target
				//hTarget = PS3Wrapper.GetTargetFromName(console.ToCharArray());
				PS3Wrapper.Connect(hTarget);
			}
			else if (numTargets == 0)
			{
				System.Diagnostics.Debug.WriteLine("FAIL: No targets are set in the PS3 Target Manager");
				return false;
			}
			else
			{
				//select the first connected kit we come across
				int i;
				for (i = 0; i < (int)numTargets; i++)
				{
					if (PS3Wrapper.GetConnectStatus(i) == ECONNECTSTATUS.CS_CONNECTED)
					{
						hTarget = i;
						break;
					}
				}

				//if no kits are already connected, prompt the user to pick one
				if (i == (int)numTargets)
				{
					hTarget = (int)PS3Wrapper.PickTarget(); //a selection box owned by the desktop

					//order Target Manager to connect to the supplied target
					PS3Wrapper.Connect(hTarget);
				}
			}

			ttyDelegate = new TMAPI_HandleEventCallback(PS3AutomationConnection.TTYEventHandlerCallback);

			PS3Wrapper.RegisterTTYEventHandler(hTarget, uint.MaxValue, ttyDelegate, System.IntPtr.Zero);
			
			//retreive a list of processes currently running on the target
			uint[] processList = PS3Wrapper.ProcessList(hTarget, PS3Wrapper.ProcessListBufferSize(hTarget));

			try
			{
				for (int i = 0; i < processList.Length; i++)
				{
				    if (processList[i] == 0)
				        break; //may be lots of empty space at the end of the list

				    string exeRunning = new string(PS3Wrapper.ProcessInfo(hTarget, processList[i], 
													PS3Wrapper.ProcessInfoBufferSize(hTarget, processList[i])).Hdr.szPath);
				    exeRunning = exeRunning.ToLower();

				    //if the exePath specified is already running and not attempting to force a restart
				    if (exeRunning.Contains(exePath.ToLower()) && !forceRestart)
				    {
						if (MessageBox.Show("The game is already running. Do you want to restart it?", "Game already running", MessageBoxButtons.YesNo) == DialogResult.No)
				        {
				            if (!conn.IsConnected())
				            {
								SNPS3GamePortIPAddressData info = PS3Wrapper.GetGamePortDebugIPAddrData(hTarget, null);
								Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(info.uIPAddress, 6317);

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
				}

				// We need to start up the exe
				Reset();
				System.Threading.WaitHandle.WaitAll(new System.Threading.ManualResetEvent[] { new System.Threading.ManualResetEvent(false) }, 100);
				uint pID = PS3Wrapper.ProcessLoad(hTarget, (uint)0x3e7, exePath.ToCharArray(), 0, new char[0,0], 0, new char[0,0], (uint)LoadParams.SNPS3_LOAD_FLAG_WAIT_BD_MOUNTED);
				System.Threading.WaitHandle.WaitAll(new System.Threading.ManualResetEvent[] { new System.Threading.ManualResetEvent(false) }, 3000);

				// We try every 5 seconds (for 10 goes) to connect to Artery
				for (int iLoop = 0; iLoop < 10; iLoop++)
				{
					System.Threading.WaitHandle.WaitAll(new System.Threading.ManualResetEvent[] { new System.Threading.ManualResetEvent(false) }, 5000);

					bool success = false;

					try
					{
						SNPS3GamePortIPAddressData info = PS3Wrapper.GetGamePortDebugIPAddrData(hTarget, null);
						Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(info.uIPAddress, 6317);

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
			catch(Exception e)
			{
				System.Diagnostics.Debug.WriteLine("FAIL: Something bad happened trying to connect to kit");
				System.Diagnostics.Debug.WriteLine(e.ToString());
			}

			return true;
		}

		//our callback handler for events generated by TTY output
		public static unsafe void TTYEventHandlerCallback(int hTarget, uint eventType,
			uint eventParam, SNRESULT result, uint dataLength, byte* eventData, void* userData)
		{
			if (dataLength != 0)
			{
				if (DebugOutputLog != null)
				{
					List<string> messages = new List<string>();
					char[] message = new char[dataLength];
					int c = 0;
					for (int b = 0; b < dataLength; b++)
					{
						message[c] = (char) eventData[b];
						if (message[c] == '\n')
						{
							string nextMessage = new string(message);
							int index = nextMessage.IndexOf('\n');
							if (index != -1)
							{
								nextMessage = nextMessage.Substring(0, index);
							}
							messages.Add(nextMessage);
							message = new char[dataLength-b];
							c = 0;
						}
						else
						{
							++c;
						}
					}
					foreach (string str in messages)
					{
						DebugOutputLog.WriteLine(str);
					}
				}
			}
		}

		public override void Reset()
		{
			//PS3Wrapper.SNPS3Reset(hTarget, (ulong)ResetType.SNPS3TM_RESETP_SOFT_RESET);
			PS3Wrapper.ResetEx(hTarget,
									(ulong)BootParams.SNPS3TM_BOOTP_DEBUG_MODE,
									(ulong)BootParams.SNPS3_BOOT_DEFAULT_MASK,
									(ulong)ResetType.SNPS3TM_RESETP_SOFT_RESET,
									(ulong)ResetType.SNPS3TM_RESETP_ALL_MASK,
									(ulong)SystemParams.SNPS3TM_SYSTEMP_TARGET_MODEL_60GB,
									(ulong)SystemParams.SNPS3TM_SYSTEMP_TARGET_MODEL);
		}

		//TODO: check if unused, should be ok to delete this if so
		private uint CreateUintIP(string ip)
		{
			int index = ip.IndexOf('\0');
			ip = ip.Remove(index);
			var ipAddress = System.Net.IPAddress.Parse(ip);
			var ipBytes = ipAddress.GetAddressBytes();
			var toReturn = (uint)ipBytes[3] << 24;
			toReturn += (uint)ipBytes[2] << 16;
			toReturn += (uint)ipBytes[1] << 8;
			toReturn += (uint)ipBytes[0];
			return toReturn;
		}
	}
}
