namespace AmaxPerfAnalyser
{
    using System;
    using System.Collections.Generic;
    using System.Data;
    using System.Linq;
    using System.Text;

    using Bizarre.Misc;
    using Bizarre.Misc.ExcelXML;

    using MySql.Data;
    using MySql.Data.MySqlClient;

    enum Platform
    {
        Unknown,
        XBOX_360,
        PS3,
        WIN32,
        WIN64
    }

    class PerfStat
    {
        public int subsession;

        public Platform platform;

        public uint LocationId; // Sorcery Id's
        public uint RouteId;
        public uint numcars;

        public float x, y, z;

        public float frameLength;

        public float cpuTotal;
        public float cpuUpdate;
        public float cpuRender;

        public float gpuTotal;

        public uint commandBufferSize;
        public uint freeMemory;
    }

    class SubSession
    {
        public Platform platform;

        public uint LocationId;
        public uint RouteId;

        public float loadingTime;
        public float loadingStall;
        public float fillStreamer;
        public uint dynamicObjects;
        public uint physicalObjects;
        public uint numCars;
    }

    class LocationInfo
    {
        public string LocationName;
        public string RouteName;
        
        public uint LocationId; // Sorcery Id
        public uint RouteId; // Sorcery Id
    }

    class Program
    {
        static string connectionString = string.Format("Database=information_schema;Data Source={0};User Id={1};Password={2}", "localhost", "", "");

