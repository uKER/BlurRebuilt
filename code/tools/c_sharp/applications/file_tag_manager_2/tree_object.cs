using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Alienbrain;

namespace file_tag_manager_2
{
	public class TreeObject : TreeNode
	{
		internal TreeManager.TreeObjectState m_state;
		//public string m_strPath;
		public bool m_bDirtyChildren;

		public AlienbrainInfo m_abInfo;

		public List<AlienbrainInfo> m_listAbFiles;

		public TreeObject(AlienbrainInfo abInfo)
			: base(abInfo.strName)
		{
			m_listAbFiles = new List<AlienbrainInfo>();

			this.Name = abInfo.strName;

			this.m_abInfo = abInfo;

			this.Text = abInfo.strName;

			m_state = TreeManager.TreeObjectState.Active;
			m_bDirtyChildren = true;
		}
	}
}

