using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;
using System.IO;
using System.Windows.Forms;

namespace ProfilerTool.Classes
{
    [XmlRoot("UserSettings")]
    public class Settings
    {
        [XmlElement("ConnectionInfo")]
        public Comms.Info connection = new Comms.Info();

        public static Settings Load()
        {
            Settings settings;

            try
            {
                XmlSerializer serializer = new XmlSerializer(typeof(Settings));
                StreamReader reader = new StreamReader(Directory.GetCurrentDirectory() + "\\settings.xml");
                settings = (Settings)serializer.Deserialize(reader);
                reader.Close();
            }
            catch (System.IO.IOException)
            {
                settings = new Settings();
            }

            return settings;
        }

        public static void Save(Settings settings)
        {
            try
            {
                XmlSerializer serializer = new XmlSerializer(typeof(Settings));
                StreamWriter writer = new StreamWriter(Directory.GetCurrentDirectory() + "\\settings.xml");
                serializer.Serialize(writer, settings);
            }
            catch (System.IO.IOException)
            {
                MessageBox.Show("Could not save settings", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
