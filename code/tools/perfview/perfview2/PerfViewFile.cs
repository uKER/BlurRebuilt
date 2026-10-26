using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;

namespace ProfilingApplication
{
	// contains some common file functionality
	[Serializable]
	public class PerfViewFile
	{
		// bool indicating whether the capture has been saved or not
        [NonSerialized()]
		public bool			m_bSaved;
		// original filename of this capture
        [NonSerialized()]
		public string		m_sOriginalFilename;

		public virtual void InitialiseUnserialisables() {}

		public void SaveToFile(Stream stream, string filename)
		{
            BinaryFormatter formatter = new BinaryFormatter();

            formatter.Serialize(stream, this);
            stream.Close();

			m_bSaved = true;
			m_sOriginalFilename = filename;
		}
	}
}
