using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using System.Windows.Controls.DataVisualization.Charting;

using MySql.Data;
using MySql.Data.MySqlClient;

namespace PerfStatsViewer
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        private string idRange = string.Empty;
        private uint locationId = 0;
        private uint routeId = 0;

        static string connectionString = string.Format("Database=information_schema;Data Source={0};User Id={1};Password={2}", "localhost", "", "");

        class PerfInfo
        {
            public float FrameTime { get; set; }
            public float FrameLength { get; set; }

            public uint FreeMemory { get; set; }
            public uint CmdBuffSize { get; set; }
            
            public float CpuTotal { get; set; }
            public float CpuUpdate { get; set; }
            public float CpuRender { get; set; }

            public float AiUpdate { get; set; }
            public float PhysicsUpdate { get; set; }
            public float UiUpdate { get; set; }
            public float ActorUpdate { get; set; }
            public float LevelSceneUpdate { get; set; }

            public uint StreamRequested { get; set; }
            public uint StreamLoaded { get; set; }
            
            public float GpuTotal { get; set; }
        }

        struct BuildInfo
        {
            public string Version { get; set; }
            public DateTime Time { get; set; }
            public bool XBox360 { get; set; }
            public bool PS3 { get; set; }
        }

        class SeriesInfo
        {
            public string Name { get; set; }
            public uint Max { get; set; }
        }

        public Window1()
        {
            InitializeComponent();

            {
                MySql.Data.MySqlClient.MySqlConnection conn = new MySql.Data.MySqlClient.MySqlConnection(connectionString);

                conn.Open();

                // Find session to dump stats on
                {
                    MySqlDataAdapter daLastSession = new MySqlDataAdapter("SELECT ref,time,platform FROM amaxperf._sessions ORDER BY time DESC", conn);
                    System.Data.DataSet dsLastSession = new System.Data.DataSet();
                    daLastSession.Fill(dsLastSession);

                    //var hack = from x in dsLastSession.Tables[0].AsEnumerable() where x.Field<string>("platform") == "360" select x;
                    var hack = from x in dsLastSession.Tables[0].AsEnumerable() group x by x.Field<string>("ref") into g select new { key = g.Key, rest = g };

                    List<BuildInfo> buildInfo = new List<BuildInfo>();

                    foreach (var x in hack)
                    {
                        BuildInfo bi = new BuildInfo() { Version = x.key };

                        bi.Time = (x.rest.ElementAt(0) as DataRow).Field<DateTime>("time");

                        foreach (var y in x.rest)
                        {
                            switch (y.ItemArray[2] as string)
                            {
                                case "360":
                                    bi.XBox360 = true;
                                    break;
                                case "PS3":
                                    bi.PS3 = true;
                                    break;
                            }
                        }

                        buildInfo.Add(bi);
                    }

                    dataGrid1.DataContext = buildInfo; // hack; // .AsDataView(); //  dsLastSession.Tables[0];
                    //System.Diagnostics.Debug.Assert(dsLastSession.Tables[0].AsEnumerable().Count() == 1);

                    //var lastSession = dsLastSession.Tables[0].AsEnumerable().First();

                    //var session = (string)lastSession["ref"];
                }
            }
        }

        //void workertranaction_RunWorkerCompleted(object sender, System.ComponentModel.RunWorkerCompletedEventArgs e)
        //{
        //    //Call method to hide wait screen
        //}

        //void workertranaction_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
        //{
        //    //My Expansive call will go here...
        //}

        private string IdCondition(List<uint> vals)
        {
            uint startRange = uint.MaxValue;
            uint endRange = uint.MaxValue;
            bool isRange = true;

            string inString = string.Empty;

            // Find range of sessions
            foreach (uint v in vals)
            {
                if (inString.Length > 0)
                {
                    inString += ",";
                }

                if (startRange == uint.MaxValue)
                {
                    startRange = endRange = v;
                }
                else
                {
                    if (v != endRange + 1)
                    {
                        isRange = false;
                    }
                    else
                    {
                        endRange = v;
                    }
                }

                inString += v;
            }

            if (isRange)
            {
                return "BETWEEN " + startRange + " AND " + endRange;
            }

            return "IN (" + inString + ")";
        }

        private void dataGrid1_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            MySql.Data.MySqlClient.MySqlConnection conn = new MySql.Data.MySqlClient.MySqlConnection(connectionString);

            conn.Open();

            //MySqlDataAdapter daStrings = new MySqlDataAdapter("SELECT * FROM amaxperf._strings", conn);
            //System.Data.DataSet dsStrings = new System.Data.DataSet();
            //daStrings.Fill(dsStrings);

            BuildInfo bi = (BuildInfo)e.AddedItems[0];

            // Query the ID's of all the sessions in this perf run
            this.idRange = string.Empty;
            {
                MySqlDataAdapter daSessions = new MySqlDataAdapter("SELECT _id FROM amaxperf._sessions WHERE amaxperf._sessions.ref = '" + bi.Version + "' ORDER BY _id", conn);
                System.Data.DataSet dsSessions = new System.Data.DataSet();
                daSessions.Fill(dsSessions);

                List<uint> values = new List<uint>();

                foreach (var b in dsSessions.Tables[0].AsEnumerable())
                {
                    values.Add(uint.Parse(b["_id"].ToString()));
                }

                this.idRange = IdCondition(values);
            }

            //Webby.NavigateToString(" ");

            //System.ComponentModel.BackgroundWorker workertranaction = new System.ComponentModel.BackgroundWorker();
            //workertranaction.DoWork += new System.ComponentModel.DoWorkEventHandler(workertranaction_DoWork);
            //workertranaction.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(workertranaction_RunWorkerCompleted);
            //workertranaction.RunWorkerAsync();
            
            //// Let's try extracting all the data for the sessions
            //{
            //    // List of all subsessions
            //    string ssIdRange = string.Empty;
            //    {
            //        MySqlDataAdapter daSubSessions = new MySqlDataAdapter("SELECT ss._id FROM amaxperf._subsessions AS ss WHERE ss._sessionid " + this.idRange + " AND ss.end IS NOT NULL", conn);
            //        System.Data.DataSet dsSS = new System.Data.DataSet();
            //        daSubSessions.Fill(dsSS);

            //        // Find range of sessions
            //        foreach (var b in dsSS.Tables[0].AsEnumerable())
            //        {
            //            if (ssIdRange.Length > 0)
            //            {
            //                ssIdRange += ",";
            //            }

            //            ssIdRange += b["_id"];
            //        }
            //    }

            //    // Now we query all the perf stats for these subsessions
            //    MySqlDataAdapter daStats = new MySqlDataAdapter("SELECT * FROM amaxperf.perfstat WHERE amaxperf.perfstat._subsessionid IN(" + ssIdRange + ")", conn);
            //    System.Data.DataSet ds = new System.Data.DataSet();
            //    daStats.Fill(ds);

            //}

            // Extract list of unique locations from the session
            {
                MySqlDataAdapter daLocations = new MySqlDataAdapter("SELECT locstr.name AS name,ss.LocationId AS _id FROM amaxperf._subsessions AS ss LEFT JOIN amaxperf._sessions AS s ON ss._sessionid = s._id LEFT JOIN amaxperf._strings AS locstr ON ss.LocationId = locstr._id WHERE ss._sessionid " + this.idRange + " AND ss.end IS NOT NULL GROUP BY name", conn);
                System.Data.DataSet ds = new System.Data.DataSet();
                daLocations.Fill(ds);

                dataGridLocation.DataContext = ds.Tables[0];
            }
        }

        private void dataGridLocation_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                MySql.Data.MySqlClient.MySqlConnection conn = new MySql.Data.MySqlClient.MySqlConnection(connectionString);

                conn.Open();

                this.locationId = (uint)(e.AddedItems[0] as DataRowView).Row.ItemArray[1];

                MySqlDataAdapter daRoutes = new MySqlDataAdapter("SELECT routestr.name AS name,ss.RouteId FROM amaxperf._subsessions AS ss LEFT JOIN amaxperf._sessions AS s ON ss._sessionid = s._id LEFT JOIN amaxperf._strings AS routestr ON ss.RouteId = routestr._id WHERE ss._sessionid " + this.idRange + " AND ss.LocationId=" + locationId + "  AND ss.end IS NOT NULL GROUP BY name", conn);
                System.Data.DataSet ds = new System.Data.DataSet();
                daRoutes.Fill(ds);

                dataGridRoute.DataContext = ds.Tables[0];
            }
            else
            {
                this.locationId = 0;
                dataGridRoute.DataContext = null;
            }
        }


        private void dataGridRoute_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                MySql.Data.MySqlClient.MySqlConnection conn = new MySql.Data.MySqlClient.MySqlConnection(connectionString);
                conn.Open();

                this.routeId = (uint)(e.AddedItems[0] as DataRowView).Row.ItemArray[1];

                // Find subsessions that match the sessions
                MySqlDataAdapter daPerfStats = new MySqlDataAdapter("SELECT ss._id,s.platform,ss.NumCars,routestr.name,ss.LoadingTime,ss.DynamicObjects,ss.PhysicalObjects FROM amaxperf._subsessions AS ss LEFT JOIN amaxperf._sessions AS s ON ss._sessionid = s._id LEFT JOIN amaxperf._strings AS routestr ON ss.RouteId = routestr._id WHERE ss._sessionid " + this.idRange + " AND ss.RouteId=" + this.routeId + "  AND ss.end IS NOT NULL ORDER BY routestr.name,ss.NumCars", conn);
                System.Data.DataSet ds = new System.Data.DataSet();
                daPerfStats.Fill(ds);

                dataGrid2.DataContext = ds.Tables[0];
            }
            else
            {
                this.routeId = 0;
                dataGrid2.DataContext = null;
            }
        }

        private void CalcMaxInSeries(LineSeries ls, System.Collections.IEnumerable dataSeries, string dependentValue)
        {
            uint max = 0;

            if (dependentValue == null)
            {
                SeriesInfo si = (SeriesInfo)ls.Tag;

                dependentValue = si.Name;
            }

            System.Reflection.PropertyInfo pi = typeof(PerfInfo).GetProperty(dependentValue);

            foreach (PerfInfo v in dataSeries)
            {
                var val = pi.GetValue(v, null);

                uint uv = Convert.ToUInt32(val);

                if (uv > max)
                {
                    max = uv;
                }
            }

            if (ls.Tag != null)
            {
                SeriesInfo si = (SeriesInfo)ls.Tag;

                si.Max = max + 1;
            }
            else
            {
                ls.Tag = new SeriesInfo() { Name = dependentValue, Max = max + 1 };
            }

            //System.Diagnostics.Debug.WriteLine("CalcMax: " + dependentValue + ": " + ((SeriesInfo)ls.Tag).Max);
        }

        private void AddLineSeries(Chart chart, string title, string independantValue, string dependentValue, Color colour, System.Collections.IEnumerable dataSeries)
        {
            LineSeries ls1 = new LineSeries();
            ls1.Title = title;
            ls1.IndependentValueBinding = new Binding(independantValue);
            ls1.DependentValueBinding = new Binding(dependentValue);
            {
                Style style = new Style(typeof(Control));
                style.Setters.Add(new Setter(Control.VisibilityProperty, Visibility.Collapsed));
                style.Setters.Add(new Setter(Control.BackgroundProperty, new SolidColorBrush(colour)));
                ls1.DataPointStyle = style;
                ls1.LegendItemStyle = (Style)Resources["LegendItemStyle"];
                ls1.TransitionDuration = TimeSpan.FromSeconds(0);
            }

            CalcMaxInSeries(ls1, dataSeries, dependentValue);

            chart.Series.Add(ls1);
            ls1.ItemsSource = dataSeries;
        }

        private void RecalcAxes(Chart chart)
        {
            uint maxV = 0;

            System.Diagnostics.Debug.WriteLine("Recalculate maximum...");

            foreach (LineSeries mls in chart.Series)
            {
                if (mls.Visibility == Visibility.Visible)
                {
                    SeriesInfo si = (SeriesInfo)mls.Tag;

                    System.Diagnostics.Debug.WriteLine("Series: " + mls.Title + ": " + si.Max);

                    if (si.Max > maxV)
                        maxV = si.Max;
                }
            }

            System.Diagnostics.Debug.WriteLine("... Max is " + maxV);

            LinearAxis linax = chart.Axes[1] as LinearAxis;

            linax.Maximum = maxV;
        }

        private void dataGrid2_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                uint subSessionId = (uint)(e.AddedItems[0] as DataRowView).Row.ItemArray[0];

                MySql.Data.MySqlClient.MySqlConnection conn = new MySql.Data.MySqlClient.MySqlConnection(connectionString);

                conn.Open();

                MySqlDataAdapter daPS = new MySqlDataAdapter("SELECT * FROM amaxperf.perfstat WHERE amaxperf.perfstat._subsessionid = " + subSessionId, conn);

                System.Data.DataSet dsp = new System.Data.DataSet();
                daPS.Fill(dsp);

                System.Data.DataTable dtp = dsp.Tables[0];

                List<PerfInfo> perfInfo = new List<PerfInfo>();

                foreach (var x in dtp.AsEnumerable())
                {
                    PerfInfo pi = new PerfInfo();

                    pi.FrameTime = x.Field<float>("FrameTime");
                    pi.FrameLength = x.Field<float>("FrameLength");

                    pi.FreeMemory = x.Field<uint>("FreeMemory") / 1024;
                    pi.CmdBuffSize = x.Field<uint>("CmdBuffSize") / 1024;

                    pi.GpuTotal = x.Field<float>("GpuTotal");
                    
                    pi.CpuTotal = x.Field<float>("CpuTotal");
                    pi.CpuUpdate = x.Field<float>("CpuUpdate");
                    pi.CpuRender = x.Field<float>("CpuRender");

                    pi.AiUpdate = x.Field<float>("AiUpdate");
                    pi.PhysicsUpdate = x.Field<float>("PhysicsUpdate");
                    pi.UiUpdate = x.Field<float>("UiUpdate");
                    pi.ActorUpdate = x.Field<float>("ActorUpdate");
                    pi.LevelSceneUpdate = x.Field<float>("LevelSceneUpdate");
                    
                    pi.StreamRequested = x.Field<uint>("StreamRequested");
                    pi.StreamLoaded = x.Field<uint>("StreamLoaded");

                    perfInfo.Add(pi);
                }

                System.Diagnostics.Debug.WriteLine("Performance samples: " + perfInfo.Count);

                if (chart1.Series.Count == 0)
                {
                    AddLineSeries(chart1, "Frame Length", "FrameTime", "FrameLength", Colors.Brown, perfInfo);
                    AddLineSeries(chart1, "GPU Total", "FrameTime", "GpuTotal", Colors.Green, perfInfo);

                    AddLineSeries(chart1, "CPU Update", "FrameTime", "CpuUpdate", Colors.Blue, perfInfo);
                    AddLineSeries(chart1, "CPU Render", "FrameTime", "CpuRender", Colors.Red, perfInfo);
                }
                else
                {
                    foreach (LineSeries ls in chart1.Series)
                    {
                        ls.ItemsSource = perfInfo;

                        CalcMaxInSeries(ls, perfInfo, null);
                    }
                }

                if (chartCpu.Series.Count == 0)
                {
                    AddLineSeries(chartCpu, "CPU Update", "FrameTime", "CpuUpdate", Colors.Blue, perfInfo);
                    AddLineSeries(chartCpu, "Actor Update", "FrameTime", "ActorUpdate", Colors.Red, perfInfo);
                    AddLineSeries(chartCpu, "Physics Update", "FrameTime", "PhysicsUpdate", Colors.Orange, perfInfo);
                    AddLineSeries(chartCpu, "AI Update", "FrameTime", "AiUpdate", Colors.Green, perfInfo);
                    AddLineSeries(chartCpu, "UI Update", "FrameTime", "UiUpdate", Colors.SaddleBrown, perfInfo);
                    AddLineSeries(chartCpu, "Level Scene Update", "FrameTime", "LevelSceneUpdate", Colors.DeepPink, perfInfo);
                }
                else
                {
                    foreach (LineSeries ls in chartCpu.Series)
                    {
                        ls.ItemsSource = perfInfo;
                        CalcMaxInSeries(ls, perfInfo, null);
                    }
                }

                if (chartStreaming.Series.Count == 0)
                {
                    AddLineSeries(chartStreaming, "Stream Requested", "FrameTime", "StreamRequested", Colors.DodgerBlue, perfInfo);
                    AddLineSeries(chartStreaming, "Stream Loaded", "FrameTime", "StreamLoaded", Colors.Firebrick, perfInfo);
                }
                else
                {
                    foreach (LineSeries ls in chartStreaming.Series)
                    {
                        ls.ItemsSource = perfInfo;
                        CalcMaxInSeries(ls, perfInfo, null);
                    }
                }

                if (chartMemory.Series.Count == 0)
                {
                    AddLineSeries(chartMemory, "Free Memory", "FrameTime", "FreeMemory", Colors.DodgerBlue, perfInfo);
                    AddLineSeries(chartMemory, "Command Buffer Usage", "FrameTime", "CmdBuffSize", Colors.Red, perfInfo);
                }
                else
                {
                    foreach (LineSeries ls in chartMemory.Series)
                    {
                        ls.ItemsSource = perfInfo;
                        CalcMaxInSeries(ls, perfInfo, null);
                    }
                }

                RecalcAxes(chart1);
                RecalcAxes(chartCpu);
                RecalcAxes(chartStreaming);
                RecalcAxes(chartMemory);

                chart1.Visibility = Visibility.Visible;
                chartCpu.Visibility = Visibility.Visible;
                chartStreaming.Visibility = Visibility.Visible;
                chartMemory.Visibility = Visibility.Visible;
            }
            else
            {
                chart1.Visibility = Visibility.Hidden;
                chartCpu.Visibility = Visibility.Hidden;
                chartStreaming.Visibility = Visibility.Hidden;
                chartMemory.Visibility = Visibility.Hidden;
            }
        }

        private void CheckBox_Click(object sender, RoutedEventArgs e)
        {
            CheckBox chk = (sender as CheckBox);

            Chart chart = null;

            switch (MrTabby.SelectedIndex)
            {
                case 0:
                    chart = chart1;
                    break;
                case 1:
                    chart = chartCpu;
                    break;
                case 2:
                    chart = chartStreaming;
                    break;
                case 3:
                    chart = chartMemory;
                    break;
            }

            LineSeries ls = chart.Series.Cast<LineSeries>().Where(s => s.Title.ToString() == chk.Tag.ToString()).ElementAtOrDefault(0);
            
            if (chk.IsChecked.Value)
            {
                chk.Opacity = 1;
                ls.Opacity = 1;
                ls.Visibility = Visibility.Visible;
            }
            else
            {
                chk.Opacity = 0.5;
                ls.Opacity = 0;
                ls.Visibility = Visibility.Collapsed;
            }

            RecalcAxes(chart);
        }
    }
}
