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

    public class RouteTests
    {
        /// <summary>
        /// Test suite to load routes and drive 20 cars for 30secs (then take a picture)
        /// </summary>
        public class DashTestSuite : TestSuite
        {
            TestFinish testFinishCriteria;

            public DashTestSuite(AmaxXtHelper context, string param)
                : base(context, "Dash")
            {
                this.testFinishCriteria = new TestFinish(TimeSpan.FromSeconds(30));
                this.testFinishCriteria.Parse(param);
            }

            public override void CreateJobs(object context)
            {
                AmaxXtHelper xt = context as AmaxXtHelper;

                AmaxVehicle shelby = xt.Vehicle("Shelby_MustangGT500_Stock");

                var dashRoutes = from r in xt.Routes where (r.SkipForAutomation == false) orderby r.City.Name, r.Index select r;

                foreach (var r in dashRoutes)
                {
                    // Check if this route is allowed for testing
                    if (WorkManager.PassPickNMix("Route", r.Id))
                    {
                        string jobName = r.Name + string.Format(" ({0:X8})", r.Id);

                        //// WorkManager.MasterLog.WriteLine("Found: " + jobName);

                        KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1 };

                        AmaxCircuitLoadTestJob job = new AmaxCircuitLoadTestJob(jobName, "Dash", this.testFinishCriteria, kitRequirements, WorkManager.BuildConfigs[0]);

                        job.RouteId = r.Id;
                        for (int iCar = 0; iCar < 20; iCar++)
                        {
                            job.Vehicles.Add(new AmaxJob.AmaxJobVehicle() { Id = shelby.Id, ControlType = AmaxJob.AmaxJobVehicle.eControlType.Ai });
                        }

                        job.AddDependency("BASIC-1");

                        Add(job);
                    }
                }
            }

            public override void AddAdditionalColumns(TestSuite.AdditionalColumns additionalColumns)
            {
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Unique Loaded", Width = 50 });
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Dynamic Objects", Width = 50 });
                additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = "Physical Objects", Width = 50 });
            }

            public override void AdditionalColumnValue(IJob job, TestSuite.AdditionalColumn column, Workbook.Worksheet worksheet, Workbook.Worksheet.Row.Cell cell)
            {
                AmaxCircuitLoadTestJob j = job as AmaxCircuitLoadTestJob;

                switch (column.Name)
                {
                    case "Unique Loaded":
                        cell.Value = j.LevelUniqueObjects;
                        break;
                    case "Dynamic Objects":
                        cell.Value = j.LevelDynamicObjects;
                        break;
                    case "Physical Objects":
                        cell.Value = j.LevelPhysicalObjects;
                        break;
                }
            }
        }
    }
}
