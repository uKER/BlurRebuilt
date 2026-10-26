using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml.Serialization;
using System.Collections.Specialized;
using System.Windows.Forms;
using System.Drawing;

namespace ProfilingApplication
{
	enum DimensionValues
	{
		Thread_Name,
		Thread_Start,
		Thread_End,
		Thread_Duration,
		Timing_Name,
		Timing_Start,
		Timing_End,
		Timing_Duration,
        Timing_CacheUsed,
		Capture_HSplit,
		Capture_VSplit,
		Hierarchy_Name,
		Hierarchy_Hits,
		Hierarchy_Time,
		Hierarchy_Ratio,
		Alloc_Name,
		Alloc_Start,
		Alloc_End,
		Alloc_Size,
		Alloc_Tag,
		Callstack_Name,
		Callstack_File,
		Callstack_Line,
		FragView_HSplit,
		FragView_VSplit,
		MemNode_Name,
		MemNode_Allocs,
		MemNode_Memory,
		MemNode_Wasted,
		MemNode_Percent,
        Timing_CacheWasted,
        Max,
	};

	// user settings for a single active counter
	[XmlRoot("CounterConfig")]
	public struct CounterConfig
	{
		public string sName;
		public bool bActive;
		public float fScale;
		public int r, g, b;
	};

	// class containing a custom counter definition
	[XmlRoot("CustomCounter")]
	public class CustomCounter
	{
		public string sName;
		public string sExpression;
	};

	// class containing a colour
	[XmlRoot("XmlColour")]
	public struct Colour
	{
		public Colour(int r, int g, int b)
		{
			R = r;
			G = g;
			B = b;
		}

		public int R, G, B;
	};

	// class containing graphview configuration
	[XmlRoot("GraphViewConfig")]
	public class GraphViewConfig
	{
		// default constructor
		public GraphViewConfig()
		{
			bgDark = new Colour(0,0,0);
			bgLight = new Colour(50,48,80);
			gridLight = new Colour(51,102,255);
			gridDark = new Colour(51,51,153);
			bGridOn = true;
			bAutoScale = false;
			bMultipleGraphs = false;
			fMin = 0;
			fMax = 100;
			iHighlightEvery = 5;
			iGridSpacing = 20;
			iUpdatesPerSec = 10;
			iMaxValues = 200;
			bMargin = false;
		}

		public Colour bgDark, bgLight, gridDark, gridLight;
		public bool bGridOn;
		public bool bAutoScale;
		public bool bMultipleGraphs;
		public int iGridSpacing;
		public int iHighlightEvery;
		public float fMin, fMax;		
		public int iUpdatesPerSec;
		public int iMaxValues;
		public bool bMargin;
	};

	// class containing the current counter properties configuration
	[XmlRoot("CounterPropertiesConfig")]
	public class CounterPropertiesConfig
	{
		public List<CounterConfig> m_ActiveCounters = new List<CounterConfig>();
		public List<CustomCounter> m_CustomCounters = new List<CustomCounter>();

		public CounterPropertiesConfig Copy()
		{
			CounterPropertiesConfig cp = new CounterPropertiesConfig();
			cp.m_ActiveCounters = new List<CounterConfig>(m_ActiveCounters);
			cp.m_CustomCounters = new List<CustomCounter>(m_CustomCounters);
			return cp;
		}
	};

    [XmlRoot("UserSettings")]
    public class UserSettings
    {
        public struct ColourSetting
        {
            public string name;
            public int iColour;
        };

		public UserSettings()
		{
			ColourSetting tmp;
			tmp.name = "[$root]";
			tmp.iColour = 2;
			m_ColourSettingsList.Add(tmp);
			InitDimensions();

			m_ColourSettings["[$root]"] = 2;
		}