        static void Main(string[] args)
        {
            string reportFolder = string.Empty;

            Arguments argParser = new Arguments(args);

            if (argParser["reports"] != null)
            {
                reportFolder = argParser["reports"];
            }

            List<LocationInfo> locList = new List<LocationInfo>();
            List<PerfStat> perfStatList = new List<PerfStat>();
            List<SubSession> subSessionList = new List<SubSession>();

            // If false, split out by route
            bool groupByLocation = true;

            // TODO: Find last run and use that

            string session = "?"; // 2009.05.30 - 13.30";

            MySql.Data.MySqlClient.MySqlConnection conn = new MySql.Data.MySqlClient.MySqlConnection(connectionString);

            conn.Open();

            // Find session to dump stats on
            {
                MySqlDataAdapter daLastSession = new MySqlDataAdapter("SELECT * FROM amaxperf._sessions ORDER BY time DESC LIMIT 1", conn);
                System.Data.DataSet dsLastSession = new System.Data.DataSet();
                daLastSession.Fill(dsLastSession);

                System.Diagnostics.Debug.Assert(dsLastSession.Tables[0].AsEnumerable().Count() == 1);

                var lastSession = dsLastSession.Tables[0].AsEnumerable().First();

                session = (string)lastSession["ref"];
            }

            reportFolder = System.IO.Path.Combine(reportFolder, "Amax Perf Report");
            reportFolder = System.IO.Path.Combine(reportFolder, session);

            MySqlDataAdapter daStrings = new MySqlDataAdapter("SELECT * FROM amaxperf._strings", conn);
            System.Data.DataSet dsStrings = new System.Data.DataSet();
            daStrings.Fill(dsStrings);

            MySqlDataAdapter daSessions = new MySqlDataAdapter("SELECT _id,config,version,platform FROM amaxperf._sessions WHERE amaxperf._sessions.ref = '" + session + "'", conn);
            System.Data.DataSet dsSessions = new System.Data.DataSet();
            daSessions.Fill(dsSessions);

            string idRange = string.Empty;
            Dictionary<uint, string> sessionPlatform = new Dictionary<uint, string>();
            Dictionary<string, Platform> platformList = new Dictionary<string, Platform>();

            // Find range of sessions
            foreach (var b in dsSessions.Tables[0].AsEnumerable())
            {
                if (idRange.Length > 0)
                {
                    idRange += ",";
                }

                idRange += b["_id"];

                string platform = (string)b["platform"];

                if (!platformList.ContainsKey(platform))
                {
                    Platform ptype = Platform.Unknown;

                    switch (platform)
                    {
                        case "360":
                            ptype = Platform.XBOX_360;
                            break;
                        case "PS3":
                            ptype = Platform.PS3;
                            break;
                        case "WIN32":
                            ptype = Platform.WIN32;
                            break;
                        case "WIN64":
                            ptype = Platform.WIN64;
                            break;
                    }

                    platformList.Add(platform, ptype);
                }

                sessionPlatform.Add((uint)b["_id"], platform);
            }

            // Find subsessions that match the sessions
            MySqlDataAdapter daPerfStats = new MySqlDataAdapter("SELECT * FROM amaxperf._subsessions WHERE amaxperf._subsessions._sessionid IN(" + idRange + ")", conn);

            System.Data.DataSet ds = new System.Data.DataSet();
            daPerfStats.Fill(ds);

            System.Data.DataTable dt = ds.Tables[0];

            var subSessions = from x in dt.AsEnumerable() where !x.IsNull("end") select x;

            // Write out report in Excel XML workbook
            Workbook workBook = new Workbook();

            // TODO: WorkSheet for each Route? Put all car, platform and runs on to the same sheet?

            Dictionary<uint, Workbook.Worksheet> routeWorksheetDict = new Dictionary<uint, Workbook.Worksheet>();
            List<string> routeNames = new List<string>();

            // Run through all the extracted stats
            foreach (var ss in subSessions)
            {
                var locationName = from x in dsStrings.Tables[0].AsEnumerable() where (uint)x["_id"] == (uint)ss["LocationId"] select x["name"];
                var routeName = from x in dsStrings.Tables[0].AsEnumerable() where (uint)x["_id"] == (uint)ss["RouteId"] select x["name"];

                LocationInfo li = new LocationInfo();

                li.LocationName = (locationName.Count() > 0) ? (string)locationName.First() : "?";
                li.RouteName = (routeName.Count() > 0) ? (string)routeName.First() : "?";
                li.LocationId = (uint)ss["LocationId"];
                li.RouteId = (uint)ss["RouteId"];

                SubSession locSubSession = new SubSession();

                locSubSession.LocationId = (uint)ss["LocationId"];
                locSubSession.RouteId = (uint)ss["RouteId"];

                locSubSession.loadingTime = (float)ss["LoadingTime"];
                locSubSession.loadingStall = (float)ss["LoadingStall"];
                locSubSession.fillStreamer = (float)ss["FillStreamer"];
                locSubSession.dynamicObjects = (uint)ss["DynamicObjects"];
                locSubSession.physicalObjects = (uint)ss["PhysicalObjects"];
                locSubSession.numCars = (byte)ss["NumCars"];

                uint ssId = (uint)ss["_id"];
                uint sessionId = (uint)ss["_sessionid"];

                locSubSession.platform = platformList[sessionPlatform[sessionId]];

                // Find all perf stats for this subsession
                MySqlDataAdapter daPS = new MySqlDataAdapter("SELECT * FROM amaxperf.perfstat WHERE amaxperf.perfstat._subsessionid = " + ssId, conn);

                System.Data.DataSet dsp = new System.Data.DataSet();
                daPS.Fill(dsp);

                System.Data.DataTable dtp = dsp.Tables[0];

                var perfs = from x in dtp.AsEnumerable() orderby x["FrameTime"] select x;

                float maxFrameLength = perfs.Max(x => (float)x["FrameLength"]);
                float maxCpuTotal = perfs.Max(x => (float)x["CpuTotal"]);
                float maxCpuUpdate = perfs.Max(x => (float)x["CpuUpdate"]);
                float maxCpuRender = perfs.Max(x => (float)x["CpuRender"]);
                float maxGpuTotal = perfs.Max(x => (float)x["GpuTotal"]);
                uint minFreeMemory = perfs.Min(x => (uint)x["FreeMemory"]);
                uint maxCmdBufferSize = perfs.Max(x => (uint)x["CmdBuffSize"]);

                Workbook.Worksheet workSheet;

                if (routeWorksheetDict.Keys.Contains(locSubSession.RouteId))
                {
                    workSheet = routeWorksheetDict[locSubSession.RouteId];
                }
                else
                {
                    string sheetName = string.Empty;

                    if (routeNames.Contains(li.RouteName))
                    {
                        int c = 1;

                        while (routeNames.Contains(li.RouteName + "_" + c))
                        {
                            c++;
                        }

                        sheetName = li.RouteName + "_" + c;
                    }
                    else
                    {
                        sheetName = li.RouteName;
                    }

                    routeNames.Add(sheetName);

                    workSheet = new Workbook.Worksheet(workBook, sheetName);

                    workSheet.AutoFilter = true;
                    workSheet.FreezeRows = 1;

                    routeWorksheetDict.Add(locSubSession.RouteId, workSheet);

                    workBook.Worksheets.Add(workSheet);

                    // Add column headers to excel
                    {
                        Workbook.Worksheet.Row row = workSheet.AddRow();

                        row.AddCell("NumCars");
                        row.AddCell("Platform");

                        foreach (var col in dtp.Columns)
                        {
                            string column = col.ToString();

                            if (column != "_id" && column != "_sessionid")
                            {
                                row.AddCell(column);
                            }
                        }
                    }
                }

                foreach (var p in perfs)
                {
                    PerfStat ps = new PerfStat();

                    ps.subsession = subSessionList.Count();

                    ps.platform = locSubSession.platform;

                    ps.LocationId = locSubSession.LocationId;
                    ps.RouteId = locSubSession.RouteId;
                    ps.numcars = locSubSession.numCars;

                    ps.x = (float)p["x"];
                    ps.y = (float)p["y"];
                    ps.z = (float)p["z"];

                    ps.frameLength = (float)p["FrameLength"];

                    ps.cpuTotal = (float)p["CpuTotal"];
                    ps.cpuUpdate = (float)p["CpuUpdate"];
                    ps.cpuRender = (float)p["CpuRender"];

                    ps.gpuTotal = (float)p["GpuTotal"];

                    ps.commandBufferSize = (uint)p["CmdBuffSize"];
                    ps.freeMemory = (uint)p["FreeMemory"];

                    perfStatList.Add(ps);

                    Workbook.Worksheet.Row row = workSheet.AddRow();

                    row.AddCell(locSubSession.numCars);
                    row.AddCell(locSubSession.platform.ToString());

                    foreach (var col in dtp.Columns)
                    {
                        string column = col.ToString();

                        if (column != "_id" && column != "_sessionid")
                        {
                            row.AddCell(p[column]);
                        }
                    }
                }

                subSessionList.Add(locSubSession);

                if (!locList.Exists(x => x.RouteId == li.RouteId))
                {
                    locList.Add(li);
                }
            }

            string reportFilename = System.IO.Path.Combine(reportFolder, "perf_report");

            string finalReport = workBook.Save(reportFilename);

            conn.Close();

            // Report
            string htmlBody = string.Empty;

            htmlBody += "<b>Raw data:</b><br><br>" + "<a href=\"file:///" + finalReport + "\">" + finalReport + "</a><br><br>";

            htmlBody += "<b>High level status:</b><br><br>\n";

            string green = "76923C"; // "688E23";
            string red = "E36C0A"; // "F4A460";

            var carCounts = subSessionList.Select(x => x.numCars).Distinct().OrderByDescending(x => x);

            foreach (var nc in carCounts)
            {
                HtmlHelper htmlHelper = new HtmlHelper();

                htmlHelper.BeginTable(1);

                // Table header
                htmlHelper.BeginTableRow();
                htmlHelper.AddTableHeader(nc + " Cars", (platformList.Count > 1) ? 2 : 1);
                htmlHelper.AddTableHeader("% of Time at Framerate (FPS)", 8);
                htmlHelper.AddTableHeader("Avg Timing (ms)", 4);
                htmlHelper.AddTableHeader("Memory (MB)", 2);
                htmlHelper.EndTableRow();

                htmlHelper.BeginTableRow();
                htmlHelper.AddTableHeader("Location");
                if (platformList.Count > 1)
                {
                    htmlHelper.AddTableHeader("Platform");
                }
                htmlHelper.AddTableHeader("Avg FPS");
                htmlHelper.AddTableHeader("< 30");
                htmlHelper.AddTableHeader("< 28");
                htmlHelper.AddTableHeader("< 25");
                htmlHelper.AddTableHeader("< 20");
                htmlHelper.AddTableHeader("< 15");
                htmlHelper.AddTableHeader("< 10");
                htmlHelper.AddTableHeader("< 5");
                htmlHelper.AddTableHeader("CpuTotal");
                htmlHelper.AddTableHeader("CpuUpdate");
                htmlHelper.AddTableHeader("CpuRender");
                htmlHelper.AddTableHeader("GpuTotal");
                htmlHelper.AddTableHeader("Min Free");
                htmlHelper.AddTableHeader("Max Cmd");
                htmlHelper.EndTableRow();

                List<uint> locDone = new List<uint>();

                foreach (var l in locList)
                {
                    if (locDone.Contains(l.LocationId))
                        continue;

                    if (groupByLocation)
                    {
                        locDone.Add(l.LocationId);
                    }

                    int iPlatform = 0;

                    foreach (KeyValuePair<string, Platform> kvp in platformList)
                    {
                        var locationData = (groupByLocation) ? 
                            perfStatList.FindAll(x => x.LocationId == l.LocationId && x.numcars == nc && x.platform == kvp.Value) :
                            perfStatList.FindAll(x => x.RouteId == l.RouteId && x.numcars == nc && x.platform == kvp.Value);

                        if (locationData.Count > 0)
                        {
                            htmlHelper.BeginTableRow();

                            float percUnder30 = 100 * locationData.Count(x => x.frameLength > (1000.0 / 30.0)) / locationData.Count;
                            float percUnder28 = 100 * locationData.Count(x => x.frameLength > (1000.0 / 28.0)) / locationData.Count;
                            float percUnder25 = 100 * locationData.Count(x => x.frameLength > (1000.0 / 25.0)) / locationData.Count;
                            float percUnder20 = 100 * locationData.Count(x => x.frameLength > (1000.0 / 20.0)) / locationData.Count;
                            float percUnder15 = 100 * locationData.Count(x => x.frameLength > (1000.0 / 15.0)) / locationData.Count;
                            float percUnder10 = 100 * locationData.Count(x => x.frameLength > (1000.0 / 10.0)) / locationData.Count;
                            float percUnder5 = 100 * locationData.Count(x => x.frameLength > (1000.0 / 5.0)) / locationData.Count;

                            uint minMemory = locationData.Min(x => x.freeMemory);
                            uint maxCmdBuffer = locationData.Max(x => x.commandBufferSize);

                            float avgFrameLength = (float)(1000.0 / locationData.Average(x => x.frameLength));

                            htmlHelper.AddTableData((iPlatform == 0) ? (groupByLocation) ? l.LocationName : l.RouteName : string.Empty);

                            htmlHelper.TextAlignment = "right";

                            if (platformList.Count > 1)
                            {
                                htmlHelper.AddTableData(kvp.Key);
                            }

                            htmlHelper.AddTableData(string.Format("{0:0.00}", avgFrameLength));
                            htmlHelper.AddTableData(HtmlHelper.ColourText(HtmlHelper.BoldText(string.Format("{0:0}%", percUnder30)), percUnder30 > 20 ? red : green));
                            htmlHelper.AddTableData(HtmlHelper.ColourText(HtmlHelper.BoldText(string.Format("{0:0}%", percUnder28)), percUnder28 > 10 ? red : green));
                            htmlHelper.AddTableData(HtmlHelper.ColourText(HtmlHelper.BoldText(string.Format("{0:0}%", percUnder25)), percUnder25 > 5 ? red : green));
                            htmlHelper.AddTableData(HtmlHelper.ColourText(HtmlHelper.BoldText(string.Format("{0:0}%", percUnder20)), percUnder20 > 1 ? red : green));
                            htmlHelper.AddTableData(HtmlHelper.ColourText(HtmlHelper.BoldText(string.Format("{0:0}%", percUnder15)), percUnder15 > .1 ? red : green));
                            htmlHelper.AddTableData(HtmlHelper.ColourText(HtmlHelper.BoldText(string.Format("{0:0}%", percUnder10)), percUnder10 > .1 ? red : green));
                            htmlHelper.AddTableData(HtmlHelper.ColourText(HtmlHelper.BoldText(string.Format("{0:0}%", percUnder5)), percUnder5 > .1 ? red : green));
                            htmlHelper.AddTableData(string.Format("{0:0.00}", locationData.Average(x => x.cpuTotal)));
                            htmlHelper.AddTableData(string.Format("{0:0.00}", locationData.Average(x => x.cpuUpdate)));
                            htmlHelper.AddTableData(string.Format("{0:0.00}", locationData.Average(x => x.cpuRender)));
                            htmlHelper.AddTableData(string.Format("{0:0.00}", locationData.Average(x => x.gpuTotal)));
                            htmlHelper.AddTableData(string.Format("{0:0.00}", minMemory / (1024.0 * 1024.0)));
                            htmlHelper.AddTableData(string.Format("{0:0.00}", maxCmdBuffer / (1024.0 * 1024.0)));

                            htmlHelper.EndTableRow();
                        }
                        iPlatform++;
                    }
                }
                htmlHelper.EndTable();

                htmlBody += htmlHelper.Body + "<br><br>";
            }

            {
                HtmlHelper htmlHelper = new HtmlHelper();

                htmlHelper.BeginTable(1);

                // Table header
                htmlHelper.BeginTableRow();
                htmlHelper.AddTableHeader(" ");
                htmlHelper.AddTableHeader("Longest Load Times (secs)", 3);
                htmlHelper.AddTableHeader("Objects", 2);
                htmlHelper.EndTableRow();

                htmlHelper.BeginTableRow();
                htmlHelper.AddTableHeader("Location");
                if (platformList.Count > 1)
                {
                    htmlHelper.AddTableHeader("Platform");
                }
                htmlHelper.AddTableHeader("Total");
                htmlHelper.AddTableHeader("Stalled");
                htmlHelper.AddTableHeader("Fill Streamer");
                htmlHelper.AddTableHeader("Dynamic");
                htmlHelper.AddTableHeader("Physical");
                htmlHelper.EndTableRow();

                List<uint> locDone2 = new List<uint>();

                foreach (var l in locList)
                {
                    if (locDone2.Contains(l.LocationId))
                        continue;

                    if (groupByLocation)
                    {
                        locDone2.Add(l.LocationId);
                    }

                    int iPlatform = 0;
                    foreach (KeyValuePair<string, Platform> kvp in platformList)
                    {
                        // Find all the subsessions for this location
                        var levelLoadInfo = (groupByLocation) ? 
                            subSessionList.FindAll(x => x.LocationId == l.LocationId && x.platform == kvp.Value) : 
                            subSessionList.FindAll(x => x.RouteId == l.RouteId && x.platform == kvp.Value);

                        if (levelLoadInfo.Count > 0)
                        {
                            htmlHelper.BeginTableRow();

                            htmlHelper.AddTableData((iPlatform == 0) ? (groupByLocation) ? l.LocationName : l.RouteName : string.Empty);

                            htmlHelper.TextAlignment = "right";

                            if (platformList.Count > 1)
                            {
                                htmlHelper.AddTableData(kvp.Key);
                            }

                            var rec = levelLoadInfo.Find(x => x.loadingTime == levelLoadInfo.Max(y => y.loadingTime));

                            htmlHelper.AddTableData(HtmlHelper.ColourText(HtmlHelper.BoldText(string.Format("{0:0.00}", rec.loadingTime)), rec.loadingTime > 15 ? red : green));
                            htmlHelper.AddTableData(HtmlHelper.ColourText(HtmlHelper.BoldText(string.Format("{0:0.00}", rec.loadingStall)), rec.loadingStall > (1 / 100) ? red : green));
                            htmlHelper.AddTableData(string.Format("{0:0.00}", rec.fillStreamer));
                            htmlHelper.AddTableData(rec.dynamicObjects.ToString());
                            htmlHelper.AddTableData(rec.physicalObjects.ToString());

                            htmlHelper.EndTableRow();
                        }
                        iPlatform++;
                    }
                }
                htmlHelper.EndTable();

                htmlBody += htmlHelper.Body + "<br><br>";
            }

            string platforms = string.Empty;
            foreach (KeyValuePair<string, Platform> kvp in platformList)
            {
                if (platforms.Length > 0)
                    platforms += "/";

                platforms += kvp.Key;
            }

            // Overview
            htmlBody += "<b>Cases executed (" + subSessions.Count() + " total):</b><br>";
            htmlBody += "Config: <b>" + dsSessions.Tables[0].AsEnumerable().First()["config"] + "</b><br>";
            htmlBody += "Platforms: <b>" + platforms + "</b><br>";
            htmlBody += "BuildNum: <b>" + dsSessions.Tables[0].AsEnumerable().First()["version"] + "</b><br>";
            // VSync: On
            // Present Immediate Threshold: 100

            // Send the actual mail
            Mail mail = new Mail();

            mail.Subject = session + " - Amax Performance Report - " + platforms;
            mail.Sender = "";

            mail.Body = htmlBody;
            mail.IsBodyHtml = true;

            mail.Send();
        }
    }
}
