//-----------------------------------------------------------------------
// <copyright file="WorkManager.cs" company="Bizarre Creations">
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
    using System.Xml;
    using System.Xml.Linq;

    using Bizarre.Misc.ExcelXML;
    using Bizarre.Misc.LogSystem;

    using AutomationHelper;
    
    using Bizarre.Misc;

    public class WorkManager
    {
        /// <summary>
        /// This lock object will be used to control all thread access to manager variables
        /// </summary>
        private static readonly object lockObj = new object();

        /// <summary>
        /// Controls whether we spit out verbose debug information about the automation process
        /// </summary>
        private static bool verbose = false;

        /// <summary>
        /// Controls whether we stop running the automation if we hit a crash, such that it can be debugged
        /// </summary>
        private static bool stopOnCrash = false;

        private static bool stopOnAssert = false;

        private static string emailConfig = null;

        private static string automationTitle = null;

        /// <summary>
        /// Controls whether we do jobs even if they have dependencies (and even if they fail)
        /// </summary>
        private static bool ignoreDependencies = false;

        private static XmlDocument config;

        /// <summary>
        /// Holds the path to the output folder for this run
        /// </summary>
        private static string reportFolder = string.Empty;

        private static XtHelper xthelper;

        private static List<BuildConfig> buildConfigs = new List<BuildConfig>();

        private static List<uint> picNMix = null;
        private static Dictionary<string, uint> picNMixCounts = null;
        
        /// <summary>
        /// Holds the list of work to do
        /// </summary>
        private static List<IJob> workList = new List<IJob>();

        /// <summary>
        /// A reference to the server that manages which kits are available to which jobs
        /// </summary>
        private static KitServer kitServer;

        /// <summary>
        /// Holds the time taken to process all the jobs
        /// </summary>
        private static TimeSpan duration;

        /// <summary>
        /// Holds a value which allows us to control the lowest job index we process
        /// </summary>
        private static uint jobStartIndex = 0;
        
        /// <summary>
        /// Holds a value which allows us to control the highest job index we process
        /// </summary>
        private static uint jobEndIndex = uint.MaxValue;

        /// <summary>
        /// The time when we started processing the jobs
        /// </summary>
        private static DateTime startTime;

        private static Bizarre.Misc.LogSystem.SimpleLogger masterLog;

        private static string masterVersion = string.Empty;
                
        private static int passed = 0;
        private static int failed = 0;

        public static List<TestSuite> TestSuites = new List<TestSuite>();

        public static KitServer KitServer
        {
            get
            {
                return kitServer;
            }
        }

        public static string MasterVersion
        {
            get
            {
                return masterVersion;
            }
        }

        public static bool StopOnCrash
        {
            get
            {
                return stopOnCrash;
            }
        }

        public static bool StopOnAssert
        {
            get
            {
                return stopOnAssert;
            }
        }

        public static XmlDocument Config
        {
            get
            {
                return config;
            }
        }

        public static XtHelper XtHelper
        {
            get
            {
                return xthelper;
            }
        }

        public static bool Verbose
        {
            get { return verbose; }
        }

        public static int TotalJobs
        {
            get { return workList.Count; }
        }

        /// <summary>
        /// Gets the number of jobs that passed
        /// </summary>
        public static int Passed
        {
            get { return passed; }
        }

        /// <summary>
        /// Gets the number of jobs that failed
        /// </summary>
        public static int Failed
        {
            get { return failed; }
        }

        /// <summary>
        /// Gets how long it took to process the jobs
        /// </summary>
        public static TimeSpan Duration
        {
            get { return duration; }
        }

        /// <summary>
        /// Gets the path to the folder we want to put all output from this run into
        /// </summary>
        public static string ReportFolder
        {
            get
            {
                return reportFolder;
            }
        }

        public static List<BuildConfig> BuildConfigs
        {
            get
            {
                return buildConfigs;
            }
        }

        public static SimpleLogger MasterLog
        {
            get
            {
                return masterLog;
            }
        }

        public static List<IJob> Jobs
        {
            get
            {
                return workList;
            }
        }

        /// <summary>
        /// Initialise the list of kits available to do work for us (we'll still need to check them to see if they work)
        /// </summary>
        /// <param name="title">The name of the automation run</param>
        /// <param name="args">An array of names</param>
        /// <returns>Returns whether we sucessfully initialised the WorkManager</returns>
        public static bool Initialise(string title, string[] args)
        {
            automationTitle = title;

            PreParseCommandline(args);

            string existingReportFolder = reportFolder;

            //// specify report folder
            DateTime now = DateTime.Now;
            if (existingReportFolder == null)
            {
                reportFolder = string.Format(@"{0}\{1}.{2:00}.{3:00} - {4:00}.{5:00}", title, now.Year, now.Month, now.Day, now.Hour, now.Minute);
            }
            else
            {
                reportFolder = System.IO.Path.Combine(existingReportFolder, string.Format(@"{0}\{1}.{2:00}.{3:00} - {4:00}.{5:00}", title, now.Year, now.Month, now.Day, now.Hour, now.Minute));
            }

            // Ensure the directory doesn't exist already and make a unique one if it does
            if (System.IO.Directory.Exists(reportFolder))
            {
                int index = 1;
                while (true)
                {
                    if (existingReportFolder == null)
                    {
                        reportFolder = string.Format(@"{0}\{1}.{2:00}.{3:00} - {4:00}.{5:00}({6})", title, now.Year, now.Month, now.Day, now.Hour, now.Minute, index);
                    }
                    else
                    {
                        reportFolder = System.IO.Path.Combine(existingReportFolder, string.Format(@"{0}\{1}.{2:00}.{3:00} - {4:00}.{5:00}({6})", title, now.Year, now.Month, now.Day, now.Hour, now.Minute, index));
                    }
                    
                    if (!System.IO.Directory.Exists(reportFolder))
                    {
                        break;
                    }
                }
            }

            masterVersion = reportFolder.Substring(title.Length + 1);

            // Create global log to track everything
            masterLog = new SimpleLogger(reportFolder, "log.txt", true);

            kitServer = new KitServer(args);

            // Process any command line arguments WorkManager is directly interested in
            if (!ParseCommandline(args))
                return false;

            return true;
        }

        public static bool PickNMixValid
        {
            get
            {
                return picNMix != null;
            }
        }

        public static bool PassPickNMix(string type, uint id)
        {
            // We only limit categories when Pic n Mix file found AND we've selected some items
            if ((picNMix != null) && picNMixCounts.ContainsKey(type))
            {
                return picNMix.Contains(id);
            }

            return true;
        }

        /// <summary>
        /// Parse a config file passed in via the command line
        /// </summary>
        /// <param name="configFilename">Name of the file we are to parse</param>
        private static void ParseConfigFile(string configFilename)
        {
            config = new XmlDocument();

            config.Load(configFilename);

            XmlNodeList recs = config.GetElementsByTagName("build");

            if (recs.Count > 0)
            {
                BuildConfig bc = new BuildConfig();

                foreach (XmlNode r in recs)
                {
                    XmlAttribute exe360 = r.Attributes["XBOX_360"];
                    XmlAttribute exePS3 = r.Attributes["PS3"];
                    XmlAttribute exeWIN32 = r.Attributes["WIN32"];
                    XmlAttribute exeWIN64 = r.Attributes["WIN64"];
                    XmlAttribute dataFolder = r.Attributes["DATA"];

                    if (exe360 != null)
                    {
                        bc.Platform360 = exe360.Value;
                    }

                    if (exePS3 != null)
                    {
                        bc.PlatformPS3 = exePS3.Value;

                        if (dataFolder != null)
                        {
                            bc.PlatformPS3_Folder = dataFolder.Value;
                        }
                    }

                    if (exeWIN32 != null)
                    {
                        bc.PlatformWin32 = exeWIN32.Value;
                        
                        if (dataFolder != null)
                        {
                            bc.PlatformWin32_Folder = dataFolder.Value;
                        }
                    }

                    if (exeWIN64 != null)
                    {
                        bc.PlatformWin64 = exeWIN64.Value;
                        
                        if (dataFolder != null)
                        {
                            bc.PlatformWin64_Folder = dataFolder.Value;
                        }
                    }
                }
                
                buildConfigs.Add(bc);
            }
        }

        private static void PreParseCommandline(string[] args)
        {
            Arguments argParser = new Arguments(args);

            if (argParser["reports"] != null)
            {
                reportFolder = argParser["reports"];
            }

            if (argParser["verbose"] == "true")
            {
                verbose = true;
            }
        }

        private static bool ParseCommandline(string[] args)
        {
            Arguments argParser = new Arguments(args);

            if (argParser["soc"] == "true" || argParser["stoponcrash"] == "true")
            {
                stopOnCrash = true;
            }

            if (argParser["soa"] == "true" || argParser["stoponassert"] == "true")
            {
                stopOnAssert = true;
            }

            if (argParser["id"] == "true" || argParser["ignoredependencies"] == "true")
            {
                ignoreDependencies = true;
            }

            if (argParser["config"] != null)
            {
                ParseConfigFile(argParser["config"]);
            }

            if (argParser["email"] != null)
            {
                emailConfig = argParser["email"];

                // Check email file is actually readable
                if (!VerifyEmailXML())
                {
                    WorkManager.MasterLog.WriteLine("ERROR: Email file '" + emailConfig + "' is not readable");
                    return false;
                }
            }

            if (argParser["js"] != null)
            {
                uint result;

                if (uint.TryParse(argParser["js"], out result))
                {
                    jobStartIndex = result;
                    ignoreDependencies = true;
                }
            }
            
            if (argParser["je"] != null)
            {
                uint result;

                if (uint.TryParse(argParser["je"], out result))
                {
                    jobEndIndex = result;
                    ignoreDependencies = true;
                }
            }

            // Has someone specified a sorcery database to use?
            if (argParser["sorcery"] != null)
            {
                // Load in Sorcery data so we can use it to generate tests
                xthelper = new XtHelper();
                xthelper.Open(argParser["sorcery"]);
            }

            // Has someone specified a 'Pick n Mix' file to use
            if (argParser["pnm"] != null)
            {
                XmlDocument doc = new XmlDocument();

                string pnmFilename = argParser["pnm"];

                if (System.IO.File.Exists(pnmFilename))
                {
                    doc.Load(argParser["pnm"]);

                    // Tag records in use
                    XmlNodeList recs = doc.GetElementsByTagName("Record");

                    if (recs.Count > 0)
                    {
                        picNMix = new List<uint>();
                        picNMixCounts = new Dictionary<string, uint>();

                        foreach (XmlNode r in recs)
                        {
                            uint id = uint.Parse(r.Attributes["id"].Value);
                            string type = r.Attributes["type"].Value;

                            picNMix.Add(id);

                            if (picNMixCounts.ContainsKey(type))
                            {
                                picNMixCounts[type] += 1;
                            }
                            else
                            {
                                picNMixCounts[type] = 1;
                            }
                        }
                    }
                }
                else
                {
                    WorkManager.MasterLog.WriteLine("ERROR: Could not find Pick 'n Mix file - '" + pnmFilename + "'");
                }
            }

            return true;
        }

        /// <summary>
        /// This allows us to set a range in which only jobs with an index in this range will get processed, all others will be skipped
        /// </summary>
        /// <param name="min">The minimum value a job needs for an index before it is processed</param>
        /// <param name="max">The maximum value a job needs for an index before it is skipped</param>
        public static void SetJobRange(uint min, uint max)
        {
            jobStartIndex = min;
            jobEndIndex = max;
        }

        /// <summary>
        /// Add a job to the work list
        /// </summary>
        /// <param name="newJob">Details of the job to add</param>
        public static void AddJob(IJob newJob)
        {
            lock (lockObj)
            {
                workList.Add(newJob);
            }
       }

        private static void EvaluateJobDependencies()
        {
            if (ignoreDependencies)
            {
                WorkManager.MasterLog.WriteLine("IgnoreDependencies Enabled - Skipping all dependency checking");
                return;
            }

            // Find all the jobs that have tags
            var taggedJobs = from j in WorkManager.Jobs where j.Tag != null select j;

            foreach (var j in WorkManager.Jobs)
            {
                // For each job that has dependencies
                if (j.Dependencies != null)
                {
                    foreach (string d in j.Dependencies)
                    {
                        if (taggedJobs.Count() > 0)
                        {
                            var djob = (taggedJobs != null) ? taggedJobs.FirstOrDefault(tj => tj.Tag == d) : null;

                            if (djob != null)
                            {
                                j.AddDependency(djob);
                                continue;
                            }
                        }

                        WorkManager.MasterLog.WriteLine(string.Format("Job({0} - {1}) has dependency on '{2}' that could not be found. Ignoring dependency.", j.ID, j.Name, d));
                    }
                }
            }
        }

        /// <summary>
        /// Process the jobs in the work queue on the available kits
        /// </summary>
        /// <param name="retryAttempts">Number of times we should retry any failed jobs before we're sure it was the job and not the existing state of the kit. We reset the kits before each retry too.</param>
        public static void Process()
        {
            // Run through all registered suites and create reports for them
            foreach (var ts in TestSuites)
            {
                ts.CreateJobs(ts.Context);
            }

            // Evaluate any dependencies between the jobs
            EvaluateJobDependencies();

            startTime = DateTime.Now;

            Bizarre.Artery.ArteryManager.StartupEx(false, "AutomationSystem", kitServer.TotalKits * 3);

            masterLog.WriteLine(string.Format("WorkManager:Process - Running {0} jobs.", workList.Count));

            // Run through the jobs
            lock (lockObj)
            {
                ////this.nextJobIndex = 0;
                List<ManualResetEvent> jobCompletionEvents = new List<ManualResetEvent>(workList.Count);

                IJob job;

                while ((job = GetNextJob()) != null)
                {
                    // Have we set a range of jobs to perform to limit how much we test? If so, only do those jobs
                    if ((job.ID < jobStartIndex) || (job.ID > jobEndIndex))
                    {
                        job.Result = JobResult.Skipped;
                        continue;
                    }

                    // TODO: Make the checkout exception (rename) and catch it
                    List<AutomationKit> kits = kitServer.LockKits(job.KitRequirements);

                    // Check enough kits are available to do this job (some require more than one)
                    if (kits != null)
                    {
                        ManualResetEvent jobEvent = new ManualResetEvent(false);
                        jobCompletionEvents.Add(jobEvent);

                        // Not using a custom thread pool because we're lazy... do this later... or not...
                        Thread jobThread = new Thread(new ParameterizedThreadStart(JobThreadProcess));

                        // Setting the thread name to be the name of our 1st kit.
                        jobThread.Name = kits[0].Name;

                        State stateInfo = new State(kitServer, job, jobEvent, kits);

                        // Before we start the new thread, make sure the ones that are done now are fully collected by the GC.
                        GC.Collect();
                        GC.WaitForPendingFinalizers();

                        jobThread.Start(stateInfo);
                    }
                    else
                    {
                        job.Result = JobResult.Failed;
                        job.FailReason = JobFailReason.NoKitsAvailable;
                        job.FailInfo = string.Format("Asked for '{0}' kits. Not enough available to run test", job.KitRequirements);
                    }
                }

                // 64 limit on WaitAll - do it manually
                foreach (ManualResetEvent currentMRE in jobCompletionEvents.ToArray())
                {
                    currentMRE.WaitOne();
                }

                passed = 0;
                failed = 0;
                foreach (IJob j in workList)
                {
                    switch (j.Result)
                    {
                        case JobResult.Passed:
                            passed++;
                            break;

                        case JobResult.Failed:
                            failed++;
                            break;
                    }
                }
            }

            // All done... how long did it take?
            duration = DateTime.Now - startTime;

            CreateReports();
        }

        private static void CreateReportForTestSuite(Workbook workbook, TestSuite ts)
        {
            //Workbook.CellStyle stylePass = workbook.FindStyle("ss:StylePass");
            //Workbook.CellStyle styleFail = workbook.FindStyle("ss:StyleFail");
            //Workbook.CellStyle styleSkipped = workbook.FindStyle("ss:StyleSkipped");

            TestSuite.AdditionalColumns additionalColumns = new TestSuite.AdditionalColumns();
            TestSuite.AdditionalColumns additionalPostColumns = new TestSuite.AdditionalColumns();

            ts.AddAdditionalColumns(additionalColumns);
            ts.AddAdditionalPostColumns(additionalPostColumns);

            Workbook.Worksheet ws = new Workbook.Worksheet(workbook, ts.Name);

            ts.AddHeader(ws, additionalColumns, additionalPostColumns);

            var jobsInSuite = from j in WorkManager.Jobs where j.TestSuite == ts.Id select j;

            foreach (IJob j in jobsInSuite)
            {
                ts.AddDataRow(j, ws, additionalColumns, additionalPostColumns);
            }

            ts.SetColumnWidths(ws, additionalColumns, additionalPostColumns);

            workbook.Worksheets.Add(ws);
        }
        
        private static string TimeSpanToString(TimeSpan ts)
        {
            string result = "";

            if (ts.Days > 0)
                result += string.Format("{0} {1} ", ts.Days, (ts.Days > 1 ? "days" : "day"));
            if (ts.Hours > 0)
                result += string.Format("{0} {1} ", ts.Hours, (ts.Hours > 1 ? "hours" : "hour"));
            if (ts.Minutes > 0)
                result += string.Format("{0} {1} ", ts.Minutes, (ts.Minutes > 1 ? "minutes" : "minute"));

            result += string.Format("{0} {1}", ts.Seconds, (ts.Seconds > 1 ? "seconds" : "second"));

            return result;
        }

        private static string CreateAssertsOverview()
        {
            var crashes = from j in WorkManager.Jobs
                          where j.Result == JobResult.Failed && j.FailReason == JobFailReason.Crashed
                          select j;

            var memLeaks = from j in WorkManager.Jobs
                          where j.Result == JobResult.Failed && j.FailReason == JobFailReason.MemoryLeak
                          select j;

            var logicProblems = from j in WorkManager.Jobs
                           where j.Result == JobResult.Failed && j.FailReason == JobFailReason.Logic
                           select j;

            var timedOut = from j in WorkManager.Jobs
                          where j.Result == JobResult.Failed && j.FailReason == JobFailReason.TimedOut
                          select j;

            // Asserts where a condition failed
            var asserts = from j in WorkManager.Jobs
                          where j.Result == JobResult.Failed && j.FailReason == JobFailReason.Asserted && (j.FailInfo as AutomationAssertException).Condition.Length > 0
                          group j by (j.FailInfo as AutomationAssertException).Condition.Split('\n')[0] into g
                          select new { Condition = g.Key, Asserts = g };

            // These were _FAIL asserts where no condition was checked
            var assertFails = from j in WorkManager.Jobs
                              where j.Result == JobResult.Failed && j.FailReason == JobFailReason.Asserted && (j.FailInfo as AutomationAssertException).Condition.Length == 0
                              group j by (j.FailInfo as AutomationAssertException).Message.Split('\n')[0] into g
                              select new { Message = g.Key, Asserts = g };

            if (asserts.Count() == 0 && assertFails.Count() == 0 && crashes.Count() == 0 && timedOut.Count() == 0 && memLeaks.Count() == 0 && logicProblems.Count() == 0)
                return "";

            string html = "Summary of asserts that fired:<br>";

            HtmlHelper htmlHelper = new HtmlHelper();

            htmlHelper.BeginTable(1);

            // Table header
            htmlHelper.BeginTableRow();
            htmlHelper.AddTableHeader("Condition/Message");
            htmlHelper.AddTableHeader("Count");
            htmlHelper.EndTableRow();

            if (crashes.Count() > 0)
            {
                htmlHelper.BeginTableRow();
                htmlHelper.AddTableData("Crashed");
                htmlHelper.AddTableData(crashes.Count().ToString());
                htmlHelper.EndTableRow();
            }

            if (memLeaks.Count() > 0)
            {
                uint maxSysLeakSize = 0;
                uint maxPhyLeakSize = 0;

                foreach (IJob ml in memLeaks)
                {
                    MemoryLeakInfo leak = (MemoryLeakInfo)ml.ResultsData;

                    if (leak.System > maxSysLeakSize)
                    {
                        maxSysLeakSize = leak.System;
                    }

                    if (leak.Physical > maxPhyLeakSize)
                    {
                        maxPhyLeakSize = leak.Physical;
                    }
                }

                htmlHelper.BeginTableRow();
                htmlHelper.AddTableData(string.Format("Memory Leak - System({0}) Physical({1})", Various.ConvertSizeToString(maxSysLeakSize), Various.ConvertSizeToString(maxPhyLeakSize)));
                htmlHelper.AddTableData(memLeaks.Count().ToString());
                htmlHelper.EndTableRow();
            }

            if (logicProblems.Count() > 0)
            {
                htmlHelper.BeginTableRow();
                htmlHelper.AddTableData("Logic problems");
                htmlHelper.AddTableData(logicProblems.Count().ToString());
                htmlHelper.EndTableRow();
            }

            if (timedOut.Count() > 0)
            {
                htmlHelper.BeginTableRow();
                htmlHelper.AddTableData("Timed Out");
                htmlHelper.AddTableData(timedOut.Count().ToString());
                htmlHelper.EndTableRow();
            }

            foreach (var ass in asserts)
            {
                htmlHelper.BeginTableRow();
                htmlHelper.AddTableData(ass.Condition);
                htmlHelper.AddTableData(ass.Asserts.Count().ToString());
                htmlHelper.EndTableRow();
            }
            
            foreach (var ass in assertFails)
            {
                htmlHelper.BeginTableRow();
                htmlHelper.AddTableData(ass.Message);
                htmlHelper.AddTableData(ass.Asserts.Count().ToString());
                htmlHelper.EndTableRow();
            }

            htmlHelper.EndTable();

            html += htmlHelper.Body;

            return html + "<br>";
        }

        private static bool VerifyEmailXML()
        {
            try
            {
                XDocument emailXml = XDocument.Load(emailConfig);

                return true;
            }
            catch (Exception ex)
            {
                WorkManager.MasterLog.WriteLine("VerifyEmailXML: Exception: " + ex.Message);
                return false;
            }
        }

        private static string ReplaceTokensInString(string originalString)
        {
            if (originalString == null)
                return null;

            // Values we put into the text based on tokens
            string fullPathToReport = System.IO.Path.GetFullPath(WorkManager.ReportFolder);
            string duration = TimeSpanToString(WorkManager.Duration);

            string assertsOverview = CreateAssertsOverview();

            string overviewHtml;
            CreateReportSheet_Overview(out overviewHtml);

            // Version, Date and Time of Build
            string version = "?.?.?.?";
            string date = "?/?/?";
            string time = "?:?";

            if (WorkManager.Jobs.Count > 0)
            {
                var jtemp = WorkManager.Jobs.Where(a => a.BuildVersion != string.Empty);

                if (jtemp.Count() > 0)
                {
                    IJob j = jtemp.First(a => a.BuildVersion != string.Empty);

                    if (j != null)
                    {
                        version = j.BuildVersion;
                        date = j.BuildDate;
                        time = j.BuildTime;
                    }
                }
            }

            // Do the replacing
            string outString = originalString.Replace("%DURATION%", duration);
            outString = outString.Replace("%ASSERTS%", assertsOverview);
            outString = outString.Replace("%RESULTS%", overviewHtml + "<br>");
            outString = outString.Replace("%PATH%", "<a href=\"file:///" + fullPathToReport + "\">" + fullPathToReport + "</a>");
            outString = outString.Replace("%TITLE%", automationTitle);
            outString = outString.Replace("%BUILDVERSION%", version);
            outString = outString.Replace("%BUILDDATE%", date);
            outString = outString.Replace("%BUILDTIME%", time);

            return outString;
        }

        private static void SendEmail(string reportFilename)
        {
            Mail mail = new Mail();

            XDocument emailXml = XDocument.Load(emailConfig);

            var xmlList = emailXml.Descendants();

            foreach (var x in xmlList)
            {
                switch (x.Name.LocalName.ToLower())
                {
                    case "to":
                        mail.AddRecipient(x.Value);
                        break;

                    case "subject":
                        mail.Subject = x.Value;
                        break;

                    case "body":
                        mail.Body = x.Value;
                        break;
                }
            }

            mail.Subject = ReplaceTokensInString(mail.Subject);
            mail.Body = ReplaceTokensInString(mail.Body);

            mail.Sender = "";
            
            mail.AddAttachment(reportFilename);

            mail.IsBodyHtml = true;

            mail.Send();
        }

        private static void CreateReports()
        {
            masterLog.WriteLine(String.Format("Time taken {0}.", WorkManager.Duration));

            masterLog.WriteLine("Job results:");
            masterLog.WriteLine("PASSED: " + WorkManager.Passed);
            masterLog.WriteLine("FAILED: " + WorkManager.Failed);
            masterLog.WriteLine("TOTAL : " + WorkManager.TotalJobs);

            if (WorkManager.Passed + WorkManager.Failed < WorkManager.TotalJobs)
            {
                masterLog.WriteLine("??????: " + (WorkManager.TotalJobs - (WorkManager.Passed + WorkManager.Failed)));
            }

            // Write out report in Excel XML workbook
            Workbook wb = new Workbook();

            // If we've done more than one suite then provide an overview page
            if (TestSuites.Count > 1)
            {
                CreateReportSheet_Overview(wb);
            }

            // Run through all registered suites and create reports for them
            foreach (var ts in TestSuites)
            {
                CreateReportForTestSuite(wb, ts);
            }

            // Write out default job result sheet
            CreateReportSheet_JobKits(wb);

            string reportFilename = System.IO.Path.Combine(WorkManager.ReportFolder, "results");

            string outputFilename = wb.Save(reportFilename);

            // Have we specified an email config file
            if (emailConfig != null && System.IO.File.Exists(emailConfig))
            {
                SendEmail(outputFilename);
            }
        }

        private static void CreateReportSheet_Overview(out string html)
        {
            html = "<table Border=\"1\">";
            
            // Table header
            html += "<tr><th>Test Suite</th><th>Results</th></tr>";

            foreach (var ts in TestSuites)
            {
                html += "<tr>";
                
                html += "<td>" + ts.Name + "</td>";

                var jobsInSuite = from j in WorkManager.Jobs where j.TestSuite == ts.Id orderby j.Result group j by j.Result into g select new { Result = g.Key, Jobs = g };

                string results = string.Empty;

                foreach (var r in jobsInSuite)
                {
                    if (results.Length > 0)
                    {
                        results += ", ";
                    }

                    results += string.Format("{0}({1})", r.Result, r.Jobs.Count());
                }

                html += "<td>" + results + "</td>";
                
                html += "</tr>";
            }

            html += "</table>";
        }

        private static void CreateReportSheet_Overview(Workbook wb)
        {
            Workbook.Worksheet ws = new Workbook.Worksheet(wb, "Overview");

            // Header
            Workbook.Worksheet.Row headerRow = ws.AddRow();

            headerRow.AddCell("Test Suite");
            headerRow.AddCell("Results");

            foreach (var ts in TestSuites)
            {
                Workbook.Worksheet.Row row = ws.AddRow();

                row.AddCell(ts.Name);

                var jobsInSuite = from j in WorkManager.Jobs where j.TestSuite == ts.Id orderby j.Result group j by j.Result into g select new { Result = g.Key, Jobs = g };

                string results = string.Empty;

                foreach (var r in jobsInSuite)
                {
                    if (results.Length > 0)
                    {
                        results += ", ";
                    }

                    results += string.Format("{0}({1})", r.Result, r.Jobs.Count());
                }

                row.AddCell(results);
            }

            ws.AddRow();
            ws.AddRow();

            if (WorkManager.Jobs.Count > 0 && WorkManager.Jobs.Count(a => a.BuildVersion != string.Empty) > 0)
            {
                var builds = WorkManager.Jobs.FindAll(a => a.BuildVersion != string.Empty).GroupBy(a => a.BuildVersion);

                foreach (var g in builds)
                {
                    var j = g.ElementAt(0);
                    
                    Workbook.Worksheet.Row row = ws.AddRow();
                    row.AddCell("Version");
                    row.AddCell(j.BuildVersion);
                    
                    row = ws.AddRow();
                    row.AddCell("Date");
                    row.AddCell(j.BuildDate);
                    
                    row = ws.AddRow();
                    row.AddCell("Time");
                    row.AddCell(j.BuildTime);
                    
                    row = ws.AddRow();
                    row.AddCell("Jobs Using");
                    row.AddCell(g.Count());

                    ws.AddRow();
                }
            }

            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 200 });
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 200 });

            wb.Worksheets.Add(ws);
        }

        private static void CreateReportSheet_JobKits(Workbook wb)
        {
            Workbook.CellStyle stylePass = wb.FindStyle("ss:StylePass");
            Workbook.CellStyle styleFail = wb.FindStyle("ss:StyleFail");
            Workbook.CellStyle styleSkipped = wb.FindStyle("ss:StyleSkipped");

            Workbook.CellStyle styleCentre = new Workbook.CellStyle();
            styleCentre.HorizontalAlignment = Workbook.eAlignment.Center;
            wb.Styles.Add(styleCentre);

            Workbook.Worksheet ws = new Workbook.Worksheet(wb, "Jobs");

            Workbook.Worksheet.Row headerRowT = ws.AddRow();
            
            // ID and Name
            headerRowT.AddCell("");
            headerRowT.AddCell("");

            Workbook.Worksheet.Row.Cell lc = headerRowT.AddCell("Log Output");
            lc.MergeAcross = 2;
            lc.Style = styleCentre;

            // Header
            Workbook.Worksheet.Row headerRow = ws.AddRow();

            headerRow.AddCell("ID");
            headerRow.AddCell("Kits");
            headerRow.AddCell("Warn");
            headerRow.AddCell("Error");
            headerRow.AddCell("Fatal");
            headerRow.AddCell("Name");
            headerRow.AddCell("Result");
            headerRow.AddCell("Comments");

            ws.FreezeRows = 2;

            // Add the jobs to the sheet
            foreach (IJob j in WorkManager.Jobs)
            {
                Workbook.Worksheet.Row row = ws.AddRow();

                row.AddCell(j.ID);
                row.AddCell(j.KitNames);

                row.AddCell(j.LogWarnings);
                row.AddCell(j.LogErrors);
                row.AddCell(j.LogFatalErrors);
                
                row.AddCell(j.Name);
                
                Workbook.Worksheet.Row.Cell resultCell = row.AddCell(j.Result.ToString());

                // Select a cell style based on the job result
                switch (j.Result)
                {
                    case JobResult.Passed:
                        resultCell.Style = stylePass;
                        break;

                    case JobResult.Failed:
                        resultCell.Style = styleFail;
                        break;

                    case JobResult.Skipped:
                        resultCell.Style = styleSkipped;
                        break;
                }

                resultCell.Comment = j.JobStage;

                if (j.Result == JobResult.Skipped || j.Result == JobResult.Failed)
                {
                    if (j.FailReason != JobFailReason.Unknown)
                    {
                        row.AddCell(j.FailReason.ToString());
                    }

                    if (j.FailInfo != null)
                    {
                        if (j.FailInfo is string)
                        {
                            row.AddCell(j.FailInfo);
                        }
                        else if (j.FailInfo is AutomationAssertException)
                        {
                            AutomationAssertException ex = j.FailInfo as AutomationAssertException;

                            if (ex.Message.Length > 0)
                            {
                                row.AddCell(ex.Message);
                            }

                            if (ex.Condition.Length > 0)
                            {
                                row.AddCell(ex.Condition);
                            }

                            if (ex.Function.Length > 0)
                            {
                                row.AddCell(ex.Function);
                            }

                            if (ex.File.Length > 0)
                            {
                                row.AddCell(ex.File + "(" + ex.Line + ")");
                            }
                        }
                    }
                }
            }

            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 50 });
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 100 });

            // Warn, Error, Fatal
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 30 });
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 30 });
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 30 });

            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 280 });
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 60 });
            ws.Columns.Add(new Workbook.Worksheet.Column() { Width = 80 });

            wb.Worksheets.Add(ws);
        }

        private static IJob GetNextJob()
        {
            lock (lockObj)
            {
                // TODO: We need to take into account any dependencies

                var jobs = from j in WorkManager.Jobs where j.Status == JobStatus.NotStarted select j;

                // Do all the non-dependant jobs first
                foreach (var j in jobs)
                {
                    if (j.Dependencies == null)
                    {
                        j.Status = JobStatus.InProgress;
                        return j;
                    }
                }
            }

            while (true)
            {
                lock (lockObj)
                {
                    var jobs = from j in WorkManager.Jobs where j.Status == JobStatus.NotStarted select j;

                    // Check there are still jobs left to process
                    if (jobs.Count() == 0)
                    {
                        return null;
                    }

                    // So if we got here it's only jobs with dependencies - let's look a little closer
                    foreach (var j in jobs)
                    {
                        bool dependencyIssue = false;

                        if (j.DependentJobs != null)
                        {
                            foreach (var d in j.DependentJobs)
                            {
                                if (d.Status == JobStatus.Completed)
                                {
                                    // Check if a job we're dependant on has completed, but not passed. If so, we get skipped
                                    if (d.Result != JobResult.Passed)
                                    {
                                        j.Status = JobStatus.Completed;
                                        j.Result = JobResult.Skipped;
                                        j.JobStage = string.Format("Job({0}) did not pass", d.ID);
                                        dependencyIssue = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    dependencyIssue = true;
                                    break;
                                }
                            }
                        }

                        // If all the dependencies are fine, we can run!
                        if (!dependencyIssue)
                        {
                            // Now we check that there's enough resources for us to run
                            int kitAvail = KitServer.KitAvailable(j.KitRequirements);

                            if (kitAvail == 1) // Enough kits to do job
                            {
                                j.Status = JobStatus.InProgress;
                                return j;
                            }
                            else if (kitAvail == -1)
                            {
                                j.Status = JobStatus.Completed;
                                j.Result = JobResult.Failed;
                                j.FailReason = JobFailReason.NoKitsAvailable;
                            }
                        }
                    }
                }

                Thread.Sleep(TimeSpan.FromSeconds(1));
            }
        }

        private static string ConvertKitListToString(List<AutomationKit> kits)
        {
            string output = string.Empty;

            foreach (AutomationKit kit in kits)
            {
                if (output.Length > 0)
                {
                    output += ", ";
                }

                output += kit.Name;
            }

            return output;
        }

        /// <summary>
        /// Build up a string which is a comma separated list of the names of the kits passed in
        /// </summary>
        /// <param name="kits">An array of kits to process into the string</param>
        /// <returns></returns>
        private static string KitsToString(List<AutomationKit> kits)
        {
            string names = string.Empty;

            foreach (var k in kits)
            {
                if (names.Length > 0)
                {
                    names += ", ";
                }

                names += k.Name;
            }

            return names;
        }

        private static void JobThreadProcess(object state)
        {
            DateTime startTime = DateTime.Now;

            State stateInfo = (State)state;

            // We try each job up to 2 times in case the game crashed for other reasons on the first try
            for (int retryCount = 0; retryCount < 2; retryCount++)
            {
                bool dontRetry = false;

                try
                {
                    stateInfo.Job.KitNames = KitsToString(stateInfo.Kits);

                    stateInfo.Job.Result = JobResult.NoResult;

                    foreach (var k in stateInfo.Kits)
                    {
                        k.WriteLine(String.Format("{0}: {1}: Trying ({2})", stateInfo.Job.ID, ConvertKitListToString(stateInfo.Kits), stateInfo.Job.Name));

                        k.LogOutputToFile(System.IO.Path.Combine(WorkManager.ReportFolder, "job-logs"), stateInfo.Job.ID + " - " + k.Name + " - " + stateInfo.Job.Name + ".txt");
                    }

                    dontRetry = stateInfo.Job.Execute(stateInfo.Kits);
                }
                catch (Exception ex)
                {
                    foreach (var k in stateInfo.Kits)
                    {
                        k.WriteLine("WorkManager:JobThreadProcess: Exception - " + ex.Message);
                    }

                    stateInfo.Job.Result = JobResult.Failed;
                }
                finally
                {
                    uint warnings = 0;
                    uint errors = 0;
                    uint fatalErrors = 0;

                    foreach (var k in stateInfo.Kits)
                    {
                        warnings += k.Warnings;
                        errors += k.Errors;
                        fatalErrors += k.FatalErrors;

                        k.WriteLine(String.Format("{0}: Result: ({1})", stateInfo.Job.ID, (stateInfo.Job.Result == JobResult.Passed) ? "Passed" : "Failed"));
                    }

                    stateInfo.Job.LogWarnings = warnings;
                    stateInfo.Job.LogErrors = errors;
                    stateInfo.Job.LogFatalErrors = fatalErrors;
                }

                // We can set this if the job was successful or if the job was failed for reasons other than a crash/assert
                if (dontRetry)
                {
                    break;
                }

                stateInfo.Job.RebootKitsBeforeRunning = true;
            }

            stateInfo.KitServer.UnlockKits(stateInfo.Kits);

            stateInfo.Job.Status = JobStatus.Completed;
            stateInfo.JobResetEvent.Set();

            stateInfo.Job.Duration = DateTime.Now - startTime;
        }

        public class BuildConfig
        {
            public string Platform360
            {
                get;
                set;
            }

            public string PlatformPS3
            {
                get;
                set;
            }

            public string PlatformPS3_Folder
            {
                get;
                set;
            }


            public string PlatformWin32
            {
                get;
                set;
            }
            
            public string PlatformWin32_Folder
            {
                get;
                set;
            }

            public string PlatformWin64
            {
                get;
                set;
            }
            
            public string PlatformWin64_Folder
            {
                get;
                set;
            }
        }

        private class State
        {
            private KitServer kitServer;
            private IJob job;
            private ManualResetEvent jobResetEvent;
            private List<AutomationKit> kits;

            /// <summary>
            /// Initializes a new instance of the State class.
            /// </summary>
            /// <param name="kitServer">The kit manager.</param>
            /// <param name="job">The job we are going to process</param>
            /// <param name="jobResetEvent">The job reset event.</param>
            /// <param name="kits">A list of kits available to do work for us</param>
            public State(KitServer kitServer, IJob job, ManualResetEvent jobResetEvent, List<AutomationKit> kits)
            {
                this.kitServer = kitServer;
                this.job = job;
                this.jobResetEvent = jobResetEvent;
                this.kits = kits;
            }

            public KitServer KitServer
            {
                get { return this.kitServer; }
            }

            public IJob Job
            {
                get { return this.job; }
            }

            /// <summary>
            /// Gets the event that is fired if a job is reset
            /// </summary>
            public ManualResetEvent JobResetEvent
            {
                get
                {
                    return this.jobResetEvent;
                }
            }
            
            /// <summary>
            /// Gets a list of available kits to do work on
            /// </summary>
            public List<AutomationKit> Kits
            {
                get
                {
                    return this.kits;
                }
            }
        }
    }
}
