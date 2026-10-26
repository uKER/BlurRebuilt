using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dImportComplete(object userData, bool result);

		public bool Import(AlienbrainRequest.Priority priority, string strLocalFile)
		{
			AlienbrainRequest ar = BeginImport(priority, null, strLocalFile, new dImportComplete(ImportComplete));
			ar.WaitForResult();
			return (bool)ar.m_result;
		}


		public AlienbrainRequest BeginImport(AlienbrainRequest.Priority priority, object userData, string strLocalFile, dImportComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dImport(DoImport), new object[] { strLocalFile }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}


		void ImportComplete(object userData, bool result)
		{
		}
	}
}
