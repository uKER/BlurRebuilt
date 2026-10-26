namespace Automation
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    using Bizarre.Misc.LogSystem;

    using System.Threading;

    /// <summary>
    /// The type of kit we're running automation on
    /// </summary>
    public enum AutomationKitType
    {
        Xbox360     = 0,
        PS3,
        WIN32,
        WIN64
    }

    public enum AutomationKitStatus
    {
        Available   = 0,
        Locked,
        OutOfOrder,             // Hasn't responded so we write off as not available
    }

    public enum AutomationKitButton
    {
        Button_A = 0,
        Button_B,
        Button_X,
        Button_Y,

		Button_DPad_Up,
		Button_DPad_Down,
		Button_DPad_Left,
		Button_DPad_Right,
        
        Button_Cross = Button_X,
        Button_Circle = Button_Y,
        Button_Square = Button_A,
        Button_Triangle = Button_B,
    }

    public interface IAutomationKit
    {
        string Name
        {
            get;
        }

        string ExePath
        {
            get;
        }

        AutomationKitType Type
        {
            get;
        }

        AutomationKitStatus Status
        {
            get;
            set;
        }

        uint Warnings
        {
            get;
        }

        uint Errors
        {
            get;
        }

        uint FatalErrors
        {
            get;
        }

        bool Connect();

        void Disconnect();

        void Reset();

        bool IsAlive();

        bool IsRunning(WorkManager.BuildConfig buildConfig);

        string Running();

        void ClearPadState(uint controllerPort);

        void SelectButtonPress(uint controllerPort, AutomationKitButton button);

        void SelectLeftStick(uint controllerPort, double x, double y);

        void SelectRightStick(uint controllerPort, double x, double y);

        bool SetPadState(uint controllerPort, bool oneFrame);
        
        uint IPAdddress();

        void TakeScreenshot(string filename);

        void LogOutputToFile(string folder, string filename);

        void Reboot();

        void Launch(WorkManager.BuildConfig buildConfig);

        void WriteLine(string text);

        void SaveCrashDump(string filename, bool takePicture);
    }

    public abstract class AutomationKit : IAutomationKit
    {
        /// <summary>
        /// Object to help stop DevIL being used across multiple threads at the same time
        /// </summary>
        protected static object devilLock = new object();

        public AutoResetEvent ExceptionEvent = new AutoResetEvent(false);
        public AutoResetEvent AssertEvent = new AutoResetEvent(false);

        protected string exePath;
        protected AssertInfo assertDetails;
        protected bool connected = false;

        protected uint warnings;
        protected uint errors;
        protected uint fatalErrors;

        private string name;
        private AutomationKitType type;
        private AutomationKitStatus status;

        private SimpleLogger logFile;

        protected object debugOutputLogFileLock = new object();
        protected SimpleLogger debugOutputLogFile;

        protected AutomationKit(string name, AutomationKitType type)
        {
            this.name = name;
            this.type = type;
            this.status = AutomationKitStatus.Available;
        }

        public AssertInfo AssertDetails
        {
            get
            {
                return this.assertDetails;
            }
        }

        public uint Warnings
        {
            get
            {
                return this.warnings;
            }
        }

        public uint Errors
        {
            get
            {
                return this.errors;
            }
        }

        public uint FatalErrors
        {
            get
            {
                return this.fatalErrors;
            }
        }

        public string Name
        {
            get
            {
                return this.name;
            }
        }

        public string ExePath
        {
            get
            {
                return this.exePath;
            }
        }

        public AutomationKitType Type
        {
            get
            {
                return this.type;
            }
        }

        public AutomationKitStatus Status
        {
            get
            {
                return this.status;
            }

            set
            {
                this.status = value;
            }
        }

        public static IAutomationKit Create(AutomationKitType type, string name)
        {
            switch (type)
            {
                case AutomationKitType.Xbox360:
                    return new AutomationKit360(name);

                case AutomationKitType.PS3:
                    return new AutomationKitPS3(name);

                ////case AutomationKitType.PC:
                ////    return new AutomationKitPC(name);
            }

            return null;
        }

        public void WriteLine(string text)
        {
            if (logFile == null)
            {
                logFile = new SimpleLogger(System.IO.Path.Combine(WorkManager.ReportFolder, "kit-logs"), this.Name + ".txt");
            }

            logFile.WriteLine(text);

            WorkManager.MasterLog.WriteLine(text);
        }

        public void LogOutputToFile(string folder, string filename)
        {
            if (folder != null && filename != null)
            {
                lock (this.debugOutputLogFileLock)
                {
                    if (this.debugOutputLogFile != null)
                    {
                        this.debugOutputLogFile.Close();
                        this.debugOutputLogFile = null;
                    }

                    string logPath = System.IO.Path.Combine(folder, filename);

                    if (System.IO.File.Exists(logPath))
                    {
                        System.IO.File.Delete(logPath);
                    }

                    this.debugOutputLogFile = new SimpleLogger(folder, filename, false);
                }
                
                this.warnings = 0;
                this.errors = 0;
                this.fatalErrors = 0;
            }
            else
            {
                lock (this.debugOutputLogFileLock)
                {
                    this.debugOutputLogFile.Close();
                    this.debugOutputLogFile = null;
                }
            }
        }

        public void AssertFired(string message, string condition, string function, string file, uint line)
        {
            this.assertDetails = new AssertInfo();

            this.WriteLine(string.Format("ASSERT: {0}: msg({1}) cond({2}) func({3}) file({4}) line({5}) ", this.Name, message, condition, function, file, line));
            
            this.assertDetails.Message = message;
            this.assertDetails.Condition = condition;
            this.assertDetails.Function = function;
            this.assertDetails.File = file;
            this.assertDetails.Line = line;

            this.AssertEvent.Set();
        }

        public abstract bool Connect();

        public abstract void Disconnect();

        public abstract void Reset();

        public abstract bool IsAlive();

        public abstract bool IsRunning(WorkManager.BuildConfig buildConfig);

        public abstract string Running();

        public abstract uint IPAdddress();

        public abstract void TakeScreenshot(string filename);

        public abstract void Reboot();

        public abstract void Launch(WorkManager.BuildConfig buildConfig);

        public abstract void SaveCrashDump(string filename, bool takePicture);

        public abstract void ClearPadState(uint controllerPort);

        public abstract void SelectButtonPress(uint controllerPort, AutomationKitButton button);

        public abstract void SelectLeftStick(uint controllerPort, double x, double y);

        public abstract void SelectRightStick(uint controllerPort, double x, double y);

        public abstract bool SetPadState(uint controllerPort, bool oneFrame);

        public class AssertInfo
        {
            public string Message
            {
                get;
                set;
            }

            public string Condition
            {
                get;
                set;
            }

            public string Function
            {
                get;
                set;
            }

            public string File
            {
                get;
                set;
            }

            public uint Line
            {
                get;
                set;
            }
        }
    }
}
