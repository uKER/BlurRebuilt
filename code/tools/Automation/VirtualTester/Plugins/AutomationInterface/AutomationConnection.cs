using System;
using System.Collections.Generic;
using System.Text;

using Bizarre.Artery;

namespace Bizarre.Test
{
	[Flags]
	public enum ControllerButtonFlags
	{
		DPadUp = 1,
		DPadDown = 2,
		DPadLeft = 4,
		DPadRight = 8,
		StartButton = 16,
		BackButton = 32,
		LeftThumbstickButton = 64,
		RightThumbstickButton = 128,
		LS = 256,
		RS = 512,
		Xbox360_Button = 1024,
		//		Bind = 2048,
		A_Button = 4096,
		B_Button = 8192,
		X_Button = 16384,
		Y_Button = 32768,
		LeftTrigger = 65536,
		RightTrigger = 131072,
		LeftThumbX = 262144,
		LeftThumbY = 524288,
		RightThumbX = 1048576,
		RightThumbY = 2097152,
		R_LeftTrigger = 4194304,
		R_RightTrigger = 8388608,
		R_LeftThumbX = 16777216,
		R_LeftThumbY = 33554432,
		R_RightThumbX = 67108864,
		R_RightThumbY = 134217728
	}

	public struct PreviousButtonState
	{
		public uint LeftTrigger;
		public uint RightTrigger;
		public int LeftThumbX;
		public int LeftThumbY;
		public int RightThumbX;
		public int RightThumbY;
	}

	public struct AvailableButtons
	{
		public OptionHandler<ControllerButtonFlags> buttons;
	}

	public struct Mode
	{
		public String name;
		public int timeToRun;
		public AvailableButtons defaultOptions;
		public Dictionary<String, AvailableButtons> screenActions;
		public Dictionary<String, List<string>> arteryMessages;
		public void Init(String Name, int Time)
		{
			name = Name;
			timeToRun = Time;
			screenActions = new Dictionary<string, AvailableButtons>();
			arteryMessages = new Dictionary<string, List<string>>();
		}
	}

	public delegate void PAD_BUTTON_PRESS_HANDLER(List<ControllerButtonFlags> buttonsPressed, PreviousButtonState prevButtons);

	public interface IAutomationConnection
	{
        void Shutdown();

		String DefaultConsole
		{
			get;
		}

        bool Connected();

        bool EnsureConnected();

        void PressButton(uint pad, ControllerButtonFlags buttons);

        void RandomButton(uint pad, OptionHandler<ControllerButtonFlags> buttons);

		List<ControllerButtonFlags> GetControlOption(ControllerButtonFlags multipleControls);

        //IAsyncResult BeginSendCommand(string cmd, TimeSpan timeout, callack);

		bool SendCommandViaArtery(uint cmdID, string command, params object[] optionalParams);

        bool SetGameVariable(string name, uint value);

        bool SetGameVariables(string name, uint[] values);

        IAsyncResult BeginSendCommand(string cmd, TimeSpan timeout, params object[] optionalParams);

        CommandResult EndSendCommand(IAsyncResult asyncResult);

        bool SetDebugVariable(string name, string value);

        bool SetDebugVariable(string name, bool value);

        bool SetDebugVariable(string name, int value);

        bool SetDebugVariable(string name, float value);

        bool SetDebugVariable(string name);

        CommandResult SendCommand(string cmd);

		CommandResult SendCommand(string cmd, params object[] optionalParams);

		CommandResult SendCommand(string cmd, TimeSpan timeout, params object[] optionalParams);

        string CurrentUIScreen();

        bool IsSystemHUDVisible();

        void HandleAutomationResponse(MessageReader msgReader);

        void ProcessArteryMessagesCallback(Bizarre.Artery.Connection con, System.Object context);

        bool LaunchOrConnect(bool forceRestart, string console, string exePath);

		void Reset();
	}

	public abstract class AutomationConnection : IAutomationConnection
	{
		protected const int Version = 2;

		protected string connectionPrefix;
		protected string arteryName;
		protected uint connectionID;
		protected bool connectionOpen;

		protected bool asserted = false;
		protected bool assertFail = false;

		protected PAD_BUTTON_PRESS_HANDLER padButtonPressEventHandler360;
		protected PAD_BUTTON_PRESS_HANDLER padButtonPressEventHandlerPS3;
		
		public bool Asserted
		{
			get { return assertFail; }
		}

		protected string consoleName;

		public static Bizarre.Automation.CustomLogging DebugOutputLog = null;
		public static Bizarre.Automation.CustomLogging ControlLog = null;
		public static Bizarre.Automation.CustomLogging ControlLogReplay = null;

		protected Connection conn;

		protected PreviousButtonState previousButtonState;

		public delegate void StopHandler(string message);

