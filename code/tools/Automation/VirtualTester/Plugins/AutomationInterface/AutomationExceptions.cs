using System;

namespace Bizarre.Test
{
    public abstract class AutomationException : Exception
    {
        private readonly string command;

        public string Command
        {
            get
            {
                return command;
            }
        }

        public AutomationException(string command, string message) : base(message)
        {
            this.command = command;
        }
    }

    public class AutomationTimeoutException : AutomationException
    {
        public AutomationTimeoutException(string command, string message) : base(command, message)
        {
        }
    }
}
