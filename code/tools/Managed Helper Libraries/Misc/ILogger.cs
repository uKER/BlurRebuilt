//-----------------------------------------------------------------------
// <copyright file="ILogger.cs" company="Bizarre Creations">
// Copywright Bizarre Creations 2009
// </copyright>
//-----------------------------------------------------------------------

namespace Bizarre.Misc.LogSystem
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    public interface ILogger
    {
        void Close();

        void WriteLine(string text);
    }

    public class SimpleLogger : ILogger
    {
        private string logName;
        private string logDirectory;
        private bool outputToConsole;

        private System.IO.StreamWriter logFile;

        private object lockObject = new object();

        protected SimpleLogger()
        {
        }

        public SimpleLogger(string directory, string filename)
        {
            this.logDirectory = directory;
            this.logName = filename;

            this.outputToConsole = false;

            this.CreateLogfile();
        }

        public SimpleLogger(string directory, string filename, bool echoToConsole)
        {
            this.logDirectory = directory;
            this.logName = filename;

            this.outputToConsole = echoToConsole;

            this.CreateLogfile();
        }

        public string Filename
        {
            get
            {
                return this.logName;
            }
        }

        public string Directory
        {
            get
            {
                return this.logDirectory;
            }
        }

        protected System.IO.StreamWriter LogFile
        {
            get
            {
                return this.logFile;
            }
        }

        protected bool OutputToConsole
        {
            get
            {
                return this.outputToConsole;
            }
        }

        public void Close()
        {
            lock (lockObject)
            {
                if (this.logFile != null)
                {
                    this.logFile.Close();
                    this.logFile = null;
                }
            }
        }

        public virtual void WriteLine(string txt)
        {
            lock (lockObject)
            {
                if (this.outputToConsole)
                {
                    System.Diagnostics.Debug.WriteLine(txt);
                    System.Console.WriteLine(txt);
                }

                System.Diagnostics.Debug.Assert(this.logFile != null, "Check log file has been created first");

                if (this.logFile != null && this.logFile.BaseStream.CanWrite)
                {
                    this.logFile.WriteLine(String.Format("[{0:00}:{1:00}:{2:00}] ", DateTime.Now.Hour, DateTime.Now.Minute, DateTime.Now.Second) + txt);
                }
            }
        }

        protected virtual void CreateLogfile()
        {
            if (!System.IO.Directory.Exists(this.logDirectory))
            {
                System.IO.Directory.CreateDirectory(this.logDirectory);
            }

            string fullPath = System.IO.Path.Combine(this.logDirectory, this.Filename);

            System.Diagnostics.Debug.Assert(System.IO.File.Exists(fullPath) == false, "Ensure file doesn't already exist");

            this.logFile = new System.IO.StreamWriter(fullPath);
            this.logFile.AutoFlush = true;
        }

        protected virtual void CreateLogfile(string filename)
        {
            this.logName = filename;

            if (!System.IO.Directory.Exists(this.logDirectory))
            {
                System.IO.Directory.CreateDirectory(this.logDirectory);
            }

            string fullPath = System.IO.Path.Combine(this.logDirectory, this.logName);

            System.Diagnostics.Debug.Assert(System.IO.File.Exists(fullPath) == false, "Ensure file doesn't already exist");

            this.logFile = new System.IO.StreamWriter(fullPath);
            this.logFile.AutoFlush = true;
        }
    }
}
