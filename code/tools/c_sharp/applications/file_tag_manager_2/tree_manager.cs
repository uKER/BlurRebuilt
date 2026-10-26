using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.IO;
using System.Collections;
using System.Diagnostics;
using Windows.Message;
using Alienbrain;
using System.Xml;

// Life's journey is not to arrive at the grave safely in a well preserved body,but rather to skid in sideways, totally worn out, shouting "...holy sh*t...what a ride! [Riscy]
namespace file_tag_manager_2
{
	class TreeManager : TreeView
	{
		ListViewManager m_listViewManager;

		internal enum TreeObjectState
		{
			PotentiallyUnwanted = 0,
			Active
		};
/*
		protected override void WndProc(ref System.Windows.Forms.Message m)
		{
			const int WM_LBUTTONDBLCLK = 0x203;
			switch(m.Msg)
			{
				case WM.KeyDown:
					return;
			}
			base.WndProc(ref m);
		} 
*/
		protected override bool ProcessDialogKey(System.Windows.Forms.Keys keyData)
		{
			System.Windows.Forms.Keys key = keyData;

			if(key == System.Windows.Forms.Keys.Multiply)
			{
				Debug.WriteLine("Star");
				TreeObject to = this.SelectedNode as TreeObject;

				AddPopulateCommand(new PopulateThreadCommand(to, true, true));

				return true;
			}
			return base.ProcessDialogKey(keyData);
		}

		string m_strPath = null;

		EventWaitHandle m_populateThread_Wait;
		EventWaitHandle m_PopulatingTreeObject_Wait;

		Thread m_populateThread;
		Queue<PopulateThreadCommand> m_PopulateCommandQueue;
		delegate TreeObject dAddTreeObject(TreeObject treeParent, AlienbrainInfo ab_info);
		delegate bool dRemoveTreeObject(TreeObject to);
		delegate bool dExpandTreeObject(TreeObject to);
		TreeObject m_selectedNode = null;
		StatusStrip m_statusStrip = null;

		bool m_bRecurseAll = false;

		public TreeManager() : base()
		{
			InitializeComponent();

			this.TreeViewNodeSorter = new NodeSorter();
		}

		public bool Initialise(StatusStrip statusStrip)
		{
			m_statusStrip = statusStrip;
			m_populateThread_Wait = new EventWaitHandle(false, EventResetMode.AutoReset);
			m_PopulatingTreeObject_Wait = new EventWaitHandle(true, EventResetMode.ManualReset);

			m_populateThread = new Thread(new ThreadStart(PopulateThread));
			m_populateThread.Name = "Populate tree";
			m_populateThread.Priority = ThreadPriority.Normal;
			m_PopulateCommandQueue = new Queue<PopulateThreadCommand>();
			m_populateThread.Start();

			fileTagManager_MainForm.m_alienbrainInterface.Initialise(AlienbrainRequest.Priority.Normal);
			string strRootFolder = fileTagManager_MainForm.m_alienbrainInterface.LoadProject(AlienbrainRequest.Priority.Normal, null);

			if(strRootFolder != "")
				SetRootFolder(strRootFolder);

			return true;
		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (m_populateThread != null)
				m_populateThread.Abort();

			base.Dispose(disposing);
		}

		private void InitializeComponent()
		{
			this.SuspendLayout();
			// 
			// TreeManager
			// 
			this.HideSelection = false;
			this.LineColor = System.Drawing.Color.Black;
			this.BeforeExpand += new System.Windows.Forms.TreeViewCancelEventHandler(this.TreeManager_BeforeExpand);
			this.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.TreeManager_AfterSelect);
			this.AfterExpand += new System.Windows.Forms.TreeViewEventHandler(this.TreeManager_AfterExpand);
			this.ResumeLayout(false);

		}

		/// <summary>
		/// Create a node sorter that implements the IComparer interface.
		/// </summary>
		public class NodeSorter : IComparer
		{
			/// <summary>
			/// Compare two tree nodes by their Text property.
			/// Case insenstive.
			/// </summary>
			/// <param name="x"></param>
			/// <param name="y"></param>
			/// <returns></returns>
			public int Compare(object x, object y)
			{
				TreeNode tx = x as TreeNode;
				TreeNode ty = y as TreeNode;

				// Compare them.
				return string.Compare(tx.Text, ty.Text, true);
			}
		}

#region Populate Thread functions.

		class PopulateThreadCommand
		{
			public PopulateThreadCommand(TreeObject treeParent, bool bRecurseOnce, bool bRecurseAll)
			{
				m_treeParent = treeParent;
				m_bRecurseOnce = bRecurseOnce;
				m_bRecurseAll = bRecurseAll;
			}
			public TreeObject m_treeParent;
			public bool m_bRecurseOnce;
			public bool m_bRecurseAll;
		}

		void PopulateThread()
		{
			while(true)
			{
				m_populateThread_Wait.WaitOne();
				while(m_PopulateCommandQueue.Count != 0)
				{
					PopulateThreadCommand ptc;

					lock(m_PopulateCommandQueue)
					{
						ptc = m_PopulateCommandQueue.Dequeue();
					}

					m_PopulatingTreeObject_Wait.Reset();
					PopulateTreeObject(ptc.m_treeParent, ptc.m_bRecurseOnce, ptc.m_bRecurseAll);
					m_PopulatingTreeObject_Wait.Set();
				}
			}
		}

