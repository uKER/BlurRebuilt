namespace Automation
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    public enum JobStatus
    {
        NotStarted  = 0,
        InProgress,
        Completed
    }

    public enum JobResult
    {
        NoResult = 0,
        Failed,
        Passed,
        Skipped,
        ConnectionIssue
    }
    
    public enum JobFailReason
    {
        Unknown,
        Canceled,
        TimedOut,
        Asserted,
        Crashed,
        MemoryLeak,
        Logic,
        NoKitsAvailable,
    }

    public class MemoryLeakInfo
    {
        public uint System { get; set; }
        public uint Physical { get; set; }
    }

    public interface IJob
    {
        int ID
        {
            get;
        }

        string Tag
        {
            get;
            set;
        }

        List<string> Dependencies
        {
            get;
        }

        List<IJob> DependentJobs
        {
            get;
        }

        uint TestSuite
        {
            get;
            set;
        }

        JobStatus Status
        {
            get;
            set;
        }

        JobResult Result
        {
            get;
            set;
        }

        JobFailReason FailReason
        {
            get;
            set;
        }

        object FailInfo
        {
            get;
            set;
        }

        TimeSpan Duration
        {
            get;
            set;
        }

        KitServer.KitRequest KitRequirements
        {
            get;
        }

        WorkManager.BuildConfig BuildConfig
        {
            get;
        }

        bool RebootKitsBeforeRunning
        {
            get;
            set;
        }

        string KitNames
        {
            get;
            set;
        }

        string Name
        {
            get;
        }

        string JobStage
        {
            get;
            set;
        }

        string Comment
        {
            get;
            set;
        }

        object ResultsData
        {
            get;
            set;
        }

        uint LogWarnings
        {
            get;
            set;
        }

        uint LogErrors
        {
            get;
            set;
        }

        uint LogFatalErrors
        {
            get;
            set;
        }

        string BuildVersion
        {
            get;
        }

        string BuildDate
        {
            get;
        }

        string BuildTime
        {
            get;
        }

        void AddDependency(string tag);

        void AddDependency(IJob job);
  
        bool Execute(List<AutomationKit> kits);
    }
}
