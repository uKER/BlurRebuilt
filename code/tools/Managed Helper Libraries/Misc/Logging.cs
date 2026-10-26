//-----------------------------------------------------------------------
// <copyright file="Logging.cs" company="Bizarre Creations">
// Copywright Bizarre Creations 2009
// </copyright>
//-----------------------------------------------------------------------

namespace Bizarre.Misc.LogSystem
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    public class DailyLogger : SimpleLogger
    {
        private DateTime lastTime;

        public DailyLogger(string directory, string filename)
            : base(directory, filename)
        {
        }

        public DailyLogger(string directory, string filename, bool echoToConsole)
            : base(directory, filename, echoToConsole)
        {
        }

        public override void WriteLine(string txt)
        {
            if (this.OutputToConsole)
            {
                System.Diagnostics.Debug.WriteLine(txt);
                System.Console.WriteLine(txt);
            }

            DateTime now = DateTime.Now;

            // Have we changed day and need to start a new log?
            if (now.Year != this.lastTime.Year || now.Month != this.lastTime.Month || now.Day != this.lastTime.Day)
            {
                this.Close();
                this.CreateLogfile();
            }

            System.Diagnostics.Debug.Assert(this.LogFile != null, "Check log file has been created first");
            this.LogFile.WriteLine(txt);
            this.LogFile.Flush();
        }

        protected override void CreateLogfile()
        {
            this.lastTime = DateTime.Now;

            int tryCount = 0;
            while (true)
            {
                string filename = String.Format("{0:0000}.{1:00}.{2:00}", this.lastTime.Year, this.lastTime.Month, this.lastTime.Day);

                if (tryCount > 0)
                {
                    filename += "." + tryCount;
                }

                filename += " - " + this.Filename + ".log";

                if (!System.IO.File.Exists(this.Directory + "\\" + filename))
                {
                    base.CreateLogfile(filename);
                    break;
                }

                tryCount++;
            }
        }
    }
}