		internal bool PopulateTreeObject(TreeObject treeParent, bool bRecurseOnce, bool bRecurseAll)
		{
			bool bRetval;

			this.Invoke((MethodInvoker)delegate{BeginUpdate();});


			try
			{
				m_bRecurseAll = bRecurseAll;

				bRetval = _PopulateTreeObject(treeParent, bRecurseOnce);
			}
			catch(Exception e)
			{
				Console.WriteLine("The process failed: {0}", e.ToString());
				MessageBox.Show(null, "Failed to populate the tree.\n\nThe system reports:'" + e.ToString(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Hand);
				bRetval = false;
			}

			this.Invoke((MethodInvoker)delegate { EndUpdate(); });

			return bRetval;
		}

		private void SetStatusText(string strText)
		{
			if(m_statusStrip.InvokeRequired)
			{
				m_statusStrip.Invoke((MethodInvoker)delegate() { m_statusStrip.Items["toolStripStatusLabel"].Text = strText; m_statusStrip.Update(); });
			}
			else
			{
				m_statusStrip.Items["toolStripStatusLabel"].Text = strText;
				m_statusStrip.Update();
			}
		}


		// DO NOT CALL THIS FROM ANYWHERE EXCEPT WITHIN THE PopulateTreeObject FUNCTION.
		internal bool _PopulateTreeObject(TreeObject treeParent, bool bRecurseOnce)
		{

			foreach(TreeObject to in treeParent.Nodes)
			{
				to.m_state = TreeObjectState.PotentiallyUnwanted;
			}



			//foreach(TreeObject to in treeParent.Nodes)
			//{
			//    to.m_state = TreeObjectState.PotentiallyUnwanted;
			//}

			SetStatusText("Getting file list from Alienbrain.");

			int numChildren = fileTagManager_MainForm.m_alienbrainInterface.PrepareToEnumerateChildren(AlienbrainRequest.Priority.Normal, treeParent.m_abInfo.strNamespacePath);

			int statusCount = 0;
			SetStatusText("Enumerating :" + statusCount.ToString());

			AlienbrainInfo abInfo = fileTagManager_MainForm.m_alienbrainInterface.GetNextEnumeratedChild(AlienbrainRequest.Priority.Normal);

			List<TreeObject> childTreeObjects = new List<TreeObject>();
			treeParent.m_listAbFiles.Clear();

			while(abInfo != null)
			{
				TreeObject to = null;
				switch(abInfo.Type)
				{
					case AlienbrainInfo.eNxNBasicNamespaceType.WORKSPACE:
						to = this.Invoke(new dAddTreeObject(AddTreeObject), new object[] { treeParent, abInfo }) as TreeObject;
						childTreeObjects.Add(to);

						break;
					case AlienbrainInfo.eNxNBasicNamespaceType.PROJECT:
						to = this.Invoke(new dAddTreeObject(AddTreeObject), new object[] { treeParent, abInfo }) as TreeObject;
						childTreeObjects.Add(to);

						break;
					case AlienbrainInfo.eNxNBasicNamespaceType.FILE:
						treeParent.m_listAbFiles.Add(abInfo);
						statusCount++;
						if((statusCount % 100) == 0)
							SetStatusText("Enumerating :" + statusCount.ToString());
						break;
					case AlienbrainInfo.eNxNBasicNamespaceType.FOLDER:
						to = this.Invoke(new dAddTreeObject(AddTreeObject), new object[] { treeParent, abInfo }) as TreeObject;
						childTreeObjects.Add(to);
						break;
					case AlienbrainInfo.eNxNBasicNamespaceType.DISKFILE:
						treeParent.m_listAbFiles.Add(abInfo);
						statusCount++;
						if((statusCount % 100) == 0)
							SetStatusText("Enumerating :" + statusCount.ToString());
						break;
					case AlienbrainInfo.eNxNBasicNamespaceType.DISKFOLDER:
						to = this.Invoke(new dAddTreeObject(AddTreeObject), new object[] { treeParent, abInfo }) as TreeObject;
						childTreeObjects.Add(to);
						break;
					case AlienbrainInfo.eNxNBasicNamespaceType.UNKNOWN:
						break;
					default:
						break;
				}

				//abInfo = fileTagManager_MainForm.m_alienbrainInterface.DoGetNextEnumeratedChild();
				abInfo = fileTagManager_MainForm.m_alienbrainInterface.GetNextEnumeratedChild(AlienbrainRequest.Priority.Normal);
			}
			SetStatusText("Enumerating :" + statusCount.ToString());


			if(treeParent == m_selectedNode)
			{
				m_listViewManager.RefreshListView(treeParent);
			}

			foreach(TreeObject to in childTreeObjects)
			{
				//if((bAllowRecurse == true) && (bRecurseOnce == true || m_bRecurseAll == true))
				if((bRecurseOnce == true || m_bRecurseAll == true))
				{
					if(m_bRecurseAll == true)
					{
						this.Invoke((MethodInvoker)delegate() { to.Parent.Expand(); });
					}
					else
					{
						//abInfo = fileTagManager_MainForm.m_alienbrainInterface.GetFirstFolder(AlienbrainRequest.Priority.Normal, to.m_abInfo.strNamespacePath);

						//if(abInfo != null)
						//	to = this.Invoke(new dAddTreeObject(AddTreeObject), new object[] { to, abInfo }) as TreeObject;
					}

						
					////if(_PopulateTreeObject(to.Nodes, to.m_strPath, false) == false)

					// Uncomment this to recurse one level down the tree on selecting a node.
					//if(_PopulateTreeObject(to, false) == false)
					//    return false;
				}
			}

			List<TreeObject> deleteList = new List<TreeObject>(treeParent.Nodes.Count);

			foreach(TreeObject to in treeParent.Nodes)
			{
				if(to.m_state == TreeObjectState.PotentiallyUnwanted)
				{
					deleteList.Add(to);
				}
			}

			foreach(TreeObject to in deleteList)
			{
				this.Invoke((MethodInvoker)delegate() { to.Remove(); });
			}
			return true;
		}

		private TreeObject AddTreeObject(TreeObject treeParent, AlienbrainInfo abInfo)
		{
			TreeObject to;

			string strFullName = abInfo.strName;// +"\\" + ab_info.strName;

			if(treeParent.Nodes.ContainsKey(strFullName))
			{
				to = treeParent.Nodes[strFullName] as TreeObject;
				to.m_state = TreeObjectState.Active;
			}
			else
			{
				to = new TreeObject(abInfo);
				//to.Nodes.Add(new TreeNode("remove me"));
				to.Nodes.Add(new TreeObject(new AlienbrainInfo()));
				//to.Expand();// = 0;
				treeParent.Nodes.Add(to);


				//this.SelectedNode.EnsureVisible();
			}
			return to;
		}

#endregion Populate Thread functions.

		public bool SetRootFolder(string strNamespacePath)
		{
			m_strPath = strNamespacePath;

			m_bRecurseAll = false;

			lock(m_PopulateCommandQueue)
			{
				m_PopulateCommandQueue.Clear();
			}

			this.Nodes.Clear();

			AlienbrainInfo abInfo = fileTagManager_MainForm.m_alienbrainInterface.GetNodeDetails( AlienbrainRequest.Priority.Normal, strNamespacePath, true);

			TreeObject toRoot = new TreeObject(abInfo);
	
			this.Nodes.Add(toRoot);

			this.SelectedNode = this.Nodes[0];
			return true;
		}


		private void TreeManager_AfterSelect(object sender, TreeViewEventArgs e)
		{
			TreeObject to = e.Node as TreeObject;
			this.m_fileTagManager_MainForm.DisableToolstrip();
			m_selectedNode = to;
			//m_selectedNode.Nodes.Clear();
			SelectNewTreeObject(to, true);
		}

		internal bool SelectNewTreeObject(TreeObject to, bool bClearTheListView)
		{
			bool bQueueIsEmpty = false;
			lock(m_PopulateCommandQueue)
			{
				if(m_PopulateCommandQueue.Count == 0)
					bQueueIsEmpty = true;
			}

			if(bQueueIsEmpty == false)					// The 'Populate' thread will refresh the list view if it populates
			{											// the currently viewed tree object, so no need to do it twice.
				Debug.WriteLine("Updating queue");
				//RefreshListView(to, false);			// TODO: Is this necessary?
			}


			// TODO: Push this into the front of the queue?
			AddPopulateCommand(new PopulateThreadCommand(to, true, false));

			m_populateThread_Wait.Set();

			return true;
		}

		private void AddPopulateCommand(PopulateThreadCommand populateThreadCommand)
		{
			lock(m_PopulateCommandQueue)
			{
				m_PopulateCommandQueue.Enqueue(populateThreadCommand);
			}
			m_populateThread_Wait.Set();
		}


		private void TreeManager_BeforeExpand(object sender, TreeViewCancelEventArgs e)
		{
		}

		private void TreeManager_AfterExpand(object sender, TreeViewEventArgs e)
		{
			TreeObject to = e.Node as TreeObject;

			if(m_PopulatingTreeObject_Wait.WaitOne(0, false) == false)
			{
				Debug.WriteLine("Quietly Expanding " + to.FullPath);
			}
			else
			{
				AddPopulateCommand(new PopulateThreadCommand(to, true, false));
				m_populateThread_Wait.Set();
			}
		}

		internal void SetListView(ListViewManager listViewManager)
		{
			m_listViewManager = listViewManager;
		}

		internal void KillThread()
		{
			this.m_populateThread.Abort();
		}

		fileTagManager_MainForm m_fileTagManager_MainForm;
		internal void SetMainForm(fileTagManager_MainForm fileTagManager_MainForm)
		{
			m_fileTagManager_MainForm = fileTagManager_MainForm;
		}


	}
}
