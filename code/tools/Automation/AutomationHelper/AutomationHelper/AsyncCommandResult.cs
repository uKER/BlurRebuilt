namespace Automation
{
    using System;
    using System.Threading;

    public abstract class AutomationAsyncResult : IAsyncResult
    {
        private readonly uint commandID;

        protected Timer timeoutTimer;

        private ManualResetEvent asyncWaitHandle = new ManualResetEvent(false);
        bool isCompleted;

        // Result
        private CommandResult result = new CommandResult();

        protected AutomationConnection automationConnection;

        public AutomationAsyncResult(uint commandID, AutomationConnection conn)
        {
            this.commandID = commandID;
            this.automationConnection = conn;
            this.isCompleted = false;
        }

        public void Set()
        {
            this.isCompleted = true;
            this.asyncWaitHandle.Set();
        }

        public uint CommandID
        {
            get
            {
                return commandID;
            }
        }

        public Timer TimeoutTimer
        {
            get
            {
                return timeoutTimer;
            }

            set
            {
                timeoutTimer = value;
            }
        }

        public object AsyncState
        {
            get
            {
                return automationConnection;
            }
        }

        public bool IsCompleted
        {
            get
            {
                return isCompleted;
            }
        }

        public WaitHandle AsyncWaitHandle
        {
            get
            {
                return asyncWaitHandle;
            }
        }

        public bool CompletedSynchronously
        {
            get
            {
                return false;
            }
        }

        internal CommandResult Result
        {
            get
            {
                return result;
            }
        }

        abstract public string Type
        {
            get;
        }

        abstract public string Info
        {
            get;
        }
    }

    public class AsyncCommandResult : AutomationAsyncResult
    {
        private readonly string command;

        public AsyncCommandResult(uint commandID, string command, AutomationConnection conn)
            : base(commandID, conn)
        {
            this.command = command;
        }

        public string Command
        {
            get
            {
                return this.command;
            }
        }

        public override string Type
        {
            get
            {
                return "Command";
            }
        }

        public override string Info
        {
            get
            {
                return this.command;
            }
        }
    }
    
    public class DebugVariableAsyncResult : AutomationAsyncResult
    {
        private readonly string variable;
        private readonly string value;

        public DebugVariableAsyncResult(uint commandID, string variable, string value, AutomationConnection conn)
            : base(commandID, conn)
        {
            this.variable = variable;
            this.value = value;
        }

        public string Variable
        {
            get
            {
                return this.variable;
            }
        }
        
        public string Value
        {
            get
            {
                return this.value;
            }
        }

        public override string Type
        {
            get
            {
                return "DebugVariable";
            }
        }

        public override string Info
        {
            get
            {
                return this.variable + " : " + this.value;
            }
        }

    }
}