		protected event StopHandler StopEvent;

		// Commands
		private Dictionary<uint, AsyncCommandResult> pendingCommands = new Dictionary<uint, AsyncCommandResult>();

		/* This class is used only for currentCommandID to allow
		  * locking on it (ie needs to be reference type). */
		private class RotatingCommandID
		{
			public uint Value = 0x00;
		}
		RotatingCommandID currentCommandID = new RotatingCommandID();

		public void AddStopHandler(StopHandler myStop)
		{
			this.StopEvent += myStop;
		}

		public void SetDebugOutputLog(Bizarre.Automation.CustomLogging log)
		{
			DebugOutputLog = log;
		}

		public void SetControlLog(Bizarre.Automation.CustomLogging log)
		{
			ControlLog = log;
		}

		public void SetControlReplay(Bizarre.Automation.CustomLogging log)
		{
			ControlLogReplay = log;
		}

		private uint GetNextCommandID()
		{
			uint commandID;
			lock (currentCommandID) { commandID = currentCommandID.Value++; }

			return commandID;
		}

		public void PressButton(uint pad, ControllerButtonFlags buttons)
		{
			//Not button mashing so reset these other values
			previousButtonState.RightTrigger = 0;
			previousButtonState.LeftTrigger = 0;
			previousButtonState.LeftThumbX = 0;
			previousButtonState.LeftThumbY = 0;
			previousButtonState.RightThumbX = 0;
			previousButtonState.RightThumbY = 0;
			InjectButton(pad, buttons);
		}

		public void MashButton(uint pad, ControllerButtonFlags buttons)
		{
			InjectButton(pad, buttons);
		}

		public void RandomButton(uint pad, OptionHandler<ControllerButtonFlags> buttons)
		{
			buttons.GetRandomOption();
			MashButton(0, buttons.GetSelectedOption);
		}

		protected static void MyTextNotifyHandler(string Source, string Notification)
		{
			System.Diagnostics.Debug.WriteLine("MyTextNotifyHandler> '" + Source + "' '" + Notification + "'");
		}

		public List<ControllerButtonFlags> GetControlOption(ControllerButtonFlags multipleControls)
		{
			List<ControllerButtonFlags> controls = new List<ControllerButtonFlags>();

			uint total = (uint)multipleControls;

			while (total > 0)
			{
				uint val = 1;
				while (val * 2 <= total)
				{
					val *= 2;
				}
				controls.Add((ControllerButtonFlags)val);
				total -= val;
			}
			return controls;
		}

		public List<ControllerButtonFlags> GetButtonOption(ControllerButtonFlags multipleControls)
		{
			List<ControllerButtonFlags> buttons = new List<ControllerButtonFlags>();

			uint total = (uint)multipleControls;

			while (total > 0)
			{
				uint val = 1;
				while (val * 2 <= total)
				{
					val *= 2;
				}
				buttons.Add((ControllerButtonFlags)val);
				total -= val;
			}
			return buttons;
		}

		public bool SendCommandViaArtery(uint cmdID, string command, params object[] optionalParams)
		{
			Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();

			b.Write("Automation");
			b.Write(cmdID);
			b.Write(command);

			if (optionalParams != null)
			{
				foreach (object op in optionalParams)
				{
					if (op is string)
					{
						b.Write((string)op);
					}
					else if (op is int)
					{
						b.Write((int)op);
					}
					else if (op is uint)
					{
						b.Write((uint)op);
					}
					else if (op is long)
					{
						b.Write((long)op);
					}
					else if (op is float)
					{
						b.Write((float)op);
					}
					else if (op is Enum)
					{
						b.Write((int)op);
					}
					else if ((op is TimeSpan) == false)
					{
						System.Diagnostics.Debug.Assert(false, "Type being used which isn't handled by SendCommand");
					}
				}
			}

			Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
			if (b.Build(msg))
			{
				conn.Send(msg);

				return true;
			}

			return false;
		}

		private void TimeoutCommand(object state)
		{
			AsyncCommandResult asyncResult = (AsyncCommandResult)state;

			lock (asyncResult)
			{
				if (!asyncResult.IsCompleted)
				{
					asyncResult.Result.Type = ResultType.Timeout;
					asyncResult.Result.Error = "Command timed out.  " + asyncResult.Command;

					lock (pendingCommands)
					{
						pendingCommands.Remove(asyncResult.CommandID);
					}
					asyncResult.Set();

					//if (asyncResult.UserCallback != null)
					//    ThreadPool.QueueUserWorkItem(PerformAsyncUserCallback, asyncResult);
				}
			}
		}

		public bool SetGameVariable(string name, uint value)
		{
			uint[] values = {value};

			return SetGameVariables(name, values);
		}

