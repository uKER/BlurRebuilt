namespace AutomationTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using NightwingAutomation;
    using NightwingAutomationHelper;
    using Automation;

    public class LevelTests
    {
        public class LevelSuite : TestSuite
        {
            private bool take360Shots;

            public LevelSuite(NightwingXtHelper context, bool take360Shots)
                : base(context, "Levels")
            {
                this.take360Shots = take360Shots;
            }

            public override void AddAdditionalColumns(TestSuite.AdditionalColumns additionalColumns)
            {
                foreach (string colName in NightwingLevelLoadTestJob.s_MemoryNames)
                {
                    additionalColumns.columns.Add(new TestSuite.AdditionalColumn() { Name = colName, Width = 75 });
                }
            }

            public override void AdditionalColumnValue(IJob job, TestSuite.AdditionalColumn column, Bizarre.Misc.ExcelXML.Workbook.Worksheet workSheet, Bizarre.Misc.ExcelXML.Workbook.Worksheet.Row.Cell cell)
            {
                NightwingLevelLoadTestJob levelJob = job as NightwingLevelLoadTestJob;
                cell.Value = levelJob.GetMemorySize(column.Name);
            }

            public override void CreateJobs(object context)
            {
                NightwingXtHelper xt = context as NightwingXtHelper;

                var spl = from l in xt.SinglePlayerLevels select l;

                //// Find all routes flagged for testing, sort them by route index
                //var vehicles = from v in xt.Vehicles orderby v.Name select v;

                foreach (var l in spl)
                {
                //    bool use = WorkManager.PickNMixValid ? WorkManager.PassPickNMix("AmaxVehicle", v.Id) : (v.Use == true);

                //    if (use)
                //    {
                        string jobName = l.Name + string.Format(" ({0:X8})", l.Id);

                //        //// AutomationHelper.DebugOutput.Write("Found: " + jobName);

                //        NightwingCity city = xt.City("UK_London_Hackney");
                //        NightwingRoute route = xt.Route(city.Id, 1);

                        KitServer.KitRequest kitRequirements = new KitServer.KitRequest() { XBOX_360 = 1 };

                        NightwingLevelLoadTestJob job = new NightwingLevelLoadTestJob(jobName, kitRequirements, WorkManager.BuildConfigs[0]);

                //        job.ShowHUD = false;

                //        job.Take360ScreenShots = take360Shots;

                        job.LevelId = l.Id;

                //        job.AddCar(v.Id);

                //        job.AddDependency("BASIC-1");

                        Add(job);
                //    }
                }
            }
        }
    }
}