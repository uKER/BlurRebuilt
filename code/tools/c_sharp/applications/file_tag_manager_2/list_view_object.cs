using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using Alienbrain;
using System.Diagnostics;

namespace file_tag_manager_2
{
	// A listViewObject is a ListViewItem with some extra fields.
	public class ListViewObject : ListViewItem
	{
		/// <summary>
		/// This is a refeence to the template used by the FileTagManager.
		/// </summary>
		public XmlElement m_xTemplate = null;
		public XmlElement m_xFileData = null;// *GONE* ;-)

		/// <summary>
		/// This is the tagdata document associated with the current listViewObject.
		/// It is null if there is not yet any associated data.
		/// </summary>
		public XmlDocument m_xTagDataDocument = null;

		/// <summary>
		/// This contains all the Alienbrain information associated with this ListViewItem
		/// </summary>
		public AlienbrainInfo m_abInfo;

		/// <summary>
		/// Constructor for the basic type in our list view.
		/// </summary>
		/// <param name="abInfo">A class reference describing the Alienbrain file this object is associated with.</param>
		public ListViewObject(AlienbrainInfo abInfo)
			: base()
		{
			Debug.Assert(abInfo != null);

			m_abInfo = abInfo;
			
			this.Text = abInfo.strName;
			this.Name = abInfo.strLocalPath;
		}
	}
}
