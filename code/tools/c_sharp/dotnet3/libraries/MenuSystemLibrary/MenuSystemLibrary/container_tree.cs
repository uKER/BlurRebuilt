using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Collections;
using Global;
using System.Diagnostics;

namespace MenuSystemLibrary
{
	public partial class Container_Tree : MenuSystemLibrary.Container_Base
	{
		public Container_Tree()
		{
			InitializeComponent();
		}

		public Container_Tree(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();
			
			treeView.PathSeparator = "\\";

			string strValue = XmlHelper.GetKeyValue(xContainer, "value");

			BuildTree(xContainer, treeView.Nodes);

			// The 'value' of this container may have changed due to the validation above,
			// so it must be written back to the xml data for this container.
//			XmlHelper.AddAttribute(m_xObject, "value", strSelectedItem);
		}

		bool BuildTree(XmlNode xContainer, TreeNodeCollection tnc)
		{
			// Add every child of type 'container' to this combobox.
			foreach(XmlNode xChild in xContainer.ChildNodes)
			{
				if(xChild.Name == "container")
				{
					string strContainerType = XmlHelper.GetKeyValue(xChild, "type");
					if(strContainerType == "tree_node")
					{
//						string strName = XmlHelper.GetKeyValue(xChild, "name");
						string strName = XmlHelper.GetKeyValue(xChild, "display_name");
						if(strName == null)
							strName = XmlHelper.GetKeyValue(xChild, "name");

						TreeNode newNode = new TreeNode(strName);
						newNode.Tag = xChild;
						tnc.Add(newNode);
						BuildTree(xChild, newNode.Nodes);
					}
				}
			}
			return true;
		}

		/*
		XmlElement SetXmlObject()
		{
			string strPath = this.treeView.SelectedNode.FullPath;

				try
				{
					string[] split = value.Split(treeView.PathSeparator.ToCharArray());

					TreeNodeCollection tnc = treeView.Nodes;

					TreeNode node = null;

					foreach(string s in split)
					{
						if(tnc == null)
							break;

						node = FindTreeChild(tnc, s);
						if(node == null)
							break;

						tnc = node.Nodes;
					}

			foreach(XmlNode xChild in xContainer.ChildNodes)
			{
				if(xChild.Name == "container")
				{
					string strContainerType = XmlHelper.GetKeyValue(xChild, "type");
					if(strContainerType == "tree_node")
					{
						string strName = XmlHelper.GetKeyValue(xChild, "name");
						TreeNode newNode = new TreeNode(strName);
						newNode.Tag = xChild;
						tnc.Add(newNode);
						BuildTree(xChild, newNode.Nodes);
					}
				}
			}
			return true;
		}
		cock */

		internal override string ObjectName
		{
//			get { return this.groupBox.Text; }
			set { this.groupBox.Text = value; }
		}


		internal override string ObjectValue
		{
			get{return this.treeView.SelectedNode.FullPath;}
			set
			{
				try
				{
					string[] split = value.Split(treeView.PathSeparator.ToCharArray());

					TreeNodeCollection tnc = treeView.Nodes;

					TreeNode node = null;

					foreach(string s in split)
					{
						if(tnc == null)
							break;

						node = FindTreeChild(tnc, s);
						if(node == null)
							break;

						tnc = node.Nodes;
					}

					if(node == null)
						if(treeView.Nodes.Count != 0)
							node = treeView.Nodes[0];
					if(this.treeView.SelectedNode != node)
						this.treeView.SelectedNode = node;
				}
				catch(Exception ex)
				{
					Debug.WriteLine("Exception in Container_Tree._ObjectValue. System reports : " + ex.Message);
				}
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.treeView;
			}
		}

		TreeNode FindTreeChild(TreeNodeCollection tnc, string strKey)
		{
			if(tnc != null)
			{
				foreach(TreeNode n in tnc)
				{
					if(n.Text == strKey)
						return n;
				}
			}
			return null;
		}

		internal override Control ChildContainer {get{return panelTreeElements;}}

		internal override List<XmlElement>xChildren
		{
			get
			{
				if(this.treeView.SelectedNode != null)
				{
					XmlNode xChildren = this.treeView.SelectedNode.Tag as XmlNode;
//					XmlNodeList xAllChildren = m_xObject.ChildNodes;

					XmlNodeList xAllChildren = xChildren.ChildNodes;

//					XmlNodeList xAllChildren = GetChildrenFrom
					List<XmlElement>xRelevantChildren =new List<XmlElement>();

					foreach(XmlNode xChild in xAllChildren)
					{
						if(xChild is XmlElement)
						{
							string strContainerType = XmlHelper.GetKeyValue(xChild, "type");
							if(strContainerType != "tree_node")
								xRelevantChildren.Add(xChild as XmlElement);
						}
						else
						{
							Debug.Assert(false, "Attend children.");
						}
					}

					return xRelevantChildren;
				}
				else
					return null;
			}
		}

		private void treeView_AfterSelect(object sender, TreeViewEventArgs e)
		{
//			ValueChanged("value", this.treeView.SelectedNode.FullPath, true);
			ValueChanged("value", e.Node.FullPath, true);
		}

//		static int guid = 0;



























		private void treeView_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
		{
/*
			if(e.Button == MouseButtons.Right)
			{
				string strPath = e.Node.FullPath;
				
				XmlNode xNode = e.Node.Tag as XmlNode;

				string strKeyPath = XmlHelper.FullNodePathByKey(xNode, xNode.OwnerDocument, "name", null);

				XmlNode xNodeSource = XmlHelper.GetNodeFromKeyPath(m_owner.m_menuDefinitionRoot["menu_system"], "name", strKeyPath);

				XmlNode xInherit = m_owner.m_menuDefinitionRoot.CreateElement("inherit");
				XmlHelper.AddAttribute(xInherit, "path", "new_tree_item");
				XmlHelper.AddAttribute(xInherit, "name", guid.ToString());
				
				xNodeSource.AppendChild(xInherit);

				XmlHelper.AddAttribute(m_xObject, "value", strPath + "\\" + guid.ToString());
				guid++;

				this.m_owner.SetMenu(xNode.OwnerDocument);
			}
*/		}
	}
}
		

