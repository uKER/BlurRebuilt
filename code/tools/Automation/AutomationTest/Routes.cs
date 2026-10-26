namespace AutomationTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using AmaxAutomation;
    using AmaxAutomationHelper;
    using Automation;

    using ExcelXML;

    public class RouteTests
    {
        public static void CreateJobs_AllFlaggedRoutes(AmaxXtHelper xt, string exe)
        {
            // Find all routes flagged for testing, sort them by route index
            //var routes = from r in xt.Routes where (r.AutomatedTesting == true) orderby r.City, r.Index select r;
            var routes = from r in xt.Routes orderby r.City, r.Index select r;

            foreach (var route in routes)
            {
//                bool use = WorkManager.PickNMixValid ? WorkManager.PassPickNMix(v.Id) : (v.Use == true);
                bool use = WorkManager.PassPickNMix("Route", route.Id);

                AmaxCity city = xt.City(route.City);

                string jobName = city.Name + " " + route.Index;

                WorkManager.MasterLog.WriteLine("Found: " + jobName);

                AmaxCircuitLoadTestJob job = new AmaxCircuitLoadTestJob(jobName, AutomationKitType.Xbox360, exe);

                //job.CityId = city.Id;
                job.RouteId = route.Id;

                WorkManager.AddJob(job);
            }
        }
    }
}