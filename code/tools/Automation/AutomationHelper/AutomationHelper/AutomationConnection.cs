//-----------------------------------------------------------------------
// <copyright file="AutomationConnection.cs" company="Bizarre Creations">
// Copywright Bizarre Creations 2009
// </copyright>
//-----------------------------------------------------------------------

namespace Automation
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading;
    using System.Windows.Forms;

    public class AutomationConnection : IDisposable
    {
        protected List<ProblemInfo> problems = new List<ProblemInfo>();

        private string buildVersion;
        private string buildDate;
        private string buildTime;

        private bool screenSaverDisabled = false;

        // Commands
        private Dictionary<uint, AutomationAsyncResult> pendingCommands = new Dictionary<uint, AutomationAsyncResult>();

        private RotatingCommandID currentCommandID = new RotatingCommandID();

        private AutomationKit kit;
        private WorkManager.BuildConfig buildConfig;

        private Bizarre.Artery.Connection consoleConnection;

        public AutomationConnection(AutomationKit kit, WorkManager.BuildConfig buildConfig)
        {
            this.kit = kit;
            this.buildConfig = buildConfig;

            if (kit.GetType() == typeof(AutomationKitPS3))
            {
                ((AutomationKitPS3)kit).SetConnection(this);
            }
        }

        public void Reset()
        {
            this.kit.Reset();
        }

        public void ClearProblems()
        {
            problems = new List<ProblemInfo>();
        }

        public string BuildVersion
        {
            get
            {
                return this.buildVersion;
            }
        }

        public string BuildDate
        {
            get
            {
                return this.buildDate;
            }
        }

        public string BuildTime
        {
            get
            {
                return this.buildTime;
            }
        }

        public AutomationKit Kit
        {
            get
            {
                return this.kit;
            }
        }

        public List<ProblemInfo> Problems
        {
            get
            {
                return problems;
            }
        }

        public List<ProblemInfo> FindProblems(string tag)
        {
            var list = from r in problems where r.Tag == tag select r;

            return list.ToList();
        }

        public List<ProblemInfo> FindProblems(string tag, string area)
        {
            var list = from r in problems where (r.Tag == tag && r.Area == area) select r;

            return list.ToList();
        }

        public List<ProblemInfo> FindProblems(string tag, string area, int index)
        {
            var list = from r in problems where (r.Tag == tag && r.Area == area && r.Index == index) select r;

            return list.ToList();
        }

        public void WriteLine(string text)
        {
            this.kit.WriteLine(text);
        }

        public int IsProblem(string tag, string area, int index)
        {
            return problems.Count(p => p.Tag == tag && p.Area == area && p.Index == index);
        }

        public int IsProblem(string tag, string area)
        {
            return problems.Count(p => p.Tag == tag && p.Area == area);
        }

        public void DisableScreenSaver()
        {
			if(this.Kit.GetType() == typeof(AutomationKit360))
			{
				this.SendCommand("EnableScreenSaver", "0", TimeSpan.FromSeconds(2));
			}
            screenSaverDisabled = true;
        }

        public void TakeScreenshot(string filename)
        {
            System.Diagnostics.Debug.Assert(screenSaverDisabled == true, "Ensure screen saver is disabled");
            kit.TakeScreenshot(filename);
        }

        public void Dispose()
        {
            if (this.consoleConnection != null)
            {
                this.consoleConnection.Close();
                this.consoleConnection.Dispose();
                this.consoleConnection = null;
            }
        }

        protected int WaitForTimeOrEvent(TimeSpan ts, WaitHandle[] wha)
        {
            int result;

            if (ts != TimeSpan.MaxValue)
            {
                result = WaitHandle.WaitAny(wha, ts, false);
            }
            else
            {
                result = WaitHandle.WaitAny(wha);
            }

            switch (result)
            {
                case 0:
                    {
                        if (WorkManager.StopOnAssert)
                        {
                            DialogResult dr = MessageBox.Show(string.Format("An Assert has fired on '{0}'", this.kit.Name), "Assert", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                        }

                        throw new AutomationAssertException(this.kit, this.kit.AssertDetails.Message, this.kit.AssertDetails.Condition, this.kit.AssertDetails.Function, this.kit.AssertDetails.File, this.kit.AssertDetails.Line);
                    }

                case 1:
                    {
                        if (WorkManager.StopOnCrash)
                        {
                            DialogResult dr = MessageBox.Show(string.Format("A Crash has occured on '{0}'", this.kit.Name), "Crash", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                        }

                        throw new AutomationCrashException(this.kit, "A crash happened");
                    }
            }

            return result;
        }

        public void WaitForTime(TimeSpan ts)
        {
            WaitHandle[] wha = { this.kit.AssertEvent, this.kit.ExceptionEvent };

            WaitForTimeOrEvent(ts, wha);
        }

        public bool WaitForEvent(WaitHandle ewh, TimeSpan ts)
        {
            WaitHandle[] wha = { this.kit.AssertEvent, this.kit.ExceptionEvent, ewh };

            int result = WaitForTimeOrEvent(ts, wha);

            // If we didn't time out, then everything went ok
            return result != WaitHandle.WaitTimeout;
        }

        public bool AutoConnectForAutomation(bool forceRebootOfKit)
        {
            bool autoConnect = this.AutoConnect(forceRebootOfKit);

            while (autoConnect)
            {
                if (this.GetToAutomationStartScreen())
                {
                    return true;
                }

                if (this.consoleConnection != null)
                {
                    this.consoleConnection.Close();
                    this.consoleConnection.Dispose();
                    this.consoleConnection = null;
                }

                this.kit.Launch(this.buildConfig);

                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(5));

                autoConnect = this.AutoConnect(false);
            }

            return false;
        }

        public bool AutoConnect(bool forceRebootOfKit)
        {
            if (this.buildConfig == null)
            {
                this.kit.WriteLine("AutoConnection: BuildConfig isn't set");
                return false;
            }

            try
            {
                for (int count = 0; count < 3; count++)
                {
                    bool needToLaunch = forceRebootOfKit || (count > 0);

                    // Is the exe already running and not crashed?
                    if (!this.kit.IsRunning(this.buildConfig))
                    {
                        needToLaunch = true;
                    }

                    if (needToLaunch)
                    {
                        this.kit.Launch(this.buildConfig);
                    }

                    if (this.TryAndEstablishChannel())
                    {
                        try
                        {
                            this.SendCommand("Ping", TimeSpan.FromSeconds(20));
                            
                            return true;
                        }
                        catch
                        {
                            // If we end up going through here then we have an artery connection, but the messages aren't being pumped
                        }
                    }

                    if (this.consoleConnection != null)
                    {
                        this.consoleConnection.Close();
                        this.consoleConnection.Dispose();
                        this.consoleConnection = null;
                    }
                }
            }
            catch (Exception ex)
            {
                this.kit.WriteLine("Exception: AutoConnect: " + ex.Message);
            }

            return false;
        }

        public CommandResult SendCommand(string cmd, params object[] optionalParams)
        {
            TimeSpan timeout = TimeSpan.FromSeconds(10);

            foreach (object ts in optionalParams)
            {
                if (ts.GetType() == typeof(TimeSpan))
                {
                    timeout = (TimeSpan)ts;
                }
            }

            if (WorkManager.Verbose)
            {
                this.WriteLine("SendCommand: " + cmd + " (Timeout: " + timeout.ToString() + ")");
            }

            IAsyncResult asyncResult = this.BeginSendCommand(cmd, timeout, optionalParams);

            CommandResult cr = this.EndSendCommand(asyncResult);

            if (cr.SingleLine == "-UNKNOWN-")
            {
                throw new Exception("Unknown Automation Command: " + cmd);
            }

            return cr;
        }

        public CommandResult SendDebugVariable(string variable, string value, params object[] optionalParams)
        {
            TimeSpan timeout = TimeSpan.FromSeconds(10);

            foreach (object ts in optionalParams)
            {
                if (ts.GetType() == typeof(TimeSpan))
                {
                    timeout = (TimeSpan)ts;
                }
            }

            if (WorkManager.Verbose)
            {
                this.WriteLine(string.Format("SendDebugVariable: '{0}' value({1}) (Timeout: {2})", variable, value, timeout.ToString()));
            }

            IAsyncResult asyncResult = this.BeginSendDebugVariable(variable, value, timeout);

            CommandResult cr = this.EndSendDebugVariable(asyncResult);

            if (cr.SingleLine == "-UNKNOWN-")
            {
                throw new Exception("Unknown Debug Variable: " + variable);
            }

            return cr;
        }

        public string CurrentUIScreen()
        {
            return this.CurrentUIScreen(TimeSpan.FromSeconds(2));
        }

        public string CurrentUIScreen(TimeSpan timeoutPeriod)
        {
            CommandResult result = this.SendCommand("CurrentUIScreen", timeoutPeriod);

            return (result != null) ? result.SingleLine : string.Empty;
        }

        protected void HandleAutomationResponse(Bizarre.Artery.MessageReader msgReader)
        {
            uint commandID, reponseLineCount;

            msgReader.Read(out commandID);
            msgReader.Read(out reponseLineCount);

            // Process result
            AutomationAsyncResult asyncResult = null;

            lock (this.pendingCommands)
            {
                if (this.pendingCommands.ContainsKey(commandID))
                {
                    asyncResult = this.pendingCommands[commandID];
                }
            }

            if (asyncResult != null)
            {
                CommandResult result = asyncResult.Result;

                switch (reponseLineCount)
                {
                    case 1:
                        string response;
                        msgReader.Read(out response);

                        result.Type = ResultType.SingleLine;
                        result.SingleLine = response;

                        //AutomationHelper.DebugOutput.Write(String.Format("AutoResponse: '{0}' '{1}'", asyncResult.Command, response));
                        break;

                    default:
                        result.Type = ResultType.MultiLine;

                        for (int lineCount = 0; lineCount < reponseLineCount; lineCount++)
                        {
                            string line;
                            msgReader.Read(out line);
                            result.MultiLine.Add(line);
                        }

                        break;
                }

                lock (asyncResult)
                {
                    if (!asyncResult.IsCompleted)
                    {
                        lock (this.pendingCommands)
                        {
                            this.pendingCommands.Remove(commandID);
                        }

                        asyncResult.Set();
                    }
                }
            }

            //AutomationHelper.DebugOutput.Write(String.Format("Automation: CommandID({0}) Response({1})", commandID, response));
        }

        protected virtual bool GetToAutomationStartScreen()
        {
            return true;
        }

        protected virtual bool HandleSpecialScreen(string screenName)
        {
            return false;
        }

        protected virtual void HandleNotifyMessage(Bizarre.Artery.MessageReader msgReader)
        {
        }
        
        protected virtual void HandleProblemMessage(Bizarre.Artery.MessageReader msgReader)
        {
        }

        protected void HandleAssertMessage(Bizarre.Artery.MessageReader msgReader)
        {
            string condition, file, function, message;
            uint line;

            if (msgReader.Read(out condition) &&
                msgReader.Read(out file) &&
                msgReader.Read(out line) &&
                msgReader.Read(out function) &&
                msgReader.Read(out message))
            {
                kit.AssertFired(message, condition, function, file, line);
            }
        }

        protected void ProcessArteryMessagesCallback(Bizarre.Artery.Connection con, object context)
        {
            if (WorkManager.Verbose)
            {
                this.kit.WriteLine("ProcessArteryMessagesCallback: Hello kit");
            }

            while (con.IsConnected())
            {
                Bizarre.Artery.Message msg = new Bizarre.Artery.Message();

                if (con.Receive(msg))
                {
                    Bizarre.Artery.MessageReader msgReader = new Bizarre.Artery.MessageReader(msg);

                    string msgString;

                    if (!msgReader.Read(out msgString))
                    {
                        continue;
                    }

                    switch (msgString)
                    {
                        case "Automation":
                            this.HandleAutomationResponse(msgReader);
                            break;

                        case "Notify":
                            this.HandleNotifyMessage(msgReader);
                            break;
                        
                        case "Problem":
                            this.HandleProblemMessage(msgReader);
                            break;

                        case "STOP":
                            string stopString;

                            if (msgReader.Read(out stopString))
                            {
                                //if (StopEvent != null)
                                //{
                                //    // Call our stop handler to deal with a stop message
                                //    StopEvent(stopString);
                                //}
                            }

                            break;

                        case "Assert":
                            this.HandleAssertMessage(msgReader);
                            break;

                        default:
                            if (WorkManager.Verbose)
                            {
                                this.kit.WriteLine("msgString: " + msgString);
                            }
                            break;
                    }
                }
            }

            if (WorkManager.Verbose)
            {
                this.kit.WriteLine("ProcessArteryMessagesCallback: Goodbye kit");
            }
        }

        protected bool TryAndEstablishChannel()
        {
            if (Automation.WorkManager.Verbose)
            {
                this.kit.WriteLine("AutomationConnection: TryAndEstablishChannel..." + this.kit.Running());
            }

            try
            {
                this.consoleConnection = new Bizarre.Artery.Connection();

                // We try and connect to the Artery Automation channel
                for (int tryCount = 0; tryCount < 10; tryCount++)
                {
                    if (Automation.WorkManager.Verbose)
                    {
                        this.kit.WriteLine("AutomationConnection: TryAndEstablishChannel - Try: " + tryCount);
                    }

                    if (!this.consoleConnection.IsConnected())
                    {
                        try
                        {
                            if (Automation.WorkManager.Verbose)
                            {
                                this.kit.WriteLine("AutomationConnection: TryAndEstablishChannel - Not connected");
                            }

                            Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(this.kit.IPAdddress(), 6317);

                            if (Automation.WorkManager.Verbose)
                            {
                                this.kit.WriteLine("AutomationConnection: TryAndEstablishChannel - Try Bizarre.Artery.ArteryManager.Connect");
                            }

                            if (!Bizarre.Artery.ArteryManager.Connect(this.consoleConnection, target, "Automation", this.ProcessArteryMessagesCallback, 0))
                            {
                                throw new Exception("Could not connect to artery.");
                            }

                            if (Automation.WorkManager.Verbose)
                            {
                                this.kit.WriteLine("AutomationConnection: TryAndEstablishChannel - Wait a bit for it to work...");
                            }

                            // Wait for a bit then try again...
                            System.Threading.Thread.Sleep(TimeSpan.FromSeconds(3));
                        }
                        catch (Exception ex)
                        {
                            this.WriteLine("AutomationConnection: TryAndEstablishChannel: Exception: " + ex.Message);
                        }
                    }

                    if (this.consoleConnection.IsConnected())
                    {
                        // Get build version
                        CommandResult result = this.SendCommand("BuildVersion", TimeSpan.FromSeconds(20));

                        if (this.kit.GetType() == typeof(AutomationKitPS3))
                        {
                            ((AutomationKitPS3)this.kit).PostArteryConnection();
                        }

                        string[] buildInfo = result.SingleLine.Split('!');

                        this.buildVersion = buildInfo[0];
                        this.buildDate = buildInfo[1];
                        this.buildTime = buildInfo[2];

                        if (Automation.WorkManager.Verbose)
                        {
                            this.kit.WriteLine(string.Format("Build Version: {0} ({1} - {2})", this.buildVersion, this.buildDate, this.buildTime));
                            this.kit.WriteLine("AutomationConnection: TryAndEstablishChannel - Connected");
                        }

                        return true;
                    }
                    else
                    {
                        this.consoleConnection.Close();
                    }
                }
            }
            catch (Automation.AutomationTimeoutException)
            {
                if (Automation.WorkManager.Verbose)
                {
                    this.kit.WriteLine("AutomationConnection: TimedOut");
                }
            }

            if (Automation.WorkManager.Verbose)
            {
                this.kit.WriteLine("AutomationConnection: TryAndEstablishChannel - Failed to connect");
            }

            if (this.consoleConnection != null)
            {
                this.consoleConnection.Close();
                this.consoleConnection.Dispose();
                this.consoleConnection = null;
            }

            return false;
        }

        private bool SendDebugVariableViaArtery(uint cmdID, string variable, string value)
        {
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();

            b.Write("DebugValue");
            b.Write(cmdID);
            b.Write(variable);
            b.Write(value);

            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                return this.consoleConnection.Send(msg);
            }

            return false;
        }

        private bool SendCommandViaArtery(uint cmdID, string command, object[] optionalParams)
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
                    else if (op is double)
                    {
                        b.Write((float)((double)op));
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
                return this.consoleConnection.Send(msg);
            }

            return false;
        }

        protected IAsyncResult BeginSendCommand(string cmd, TimeSpan timeout, object[] optionalParams)
        {
            try
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

                this.SendCommandViaArtery(commandID, cmd, optionalParams);

                return asyncResult;
            }
            catch (Exception ex)
            {
                this.kit.WriteLine("Exception: BeginSendCommand: " + ex.Message);

                return null;
            }
        }

        protected CommandResult EndSendCommand(IAsyncResult asyncResult)
        {
            AsyncCommandResult asyncCommandResult = (AsyncCommandResult)asyncResult;
            CommandResult result = asyncCommandResult.Result;

            WaitHandle[] wha = { asyncResult.AsyncWaitHandle, this.kit.AssertEvent, this.kit.ExceptionEvent };

            WaitForEvent(asyncResult.AsyncWaitHandle, TimeSpan.MaxValue); // WaitHandle.WaitAny(wha, ts, false);

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
                throw new AutomationTimeoutException(this.kit, asyncCommandResult.Command, result.Error);
            }

            return result;
        }

        protected IAsyncResult BeginSendDebugVariable(string variable, string value, TimeSpan timeout)
        {
            try
            {
                uint commandID = this.GetNextCommandID();

                DebugVariableAsyncResult asyncResult = new DebugVariableAsyncResult(commandID, variable, value, this);

                asyncResult.TimeoutTimer = new System.Threading.Timer(this.TimeoutCommand, asyncResult, timeout, TimeSpan.FromMilliseconds(-1));

                // Track async result in internal collection
                lock (this.pendingCommands)
                {
                    System.Diagnostics.Debug.Assert(!this.pendingCommands.ContainsKey(commandID), "This command ID is already in the list");
                    this.pendingCommands.Add(commandID, asyncResult);
                }

                this.SendDebugVariableViaArtery(commandID, variable, value);

                return asyncResult;
            }
            catch (Exception ex)
            {
                this.kit.WriteLine("Exception: BeginSendCommand: " + ex.Message);

                return null;
            }
        }

        protected CommandResult EndSendDebugVariable(IAsyncResult asyncResult)
        {
            DebugVariableAsyncResult asyncCommandResult = (DebugVariableAsyncResult)asyncResult;
            CommandResult result = asyncCommandResult.Result;

            WaitHandle[] wha = { asyncResult.AsyncWaitHandle, this.kit.AssertEvent, this.kit.ExceptionEvent };

            WaitForEvent(asyncResult.AsyncWaitHandle, TimeSpan.MaxValue); // WaitHandle.WaitAny(wha, ts, false);

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
                throw new AutomationTimeoutException(this.kit, "DebugVariable: " + asyncCommandResult.Variable + " = " + asyncCommandResult.Value, result.Error);
            }

            return result;
        }

        public void ClearPadState(uint controllerPort)
        {
			this.kit.ClearPadState(controllerPort);
        }

        public void SelectButtonPress(uint controllerPort, AutomationKitButton button)
        {
            this.kit.SelectButtonPress(controllerPort, button);
        }

        public void SelectLeftStick(uint controllerPort, double x, double y)
        {
            this.kit.SelectLeftStick(controllerPort, x, y);
        }

        public void SelectRightStick(uint controllerPort, double x, double y)
        {
            this.kit.SelectRightStick(controllerPort, x, y);
        }

        public bool SetPadState(uint controllerPort, bool oneFrame)
        {
            this.kit.SetPadState(controllerPort, oneFrame);

            return true;
        }

        private uint GetNextCommandID()
        {
            lock (this.currentCommandID)
            {
                return this.currentCommandID.Value++;
            }
        }

        private void TimeoutCommand(object state)
        {
            AutomationAsyncResult asyncResult = (AutomationAsyncResult)state;

            lock (asyncResult)
            {
                if (!asyncResult.IsCompleted)
                {
                    asyncResult.Result.Type = ResultType.Timeout;
                    asyncResult.Result.Error = asyncResult.Type + " timed out.  " + asyncResult.Info;

                    lock (this.pendingCommands)
                    {
                        this.pendingCommands.Remove(asyncResult.CommandID);
                    }

                    asyncResult.Set();
                }
            }
        }

        private class RotatingCommandID
        {
            private uint value = 0x00;

            public uint Value
            {
                get
                {
                    return this.value;
                }

                set
                {
                    this.value = value;
                }
            }
        }

        public class ProblemInfo
        {
            private string tag;
            private string[] avalues;
            private int index;

            public ProblemInfo(string tag, string[] areaVals, int index)
            {
                this.tag = tag;
                this.avalues = areaVals;
                this.index = index;
            }

            public string Tag
            {
                get
                {
                    return tag;
                }
            }

            public string Area
            {
                get
                {
                    return avalues[0];
                }
            }

            public int ValuesCount
            {
                get
                {
                    return (avalues.Count() > 0) ? avalues.Count() - 1 : 0;
                }
            }

            public string Value(uint index)
            {
                System.Diagnostics.Debug.Assert(index + 1 < avalues.Count());
                return avalues[index + 1];
            }
            
            public int Index
            {
                get
                {
                    return index;
                }
            }
        }
    }
}

