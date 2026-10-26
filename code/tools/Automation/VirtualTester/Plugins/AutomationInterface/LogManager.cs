using System;
using System.Diagnostics;
using System.IO;

namespace Bizarre.Automation
{
    // Base log class
    // Allow us to write to files
    // Allow us to register callbacks when messages are added (so we can do other stuff)
    // Create folder for log to live in
    public class Logging
    {
        //public delegate void AddMessageHandler(System.Drawing.Color colour, string message);

        //protected event AddMessageHandler AddMessageEvent;

        protected bool timestamp = false;

        // This little guy is used to stop multiple things writing at the same time
        private readonly Object lockingObject = new Object();

        protected StreamWriter outputFile;

        protected Logging(string filename, bool append, bool timestamp)
        {
            string fullpath = System.IO.Path.GetDirectoryName(filename);

            EnsureFolderExists(fullpath);

            this.timestamp = timestamp;
            this.outputFile = new StreamWriter(filename, append, System.Text.Encoding.Default);

            if (timestamp)
            {
                DateTime now = DateTime.Now;

                outputFile.WriteLine(String.Format("[{0}.{1:00}.{2:00}.{3:00}{4:00}.{5:00}] Start Logging", now.Year, now.Month, now.Day, now.Hour, now.Minute, now.Second));
            }
        }

        //public void AddMessageCallback(AddMessageHandler handler)
        //{
        //    AddMessageEvent += handler;
        //}

        protected void Write(string message)
        {
            lock (lockingObject)
            {
                outputFile.Write(message);
                outputFile.Flush();
            }
        }

        protected void WriteLine(string message)
        {
            lock (lockingObject)
            {
                if (timestamp)
                {
                    DateTime now = DateTime.Now;

                    message = String.Format("[{0:00}:{1:00}:{2:00}] ", now.Hour, now.Minute, now.Second) + message;
                }

                outputFile.WriteLine(message);
                outputFile.Flush();

                //if (AddMessageEvent != null)
                //{
                //    AddMessageEvent(System.Drawing.Color.Black, message + "\r\n");
                //}
            }
        }

        //protected void WriteLine(System.Drawing.Color colour, string message)
        //{
        //    lock (lockingObject)
        //    {
        //        if (timestamp)
        //        {
        //            DateTime now = DateTime.Now;

        //            message = String.Format("[{0:00}:{1:00}:{2:00}] ", now.Hour, now.Minute, now.Second) + message;
        //        }

        //        outputFile.WriteLine(message);
        //        outputFile.Flush();

        //        if (AddMessageEvent != null)
        //        {
        //            AddMessageEvent(colour, message + "\r\n");
        //        }
        //    }
        //}

        protected void EnsureFolderExists(string folder)
        {
            if (folder.Length > 0 && !System.IO.Directory.Exists(folder))
            {
                // Recursively check that the parent folder is here
                EnsureFolderExists(System.IO.Directory.GetParent(folder).FullName);

                System.IO.Directory.CreateDirectory(folder);
            }
        }
    }

    public class CustomLogging : Logging
    {
        public CustomLogging(string filename, bool append, bool timestamp)
            : base(filename, append, timestamp)
        {
        }

        public new void Write(string message)
        {
            base.Write(message);
        }

        public new void WriteLine(string message)
        {
            base.WriteLine(message);
        }
    }


    /// <summary>
    /// Base abstract class that performs the writing of data to a file.
    /// </summary>
    public abstract class Logger : IDisposable
    {
        protected StreamWriter sw;
        private readonly Object lockObj = new Object();
        private Boolean disposed = false;
        private Boolean outputToConsole = false;

        /// <summary>
        /// Gets a value indicating whether this instance is initialised.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if this instance is initialised; otherwise, <c>false</c>.
        /// </value>
        public Boolean IsInitialised { get { return (this.sw != null); } }

