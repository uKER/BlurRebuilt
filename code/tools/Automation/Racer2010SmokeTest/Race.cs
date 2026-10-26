namespace AutomationTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using AmaxAutomation;
    using AmaxAutomationHelper;
    using Automation;

    public class RaceTests
    {
        //public class RaceTestSuite : TestSuite
        //{
        //    public RaceTestSuite(AmaxXtHelper context)
        //        : base(context, "Dash")
        //    {
        //    }

        //    public override void CreateJobs(object context)
        //    {
        //        AmaxXtHelper xt = context as AmaxXtHelper;

        //        foreach (AmaxRoute r in xt.Routes)
        //        {
        //            // Check if this route is allowed for testing
        //            if (WorkManager.PassPickNMix("Route", r.Id))
        //            {
        //                string jobName = "Dash - " + r.Name + string.Format(" ({0:X8})", r.Id);

        //                //// AutomationHelper.DebugOutput.Write("Found: " + jobName);

        //                KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1 };

        //                AmaxRaceTestJob job = new AmaxRaceTestJob(jobName, TimeSpan.FromSeconds(30), 1, kitRequirements, WorkManager.BuildConfigs[0]);

        //                job.RouteId = r.Id;
        //                job.SetRandomCars(20, xt);

        //                job.AddDependency("BASIC-1");

        //                Add(job);
        //            }
        //        }
        //    }
        //}
    }
}