		public void InitDimensions()
		{
			m_iDimensions = new int[(int)DimensionValues.Max];

			m_iDimensions[(int)DimensionValues.Thread_Name] = 203;
			m_iDimensions[(int)DimensionValues.Thread_Start] = 60;
			m_iDimensions[(int)DimensionValues.Thread_End] = 60;
			m_iDimensions[(int)DimensionValues.Thread_Duration] = 60;
			m_iDimensions[(int)DimensionValues.Capture_VSplit] = 380;
			m_iDimensions[(int)DimensionValues.Capture_HSplit] = 263;
			m_iDimensions[(int)DimensionValues.Timing_Name] = 300;
			m_iDimensions[(int)DimensionValues.Timing_Start] = 60;
			m_iDimensions[(int)DimensionValues.Timing_End] = 60;
			m_iDimensions[(int)DimensionValues.Timing_Duration] = 60;
			m_iDimensions[(int)DimensionValues.Timing_CacheUsed] = 100;
            m_iDimensions[(int)DimensionValues.Timing_CacheWasted] = 100;
            m_iDimensions[(int)DimensionValues.Hierarchy_Name] = 300;
			m_iDimensions[(int)DimensionValues.Hierarchy_Hits] = 60;
			m_iDimensions[(int)DimensionValues.Hierarchy_Time] = 60;
			m_iDimensions[(int)DimensionValues.Hierarchy_Ratio] = 60;
			m_iDimensions[(int)DimensionValues.Alloc_Name] = 139;
			m_iDimensions[(int)DimensionValues.Alloc_Start] = 81;
			m_iDimensions[(int)DimensionValues.Alloc_End] = 73;
			m_iDimensions[(int)DimensionValues.Alloc_Size] = 66;
			m_iDimensions[(int)DimensionValues.Alloc_Tag] = 33;
			m_iDimensions[(int)DimensionValues.Callstack_Name] = 202;
			m_iDimensions[(int)DimensionValues.Callstack_File] = 137;
			m_iDimensions[(int)DimensionValues.Callstack_Line] = 45;
			m_iDimensions[(int)DimensionValues.FragView_VSplit] = 420;
			m_iDimensions[(int)DimensionValues.FragView_HSplit] = 372;
			m_iDimensions[(int)DimensionValues.MemNode_Name] = 250;
			m_iDimensions[(int)DimensionValues.MemNode_Allocs] = 80;
			m_iDimensions[(int)DimensionValues.MemNode_Memory] = 80;
			m_iDimensions[(int)DimensionValues.MemNode_Wasted] = 80;
			m_iDimensions[(int)DimensionValues.MemNode_Percent] = 80;
		}

        public static UserSettings Load(string filename)
        {
            XmlSerializer xs = new XmlSerializer(typeof(UserSettings));
            UserSettings us;

            // load data
            using (StreamReader sr = new StreamReader(filename))
            {
                us = (UserSettings)xs.Deserialize(sr);
            }

            // convert list to dictionary
            us.m_ColourSettings.Clear();
            foreach (ColourSetting x in us.m_ColourSettingsList)
                us.m_ColourSettings[x.name] = x.iColour;

			if (us.m_iDimensions == null || us.m_iDimensions.Length < ((int)DimensionValues.Max))
				us.InitDimensions();

            return us;
        }

        public static void Save(string filename, UserSettings settings)
        {
            XmlSerializer xs = new XmlSerializer(typeof(UserSettings));

            // convert dictionary to list
            settings.m_ColourSettingsList.Clear();
            foreach (KeyValuePair<string, int> x in settings.m_ColourSettings)
            {
                ColourSetting tmp;
                tmp.name = x.Key;
                tmp.iColour = x.Value;
                settings.m_ColourSettingsList.Add(tmp);
            }

            // save data
            using (StreamWriter sw = new StreamWriter(filename))
            {
                xs.Serialize(sw, settings);
            }
        }

        [XmlElement("MainAppState")]
        public FormWindowState m_MainAppState;

        [XmlElement("WindowPos")]
        public Point m_MainAppLocation;

        [XmlElement("WindowSize")]
        public Size m_MainAppSize;

        [XmlElement("ConnectionInfo")]
        public ConnectionMethodInfo m_ConnectInfo = new ConnectionMethodInfo();

        [XmlElement("DimensionValues")]
		public int[] m_iDimensions = new int[(int)DimensionValues.Max];

        [XmlIgnore]
        public Dictionary<string,int> m_ColourSettings = new Dictionary<string,int>();

        [XmlElement("ColourSettings")]
        public List<ColourSetting> m_ColourSettingsList = new List<ColourSetting>();

		[XmlElement("CounterPropertiesConfig")]
		public CounterPropertiesConfig m_CounterPropertiesConfig = new CounterPropertiesConfig();

        [XmlElement("GraphViewConfig")]
        public GraphViewConfig m_GraphViewConfig = new GraphViewConfig();

        [XmlElement("CustomConnectionConfig")]
        public CustomLastConnectionInfo m_CustomConnectionConfig = new CustomLastConnectionInfo();
    }
}