        protected Logger()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="T:Logger"/> class.
        /// </summary>
        /// <param name="baseLogLocation">The base log location.</param>
        /// <param name="fileName">Name of the file.</param>
        /// <param name="append">if set to <c>true</c> append.</param>
        protected Logger(String baseLogLocation, String fileName, Boolean append, Boolean outputToConsole)
        {
            if (!Directory.Exists(baseLogLocation))
            {
                Directory.CreateDirectory(baseLogLocation);
            }

            // Make sure the log location ends with a trailing "\"
            if (!baseLogLocation.EndsWith(@"\"))
            {
                baseLogLocation = baseLogLocation + @"\";
            }

            String path = baseLogLocation + fileName;

            // Write out as default ANSI... this makes it so goofy characters  (like ó) will display properly when Excel opens a .csv file.
            this.sw = new StreamWriter(path, append, System.Text.Encoding.Default);

            this.outputToConsole = outputToConsole;
        }

        /// <summary>
        /// Writes the specified entry.
        /// </summary>
        /// <param name="entry">The entry.</param>
        protected void Write(String entry)
        {
            if (this.disposed)
            {
                throw new ObjectDisposedException(this.ToString());
            }

            lock (this.lockObj)
            {
                //Console.WriteLine(entry); // TODO: Is this too much for the console?
                System.Diagnostics.Debug.WriteLine(entry);

                this.sw.WriteLine(entry);
                this.sw.Flush();
            }
        }

        #region Dispose Pattern

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                if (disposing)
                {
                    if (this.sw != null)
                    {
                        sw.Dispose();
                    }
                }
            }

            this.disposed = true;
        }
        /// <summary>
        /// Releases unmanaged resources and performs other cleanup operations before the
        /// <see cref="T:Microsoft.PGR4.Logger"/> is reclaimed by garbage collection.
        /// </summary>
        ~Logger()
        {
            this.Dispose(false);
        }

        #endregion // Dispose Pattern
    }

    /// <summary>
    /// Specific logging class for reporting job results.
    /// </summary>
    public sealed class ResultsLog : Logger
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="T:ResultsLog"/> class.
        /// </summary>
        /// <param name="baseLogLocation">The base log location.</param>
        /// <param name="columnHeaders">The column headers.</param>
        public ResultsLog(String baseLogLocation, String columnHeaders)
            : base(baseLogLocation, "Results.csv", false, false) // Don't append for Results Logger
        {
            base.Write(columnHeaders);
        }