		public bool SetGameVariables(string name, uint[] values)
		{
			if (EnsureConnected())
			{

				uint commandID = GetNextCommandID();

				Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();

				b.Write("Automation");
				b.Write((uint)commandID);
				b.Write((string)name);
				for (int v = 0; v < values.Length; v++)
				{
					b.Write((uint)values[v]);
				}

				Bizarre.Artery.Message msg = new Bizarre.Artery.Message();

				if (b.Build(msg))
				{
					conn.Send(msg);
					return true;
				}
			}

			return false;
		}

		public IAsyncResult BeginSendCommand(string cmd, TimeSpan timeout, params object[] optionalParams)
		{
			uint commandID = this.GetNextCommandID();

			AsyncCommandResult asyncResult = new AsyncCommandResult(commandID, cmd, this);

			asyncResult.TimeoutTimer = new System.Threading.Timer(this.TimeoutCommand, asyncResult, timeout, TimeSpan.FromMilliseconds(-1));

			// Track async result in internal collection
			lock (this.pendingCommands)
			{
				System.Diagnostics.Debug.Assert(!this.pendingCommands.ContainsKey(commandID), "This command ID is already in the list");
				this.pendingCommands.Add(commandID, asyncResult);
			}

			SendCommandViaArtery(commandID, cmd, optionalParams);

			return asyncResult;
		}

		public CommandResult EndSendCommand(IAsyncResult asyncResult)
		{
			AsyncCommandResult asyncCommandResult = (AsyncCommandResult)asyncResult;
			CommandResult result = asyncCommandResult.Result;

			asyncResult.AsyncWaitHandle.WaitOne();

			if (asyncCommandResult.TimeoutTimer != null)
			{
				asyncCommandResult.TimeoutTimer.Dispose();
				asyncCommandResult.TimeoutTimer = null;
			}

			//if (result.Type == ResultType.Error)
			//{
			//    throw new HookFailedException(asyncCommandResult.Command, result.Error);
			//}
			//else
			if (result.Type == ResultType.Timeout)
			{
				throw new AutomationTimeoutException(asyncCommandResult.Command, result.Error);
			}

			return result;
		}

		public bool SetDebugVariable(string name, string value)
		{
			if (EnsureConnected())
			{
				if (ControlLog != null)
				{
					ControlLog.WriteLine("SetDebugValue: '" + name + "' '" + value + "'");
				}

				if (ControlLogReplay != null)
				{
					ControlLogReplay.WriteLine("DebugV: '" + name + "' '" + value + "'");
				}

				Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();

				b.Write("DebugValue");
				b.Write(name);
				b.Write(value);

				Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
				if (b.Build(msg))
				{
					conn.Send(msg);
					return true;
				}
			}

			return false;
		}

		public bool SetDebugVariable(string name, bool value)
		{
			return SetDebugVariable(name, value ? "1" : "0");
		}

		public bool SetDebugVariable(string name, int value)
		{
			return SetDebugVariable(name, value.ToString());
		}

		public bool SetDebugVariable(string name, float value)
		{
			return SetDebugVariable(name, value.ToString());
		}

		public bool SetDebugVariable(string name)
		{
			return SetDebugVariable(name, "");
		}

		public CommandResult SendCommand(string cmd)
		{
			return SendCommand(cmd, TimeSpan.FromSeconds(10));
		}
		public CommandResult SendCommand(string cmd, object optionalParams)
		{
			return SendCommand(cmd, TimeSpan.FromSeconds(10), optionalParams);
		}


		public CommandResult SendCommand(string cmd, object[] optionalParams)
		{
			return SendCommand(cmd, TimeSpan.FromSeconds(10), optionalParams);
		}

		public CommandResult SendCommand(string cmd, TimeSpan timeout, params object[] optionalParams)
		{
			if (EnsureConnected())
			{
				IAsyncResult asyncResult = BeginSendCommand(cmd, timeout, optionalParams);

				return EndSendCommand(asyncResult);
			}
			else
			{
				return null;
			}
		}

		public string CurrentUIScreen()
		{
			CommandResult result = SendCommand("CurrentUIScreen");

			return result.SingleLine;
		}

		public string CurrentUISubscreen()
		{
			CommandResult result = SendCommand("CurrentUISubscreen");

			return result.SingleLine;
		}

