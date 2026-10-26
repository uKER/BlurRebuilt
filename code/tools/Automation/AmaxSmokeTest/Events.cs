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
            TestFinish testFinishCriteria;

            public EventTestSuite(AmaxXtHelper context, string param)
                : base(context, "Events")
            {
                this.testFinishCriteria = new TestFinish(TimeSpan.FromSeconds(60));
                this.testFinishCriteria.Parse(param);
            }

            public override void CreateJobs(object context)
            {
                AmaxXtHelper xt = context as AmaxXtHelper;

                AmaxXtRecord shelby = xt.RaceBookVehicle("Shelby_MustangGT500_Stock_1");

                if (shelby == null)
                {
                    if (xt.RacebookVehicles.Count > 0)
                    {
                        shelby = xt.RacebookVehicles[0];
                    }
                    else
                    {
                        WorkManager.MasterLog.WriteLine("ERROR: Events: CreateJobs - Can't find any RacebookVehicles");
                        return;
                    }
                }

                var rbGroups = from g in xt.RacebookGroups orderby g.Index select g;

                foreach (var g in rbGroups)
                {
                    foreach (var e in g.events)
                    {
                        // WorkManager.MasterLog.WriteLine(string.Format("Group({0}) Event({1})", g.Name, e.Name));

                        string jobName = string.Format("Group({0}) Event({1})", g.Name, e.Name) + string.Format(" ({0:X8})", e.Id);

                        KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1 };

                        AmaxEventTestJob job = new AmaxEventTestJob(jobName, this.testFinishCriteria, kitRequirements, WorkManager.BuildConfigs[0]);

                        job.EventId = e.Id;
                        job.AddCar(shelby.Id);

                        job.AddDependency("BASIC-1");

                        Add(job);
                    }
                }
            }
        } 
    }
}
