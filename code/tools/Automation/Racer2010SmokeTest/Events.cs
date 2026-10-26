namespace AutomationTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using AmaxAutomation;
    using AmaxAutomationHelper;
    using Automation;

    using Bizarre.Misc.ExcelXML;

    public class EventTests
    {
        /// <summary>
        /// Test suite to load routes and drive 20 cars for 30secs (then take a picture)
        /// </summary>
        public class EventTestSuite : TestSuite
        {
            public EventTestSuite(AmaxXtHelper context)
                : base(context, "Events")
            {
            }

            public override void CreateJobs(object context)
            {
                AmaxXtHelper xt = context as AmaxXtHelper;

                AmaxXtRecord shelby = xt.RaceBookVehicle("Shelby_MustangGT500_Stock_1");

                var rbGroups = from g in xt.RacebookGroups orderby g.Index select g;

                foreach (var g in rbGroups)
                {
                    foreach (var e in g.events)
                    {
                        // WorkManager.MasterLog.WriteLine(string.Format("Group({0}) Event({1})", g.Name, e.Name));

                        string jobName = string.Format("Group({0}) Event({1})", g.Name, e.Name) + string.Format(" ({0:X8})", e.Id);

                        KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1 };

						//AmaxEventTestJob job = new AmaxEventTestJob(jobName, TimeSpan.FromSeconds(30), kitRequirements, WorkManager.BuildConfigs[0]);

						//job.EventId = e.Id;
						//job.AddCar(shelby.Id, AmaxJob.AmaxJobVehicle.eControlType.Ai);

						//job.AddDependency("BASIC-1");

						//Add(job);
                    }
                }

                //var dashRoutes = from r in xt.Routes orderby r.City.Name, r.Index select r;

                //foreach (var r in dashRoutes)
                //{
                //    // Check if this route is allowed for testing
                //    if (WorkManager.PassPickNMix("Route", r.Id))
                //    {
                //        string jobName = r.Name + string.Format(" ({0:X8})", r.Id);

                //        //// WorkManager.MasterLog.WriteLine("Found: " + jobName);

                //        KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1 };

                //        AmaxCircuitLoadTestJob job = new AmaxCircuitLoadTestJob(jobName, "Dash", TimeSpan.FromSeconds(30), kitRequirements, WorkManager.BuildConfigs[0]);

                //        job.RouteId = r.Id;
                //        //job.SetRandomCars(20, xt);
                //        for (int iCar = 0; iCar < 20; iCar++)
                //        {
                //            job.Vehicles.Add(new AmaxJob.AmaxJobVehicle() { Id = shelby.Id, ControlType = AmaxJob.AmaxJobVehicle.eControlType.Ai });
                //        }

                //        job.AddDependency("BASIC-1");

                //        Add(job);
                //    }
                //}
            }
        } 
    }
}
