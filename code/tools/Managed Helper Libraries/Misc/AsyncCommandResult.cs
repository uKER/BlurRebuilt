namespace Bizarre.Misc
{
    using System;
    using System.Threading;

    public class GenericAsyncResult : IAsyncResult
    {
        private readonly uint commandID;

        protected Timer timeoutTimer;

        private ManualResetEvent asyncWaitHandle = new ManualResetEvent(false);
        bool isCompleted;

        // Result
        private GenericCommandResult result = new GenericCommandResult();

        //protected AutomationConnection automationConnection;

        public GenericAsyncResult(uint commandID) //, AutomationConnection conn)
        {
            this.commandID = commandID;
            //this.automationConnection = conn;
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
                throw new NotImplementedException();
                //return automationConnection;
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

        public GenericCommandResult Result
        {
            get
            {
                return result;
            }
        }
    }

    public class AsyncCommandResult : GenericAsyncResult
    {
        private readonly string command;

        public AsyncCommandResult(uint commandID, string command) //, AutomationConnection conn)
            : base(commandID) // , conn)
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
    }
}
