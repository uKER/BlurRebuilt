//-----------------------------------------------------------------------
// <copyright file="AutomationExceptions.cs" company="Bizarre Creations">
// Copywright Bizarre Creations 2009
// </copyright>
//-----------------------------------------------------------------------
namespace Automation
{
    using System;

    public abstract class AutomationException : Exception
    {
        private readonly string command;
        private AutomationKit kit;

        public AutomationException(AutomationKit kit, string command, string message)
            : base(message)
        {
            this.command = command;
            this.kit = kit;
        }

        public AutomationKit Kit
        {
            get
            {
                return this.kit;
            }
        }

        public string Command
        {
            get
            {
                return this.command;
            }
        }
    }

    public class AutomationTimeoutException : AutomationException
    {
        public AutomationTimeoutException(AutomationKit kit, string command, string message)
            : base(kit, command, message)
        {
        }
    }
    
    public class AutomationCrashException : AutomationException
    {
        public AutomationCrashException(AutomationKit kit, string message)
            : base(kit, "Crash", message)
        {
        }
    }
    
    public class AutomationAssertException : AutomationException
    {
        private string condition;
        private string function;
        private string file;
        private uint line;

        public AutomationAssertException(AutomationKit kit, string message, string condition, string function, string file, uint line)
            : base(kit, "Assert", message)
        {
            this.condition = condition;
            this.function = function;
            this.file = file;
            this.line = line;
        }

        public string Condition
        {
            get
            {
                return this.condition;
            }
        }

        public string Function
        {
            get
            {
                return this.function;
            }
        }

        public string File
        {
            get
            {
                return this.file;
            }
        }

        public uint Line
        {
            get
            {
                return this.line;
            }
        }
    }
}