        /// <summary>
        /// Logs the result.
        /// </summary>
        /// <param name="ID">The ID.</param>
        /// <param name="jobName">Name of the job.</param>
        /// <param name="jobResult">The job result.</param>
        //public void LogResult(Int32 ID, String jobName, JobResult jobResult)
        //{
        //    base.Write(String.Format("{0},{1},{2}", ID, jobName, jobResult.ToString()));
        //}
    }

    /// <summary>
    /// Specific logging class for reporting errors.
    /// </summary>
    public sealed class ErrorLog : Logger
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="T:ErrorLog"/> class.
        /// </summary>
        /// <param name="baseLogLocation">The base log location.</param>
        public ErrorLog(String baseLogLocation)
            : base(baseLogLocation, "ErrorLog.txt", false, true) { } // Don't append for Error Logger

        /// <summary>
        /// Writes the error.
        /// </summary>
        /// <param name="errorMessage">The error message.</param>
        public void WriteError(String errorMessage)
        {
            base.Write(errorMessage);
        }
    }

    /// <summary>
    /// Specific logging class for reporting information.
    /// </summary>
    public sealed class InfoLog : Logger
    {
        /// <summary>
        /// Priority levels to determine if the Info Log should report the given entry
        /// </summary>
        public enum PriorityLevel
        {
            High = 1,
            Medium = 2,
            Low = 3
        }

        private InfoLog.PriorityLevel minPriorityLevel;
        /// <summary>
        /// Gets or sets the min priority level.
        /// </summary>
        /// <value>The min priority level.</value>
        public InfoLog.PriorityLevel MinPriorityLevel { get { return this.minPriorityLevel; } set { this.minPriorityLevel = value; } }

        /// <summary>
        /// Initializes a new instance of the <see cref="T:InfoLog"/> class.
        /// </summary>
        /// <param name="baseLogLocation">The base log location.</param>
        public InfoLog(String baseLogLocation)
            : base(baseLogLocation, "InfoLog.txt", true, true) // Append for Info Logger
        {
            this.minPriorityLevel = PriorityLevel.Low;
        }

        /// <summary>
        /// Writes the line.
        /// </summary>
        /// <param name="entry">The entry.</param>
        /// <param name="priorityLevel">The priority level.</param>
        public void WriteLine(String entry, InfoLog.PriorityLevel priorityLevel)
        {
            // Only log entries that are at least as important as our minPriorityLevel
            if (priorityLevel <= this.minPriorityLevel)
            {
                base.Write(String.Format("{0}: {1}", DateTime.Now.ToString("yyyy:MM:dd-hh:mm:ss"), entry)); // Prepend the date/time of the entry
            }
        }
    }

    /// <summary>
    /// Specific logging class for reporting data in an XML format
    /// </summary>
    public sealed class XMLLog : Logger
    {
        private Int32 row;
        private Int32 cell;

        /// <summary>
        /// Initializes a new instance of the <see cref="T:XMLLog"/> class.
        /// </summary>
        /// <param name="baseLogLocation">The base log location.</param>
        /// <param name="fileName">Name of the file.</param>
        /// <param name="append">if set to <c>true</c> append.</param>
        public XMLLog(String baseLogLocation, String fileName, Boolean append)
            : base(baseLogLocation, fileName, append, false)
        {
            this.row = 1;
            this.cell = 1;
        }

        /// <summary>
        /// Writes the line.
        /// </summary>
        /// <param name="entry">The entry.</param>
        public void WriteLine(String entry)
        {
            base.Write(entry);
        }

        /// <summary>
        /// Writes the initial header.
        /// </summary>
        public void WriteInitialHeader()
        {
            base.Write("<?xml version=\"1.0\"?>");
            base.Write("<?mso-application progid=\"Excel.Sheet\"?>");
            base.Write("<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\" xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\" xmlns:html=\"http://www.w3.org/TR/REC-html40\">");
            base.Write("");
        }

        /// <summary>
        /// Starts the sheet.
        /// </summary>
        /// <param name="sheetName">Name of the sheet.</param>
        public void StartSheet(String sheetName)
        {
            base.Write(String.Format("<Worksheet ss:Name=\"{0}\">", sheetName));
            base.Write("<Table ss:DefaultRowHeight=\"16.000000\">");
            this.row = 1;
            this.cell = 1;
        }

        /// <summary>
        /// Ends the sheet.
        /// </summary>
        /// <param name="special">The special.</param>
        public void EndSheet(String special)
        {
            base.Write("</Table>");
            if (special.Length > 0)
            {
                base.Write(special);
            }
            base.Write("</Worksheet>");
        }

        /// <summary>
        /// Starts the row.
        /// </summary>
        /// <param name="extra">The extra.</param>
        public void StartRow(String extra)
        {
            if (extra.Length > 0)
            {
                base.Write(String.Format("<Row ss:Index=\"{0}\" {1}>", row, extra));
            }
            else
            {
                base.Write(String.Format("<Row ss:Index=\"{0}\">", row));
            }
        }

        /// <summary>
        /// Ends the row.
        /// </summary>
        public void EndRow()
        {
            base.Write("</Row>");
            this.row++;
            this.cell = 1;
        }

        /// <summary>
        /// Starts the cell.
        /// </summary>
        /// <param name="extra">The extra.</param>
        public void StartCell(String extra)
        {
            if (extra.Length > 0)
            {
                base.Write(String.Format("<Cell ss:Index=\"{0}\" {1}>", cell, extra));
            }
            else
            {
                base.Write(String.Format("<Cell ss:Index=\"{0}\">", cell));
            }
        }

        /// <summary>
        /// Ends the cell.
        /// </summary>
        public void EndCell()
        {
            base.Write("</Cell>");
            this.cell++;
        }
    }

    /// <summary>
    /// Simple implementation of the Logger abstract class.
    /// </summary>
    public class CustomLog : Logger
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="T:CustomLog"/> class.
        /// </summary>
        /// <param name="baseLogLocation">The base log location.</param>
        /// <param name="fileName">Name of the file.</param>
        /// <param name="append">if set to <c>true</c> append.</param>
        public CustomLog(String baseLogLocation, String fileName, Boolean append, Boolean outputToConsole)
            : base(baseLogLocation, fileName, append, outputToConsole) { }

        /// <summary>
        /// Writes the line.
        /// </summary>
        /// <param name="entry">The entry.</param>
        public void WriteLine(String entry)
        {
            base.Write(entry);
        }
    }
}