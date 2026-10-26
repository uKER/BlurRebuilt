using System;
using System.Threading;

using Bizarre.Test;

namespace Bizarre.Test
{
    public class AsyncCommandResult : IAsyncResult
    {
        private readonly uint commandID;
        private readonly string command;

        protected Timer timeoutTimer;

        private ManualResetEvent asyncWaitHandle = new ManualResetEvent(false);
        bool isCompleted;

        // Result
        private CommandResult result = new CommandResult();

        protected Bizarre.Test.AutomationConnection automationConnection;

        public AsyncCommandResult(uint commandID, string command, Bizarre.Test.AutomationConnection conn)
        {
            this.commandID = commandID;
            this.command = command;
            this.automationConnection = conn;
            this.isCompleted = false;
        }

        public void Set()
        {
            this.isCompleted = true;
            this.asyncWaitHandle.Set();
        }

        public string Command
        {
            get
            {
                return command;
            }
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
    }
}
