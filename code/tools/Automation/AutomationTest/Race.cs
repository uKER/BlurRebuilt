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

    public class RaceTests
    {
        public static void CreateJobs(AmaxXtHelper xt, string exe)
        {
            //// Find all routes flagged for testing, sort them by route index
            //var vehicles = from v in xt.Vehicles where v.Use == true orderby v.Name select v;

            //foreach (var v in vehicles)
            {
                string jobName = "My first race";
                //string jobName = v.Name + string.Format(" ({0:X8})", v.Id);

                //// AutomationHelper.DebugOutput.Write("Found: " + jobName);

                AmaxCity city = xt.City("UK_London_Hackney");
                AmaxRoute route = xt.Route(city.Id, 1);

                AmaxRaceTestJob job = new AmaxRaceTestJob(jobName, AutomationKitType.Xbox360, exe);

                //job.CityId = city.Id;
                job.RouteId = route.Id;

                job.SetRandomCars(20, xt);

                foreach (var v in job.Vehicles)
                {
                    v.ControlType = AmaxJob.AmaxJobVehicle.eControlType.Ai;
                }

                //job.AddCar(v.Id);

                WorkManager.AddJob(job);
            }
        }
    }
}