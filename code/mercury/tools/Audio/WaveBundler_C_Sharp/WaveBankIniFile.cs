using System;
using System.Xml;
using System.Xml.Serialization;
using System.Collections;
using System.IO;

namespace WaveBundler
{
	// an ini file reader class. reads the xml into a WaveBankIniFile class
	public class WaveBankIniFileReader
	{
		public static WaveBankIniFile Read(string fileName)
		{
			XmlSerializer s = new XmlSerializer( typeof( WaveBankIniFile ) );
			WaveBankIniFile iniFile = new WaveBankIniFile(fileName);
			TextReader r;

			// try to open the xml file
			try
			{
				r = new StreamReader( fileName );
			}
			catch (Exception e)
			{
				Console.WriteLine("\n\nError:  \"{0}\"\n", e.GetBaseException().Message);
				return null;
			}

			// try to read the xml data into our WaveBankIniFile class
			try
			{
				iniFile = (WaveBankIniFile)s.Deserialize( r );
			}
			catch (Exception e)
			{
				Console.WriteLine("\n\nError:  \"{0}\"\n", e.GetBaseException().Message);
				return null;
			}

			r.Close();

			return iniFile;
		}
	}

	[XmlRoot("wavebank")]
	public class WaveBankIniFile 
	{
		[XmlAttribute("name")] public string name;

		private ArrayList listWaves;
			
		protected string sFileName;

		public WaveBankIniFile(string fName) 
		{
			listWaves = new ArrayList();
			sFileName = fName;
		}
		public WaveBankIniFile() 
		{
			listWaves = new ArrayList();
		}

		[XmlElement("wave")]
		public WaveElement[] Waves 
		{
			get 
			{
				WaveElement[] waves = new WaveElement[ listWaves.Count ];
				listWaves.CopyTo( waves );
				return waves;
			}
			set 
			{
				if( value == null ) return;
				WaveElement[] waves = (WaveElement[])value;
				listWaves.Clear();
				foreach( WaveElement wave in waves )
					listWaves.Add( wave );
			}
		}
/*
		// this doesnt seem to be used?
		public int AddItem( WaveElement waves ) 
		{
			return listWaves.Add( waves );
		}
	*/
//
//		public static string GetKeyValue(XmlNode node, string strKey)
//		{
//			string strRetval = null;
//			if(node != null)
//			{
//				XmlNode attr = node.Attributes.GetNamedItem(strKey);
//				if(attr != null)
//				{
//					strRetval = attr.Value;
//				}
//			}
//			return strRetval;
//		}
	}

	public class WaveElement 
	{
		[XmlAttribute("filename")] public string filename;
		[XmlAttribute("internalName")] public string internalName;

		public WaveElement() {	}

		public WaveElement( string fname, string intname ) 
		{
			filename = fname;
			internalName = intname;
		}
	}

	
}