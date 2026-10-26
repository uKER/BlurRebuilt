using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public partial class AlienBrainInterface
	{
		public delegate void dLoadProjectComplete(object userData, string strResult);

		public string LoadProject(AlienbrainRequest.Priority priority, string strProjectName)
		{
			AlienbrainRequest ar = BeginLoadProject(priority, null, strProjectName, new dLoadProjectComplete(LoadProjectComplete));
			ar.WaitForResult();
			return ar.m_result as string;
		}


		public AlienbrainRequest BeginLoadProject(AlienbrainRequest.Priority priority, object userData, string strProjectName, dLoadProjectComplete myCompleteCallback)
		{
			AlienbrainRequest ar = new AlienbrainRequest(priority, userData, new dLoadProject(DoLoadProject), new object[] { strProjectName }, myCompleteCallback);

			m_queue.AddAbCommand(ar);

			return ar;
		}

		
		void LoadProjectComplete(object userData, string strResult)
		{
		}
	}
}