		public void HandleAutomationResponse(MessageReader msgReader)
		{
			UInt32 commandID, reponseLineCount;

			msgReader.Read(out commandID);
			msgReader.Read(out reponseLineCount);

			// Process result
			AsyncCommandResult asyncResult = null;

			lock (pendingCommands)
			{
				if (pendingCommands.ContainsKey(commandID))
					asyncResult = pendingCommands[commandID];
			}

			if (asyncResult != null)
			{
				CommandResult result = asyncResult.Result;

				switch (reponseLineCount)
				{
					case 1:
						String response;
						msgReader.Read(out response);

						result.Type = ResultType.SingleLine;
						result.SingleLine = response;

						//System.Diagnostics.Debug.WriteLine(String.Format("AutoResponse: '{0}' '{1}'", asyncResult.Command, response));
						break;

					default:
						result.Type = ResultType.MultiLine;

						for (int iLine = 0; iLine < reponseLineCount; iLine++)
						{
							String line;

							msgReader.Read(out line);
							result.MultiLine.Add(line);
						}

						break;
				}


				lock (asyncResult)
				{
					if (!asyncResult.IsCompleted)
					{
						lock (pendingCommands)
						{
							pendingCommands.Remove(commandID);
						}
						asyncResult.Set();
					}
				}
			}

			//System.Diagnostics.Debug.WriteLine(String.Format("Automation: CommandID({0}) Response({1})", commandID, response));
		}

		public void ProcessArteryMessagesCallback(Bizarre.Artery.Connection con, System.Object context)
		{
			System.Diagnostics.Debug.WriteLine("Hello"); // > " + poo);

			while (con.IsConnected())
			{
				Bizarre.Artery.Message msg = new Bizarre.Artery.Message();

				if (con.Receive(msg))
				{
					Bizarre.Artery.MessageReader msgReader = new Bizarre.Artery.MessageReader(msg);

					String msgString;

					if (!msgReader.Read(out msgString))
						continue;

					switch (msgString)
					{
						case "Automation":
							HandleAutomationResponse(msgReader);
							break;

						case "STOP":
							String stopString;

							if (msgReader.Read(out stopString))
							{
								if (StopEvent != null)
								{
									// Call our stop handler to deal with a stop message
									StopEvent(stopString);
								}
							}

							break;

						default:
							System.Diagnostics.Debug.WriteLine("msgString: " + msgString);
							if (msgString == "Assert") //Watch out for Asserts
							{
								//Note that we have just asserted
								//assertFail = true;
							}
							break;
					}
				}
			}
		}

		protected bool CheckVersion()
		{
			try
			{
				CommandResult result = SendCommand("AutomationVersion", TimeSpan.FromSeconds(20));

				if (Convert.ToInt32(result.SingleLine) >= Version)
				{
					System.Diagnostics.Debug.WriteLine("SUCCESS: Connected to Artery");
					return true;
				}
				else
				{
					//System.Windows.Forms.MessageBox.Show(String.Format("The version of the automation channel support by the game is v{0}, this tool now requires v{1}. Please get a later version of the game to continue testing.", result.SingleLine, Version), "FAILED: " + consoleName, System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Error);
					System.Diagnostics.Debug.WriteLine(String.Format("FAILED: Automation version of game is {0} - Need {1}", result.SingleLine, Version));
				}
			}
			catch (AutomationTimeoutException)
			{
				//System.Windows.Forms.MessageBox.Show("A request was made to the game for a version number for the automation channel. No response was received, you might need a later version of the game that has this support.", "FAILED: " + consoleName, System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Error);
				System.Diagnostics.Debug.WriteLine("FAILED: No reply to Version request");
			}

			return false;
		}

		public void RegisterButtonPressHandler360(PAD_BUTTON_PRESS_HANDLER handler)
		{
			this.padButtonPressEventHandler360 = handler;
		}

		public void RegisterButtonPressHandlerPS3(PAD_BUTTON_PRESS_HANDLER handler)
		{
			this.padButtonPressEventHandlerPS3 = handler;
		}

		public abstract void Shutdown();

		public abstract String DefaultConsole
		{
			get;
		}

		public abstract bool IsSystemHUDVisible();

		public abstract bool Connected();

		public abstract bool EnsureConnected();

		public abstract void InjectButton(uint pad, ControllerButtonFlags buttons);

		public void ResetPreviousButtonState()
		{
			previousButtonState.LeftThumbX = 0;
			previousButtonState.LeftThumbY = 0;
			previousButtonState.LeftTrigger = 0;
			previousButtonState.RightThumbX = 0;
			previousButtonState.RightThumbY = 0;
			previousButtonState.RightTrigger = 0;
			InjectButton(0, 0);
		}

		public void SetLeftAxis(int x, int y)
		{
			previousButtonState.LeftThumbX = x;
			previousButtonState.LeftThumbY = y;
			InjectButton(0, 0);
		}

		public void SetRightAxis(int x, int y)
		{
			previousButtonState.RightThumbX = x;
			previousButtonState.RightThumbY = y;
			InjectButton(0, 0);
		}

		// Check to see what's running, if the exe we want is then use that, else launch a new copy
		public abstract bool LaunchOrConnect(bool forceRestart, string console, string exePath);

		public abstract void Reset();
	}
}