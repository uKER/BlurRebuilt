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

    public class PerfTests
    {
        /// <summary>
        /// Test suite to load routes and drive 20 cars for 30secs (then take a picture)
        /// </summary>
        public class PerfSuite : TestSuite
        {
            const bool DoAllRoutes = true;
            const bool JustFirstRoute = false;

            TestFinish testFinishCriteria;

            public PerfSuite(AmaxXtHelper context, string param)
                : base(context, "Performance")
            {
                this.testFinishCriteria = new TestFinish(TimeSpan.FromSeconds(120));
                this.testFinishCriteria.Parse(param);
            }

            public void CreateJobs_Events(object context)
            {
                AmaxXtHelper xt = context as AmaxXtHelper;

                AmaxXtRecord shelby = xt.RaceBookVehicle("Shelby_MustangGT500_Stock_1");

                var rbGroups = from g in xt.RacebookGroups orderby g.Index select g;

                foreach (var g in rbGroups)
                {
                    foreach (var e in g.events)
                    {
                        foreach (var kitRequest in WorkManager.KitServer.RequestEachAvailable(1))
                        {
                            // WorkManager.MasterLog.WriteLine(string.Format("Group({0}) Event({1})", g.Name, e.Name));

                            string jobName = string.Format("Group({0}) Event({1})", g.Name, e.Name) + string.Format(" ({0:X8})", e.Id);

                            AmaxPerfJob job = new AmaxPerfJob(jobName, this.testFinishCriteria, kitRequest, WorkManager.BuildConfigs[0]);

                            job.EventId = e.Id;
                            job.AddCar(shelby.Id);

                            job.AddDependency("BASIC-1");

                            Add(job);
                        }
                    }
                }
            }

            class PerfRoute
            {
                public string City { get; set; }
                public int RouteIndex { get; set; }
                
                public AmaxRoute AmaxRoute { get; set; }

                public PerfRoute(string city, int route)
                {
                    this.City = city;
                    this.RouteIndex = route;
                    this.AmaxRoute = null;
                }
                
                public PerfRoute(AmaxRoute route)
                {
                    this.City = null;
                    this.RouteIndex = 0;
                    this.AmaxRoute = route;
                }
            }

            private void CreateJobs_E3(object context)
            {
                AmaxXtHelper xt = context as AmaxXtHelper;

                List<PerfRoute> perfRoutes = new List<PerfRoute>();
                List<int> carScenarios = new List<int>();

                int numRacesOfEachType = 2;

                // Add selected routes
                if (!DoAllRoutes)
                {
                    perfRoutes.Add(new PerfRoute("USA_LA_River", 3));
                    perfRoutes.Add(new PerfRoute("USA_LA_Downtown", 6));
                    //perfRoutes.Add(new PerfRoute("UK_London_Hackney", 1));
                    perfRoutes.Add(new PerfRoute("UK_London_Hackney", 2));
                    //perfRoutes.Add(new PerfRoute("UK_London_Hackney", 4));
                    perfRoutes.Add(new PerfRoute("USA_Amboy", 2));
                    perfRoutes.Add(new PerfRoute("SPA_Barcelona_Oval", 2));

                    perfRoutes.Add(new PerfRoute("SPA_Barcelona_Gracia", 2));
                    perfRoutes.Add(new PerfRoute("USA_SanFran_RussianHill", 1));
                }
                else
                {
                    // Add all the dash routes
                    var dashRoutes = from r in xt.Routes where (r.SkipForAutomation == false && r.Index != 0) orderby r.City.Name, r.Index select r;

                    List<string> cityNames = new List<string>();

                    foreach (var r in dashRoutes)
                    {
                        // Check if this route is allowed for testing
                        if (WorkManager.PassPickNMix("Route", r.Id))
                        {
                            // Do we only want to do the tests on one route in every city?
                            if (JustFirstRoute)
                            {
                                if (cityNames.Contains(r.City.Name))
                                {
                                    continue;
                                }
                            }

                            cityNames.Add(r.City.Name);

                            perfRoutes.Add(new PerfRoute(r));
                        }
                    }
                }

                carScenarios.Add(1);
                carScenarios.Add(10);
                carScenarios.Add(20);

                foreach (var numCarsInRace in carScenarios)
                {
                    foreach (var proute in perfRoutes)
                    {
                        AmaxRoute route = (proute.AmaxRoute != null) ? proute.AmaxRoute : xt.Route(proute.City, proute.RouteIndex);

                        System.Diagnostics.Debug.Assert(route != null, "Route specified for performance run cannot be found");

                        AmaxVehicle shelby = xt.Vehicle("Shelby_MustangGT500_Stock");

                        string jobName = route.Name + string.Format(" ({0:X8})", route.Id);

                        // We want to run this test on all types of kit we have available
                        foreach (var kitRequest in WorkManager.KitServer.RequestEachAvailable(1))
                        {
                            for (int iRaces = 0; iRaces < numRacesOfEachType; iRaces++)
                            {
                                AmaxPerfJob job = new AmaxPerfJob(jobName, this.testFinishCriteria, kitRequest, WorkManager.BuildConfigs[0]);

                                job.RouteId = route.Id;
                                for (int iCar = 0; iCar < numCarsInRace; iCar++)
                                {
                                    job.Vehicles.Add(new AmaxJob.AmaxJobVehicle() { Id = shelby.Id, ControlType = AmaxJob.AmaxJobVehicle.eControlType.Ai });
                                }

                                job.AddDependency("BASIC-1");

                                Add(job);
                            }
                        }
                    }
                }
            }

            public override void CreateJobs(object context)
            {
                //   CreateJobs_Events(context);
                CreateJobs_E3(context);
            }
        }
    }
}
