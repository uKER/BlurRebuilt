//-----------------------------------------------------------------------
// <copyright file="BaseJob.cs" company="Bizarre Creations">
// Copywright Bizarre Creations 2009
// </copyright>
//-----------------------------------------------------------------------
namespace Automation
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    public abstract class BaseJob : IJob
    {
        private static int uniqueJobID = 0;

        private uint testsuiteId = 0;

        private string jobStage;
        private string comment;

        protected string buildVersion = string.Empty;
        protected string buildDate = string.Empty;
        protected string buildTime = string.Empty;

        private uint logWarnings;
        private uint logErrors;
        private uint logFatalErrors;
        
        /// <summary>
        /// The tag field is used so jobs can make themselves dependant on other jobs (i.e. only run if BASIC-1 is successful)
        /// </summary>
        private string tag;

        /// <summary>
        /// Should we reboot all the kits before trying this job (we set this to try on retries too)
        /// </summary>
        private bool rebootKitsBeforeRunning = false;
        
        /// <summary>
        /// A ID which uniquely identifies the job
        /// </summary>
        private int id;

        private List<string> dependencies;
        private List<IJob> jobDependencies;

        private JobStatus status = JobStatus.NotStarted;
        private JobResult result = JobResult.NoResult;
        private JobFailReason failReason = JobFailReason.Unknown;
        private TimeSpan duration;
        private object failInfo;
        
        private string name;
        private KitServer.KitRequest kitRequirements;
        private string kitNames;
        
        /// <summary>
        /// Holds the names of the executables to run on each of the platforms required
        /// </summary>
        private WorkManager.BuildConfig buildConfig;

        private List<AutomationConnection.ProblemInfo> problems;

        public BaseJob(string name, KitServer.KitRequest kitRequirements, WorkManager.BuildConfig buildConfig)
        {
            this.id = uniqueJobID++;
            this.name = name;
            this.kitRequirements = kitRequirements;
            this.buildConfig = buildConfig;
        }

        public string BuildVersion
        {
            get
            {
                return this.buildVersion;
            }
        }

        public string BuildDate
        {
            get
            {
                return this.buildDate;
            }
        }

        public string BuildTime
        {
            get
            {
                return this.buildTime;
            }
        }

        public string Tag
        {
            get
            {
                return this.tag;
            }

            set
            {
                this.tag = value;
            }
        }

        public uint LogWarnings
        {
            get
            {
                return this.logWarnings;
            }

            set
            {
                this.logWarnings = value;
            }
        }

        public uint LogErrors
        {
            get
            {
                return this.logErrors;
            }

            set
            {
                this.logErrors = value;
            }
        }
        
        public uint LogFatalErrors
        {
            get
            {
                return this.logFatalErrors;
            }

            set
            {
                this.logFatalErrors = value;
            }
        }

        public uint TestSuite
        {
            get
            {
                return this.testsuiteId;
            }

            set
            {
                this.testsuiteId = value;
            }
        }

        /// <summary>
        /// Gets or sets whether we should reboot all the kits before we run this job
        /// </summary>
        public bool RebootKitsBeforeRunning
        {
            get
            {
                return this.rebootKitsBeforeRunning;
            }

            set
            {
                this.rebootKitsBeforeRunning = value;
            }
        }

        /// <summary>
        /// Gets the unique ID to identify this job by
        /// </summary>
        public int ID
        {
            get 
            {
                return this.id;
            }
        }

        /// <summary>
        /// Gets or sets the duration of the job
        /// </summary>
        public TimeSpan Duration
        {
            get
            {
                return this.duration;
            }

            set
            {
                this.duration = value;
            }
        }

        public List<AutomationConnection.ProblemInfo> Problems
        {
            get
            {
                return this.problems;
            }

            set
            {
                this.problems = value;
            }
        }
        
        /// <summary>
        /// Gets or sets a comment which helps identify how far through a job it is
        /// </summary>
        public string JobStage
        {
            get
            {
                return this.jobStage; 
            }
            
            set
            {
                this.jobStage = value;
            }
        }

        public string Comment
        {
            get
            {
                return this.comment;
            }

            set
            {
                this.comment = value;
            }
        }

        public object ResultsData
        {
            get;
            set;
        }
        
        public JobStatus Status
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
        
        public JobResult Result
        {
            get
            {
                return this.result;
            }

            set
            {
                this.result = value;
            }
        }

        public JobFailReason FailReason
        {
            get
            {
                return this.failReason;
            }

            set
            {
                this.failReason = value;
            }
        }

        public object FailInfo
        {
            get
            {
                return this.failInfo;
            }

            set
            {
                this.failInfo = value;
            }
        }

        public string Name
        {
            get { return this.name; }
        }

        public KitServer.KitRequest KitRequirements
        {
            get
            {
                return this.kitRequirements;
            }
        }

        public WorkManager.BuildConfig BuildConfig
        {
            get
            {
                return this.buildConfig;
            }
        }

        public string KitNames
        {
            get
            {
                return this.kitNames;
            }

            set
            {
                this.kitNames = value;
            }
        }

        public List<string> Dependencies
        {
            get
            {
                return this.dependencies;
            }
        }

        public List<IJob> DependentJobs
        {
            get
            {
                return this.jobDependencies;
            }
        }

        /// <summary>
        /// Allows us to specify a job on which this job is dependant. We won't run unless this other job is successful
        /// </summary>
        /// <param name="job"></param>
        public void AddDependency(string tag)
        {
            if (this.dependencies == null)
            {
                this.dependencies = new List<string>();
            }

            this.dependencies.Add(tag);
        }

        public void AddDependency(IJob job)
        {
            if (this.jobDependencies == null)
            {
                this.jobDependencies = new List<IJob>();
            }

            this.jobDependencies.Add(job);
        }

        ////public abstract void Execute(List<KitManager.XenonKit> kits);
        public abstract bool Execute(List<AutomationKit> kits);

        /// <summary>
        /// Adds text onto the comment and comma separates where necessary
        /// </summary>
        /// <param name="text">The text to add onto the end of the comment</param>
        protected void AddToComment(string text)
        {
            if (this.Comment != null && this.Comment.Length > 0)
            {
                this.Comment += ", " + text;
            }
            else
            {
                this.Comment = text;
            }
        }
    }
}
