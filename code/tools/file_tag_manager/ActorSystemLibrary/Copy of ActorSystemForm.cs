using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

using System.Data;
using System.Xml;
using System.Diagnostics;

namespace ActorSystem
{
	/// <summary>
	/// Summary description for ActorSystemForm.
	/// </summary>
	public class ActorSystemForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Panel controlPanel;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		private ControlManager m_controlManager = null;

		private DebugWindow Output = null;

		public ActorSystemForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
			m_controlManager = new ControlManager(controlPanel);
			Output = new DebugWindow();
			Output.Show();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		XmlDocument m_actorDocument = null;

		public bool Initialise(string pathFilename)
		{
			m_actorDocument = new XmlDocument();

			try
			{
				m_actorDocument.Load(pathFilename);
			}
			catch (Exception ex)
			{
				MessageBox.Show(this, "Cannot load the XML actor document\n'"+pathFilename+"'\nSysyem error:"+ex.ToString(), "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return false;
			}
			return true;
		}

		private string FullXmlNodePath(XmlNode node, string delim)
		{
			if (delim == null)
				delim = "/";
 
			if (node.ParentNode != null && !(node.ParentNode is XmlDocument))
				return FullXmlNodePath(node.ParentNode, delim) + delim + node.Name;
			else
				return node.Name;
		}


		public bool SetString(string strActorString)
		{
			if(m_actorDocument == null)
				return false;

			this.SuspendLayout();

			// TODO: Delete any controls in the form.
			m_controlManager.ClearControls();

			// TODO: Parse the input.
			XmlDocument doc = new XmlDocument();

			System.IO.StringReader sr = new System.IO.StringReader(strActorString);
			try
			{
				doc.Load(sr);
			}
			catch(Exception ex)
			{
				// TODO: Draw text to the panel describing a bad string.
				MessageBox.Show(this, "\n\nTEMPORARY CODE ***\n\nBAD STRING passed to SetString\nSystem report:\n"+ex.ToString(), "BROKEN", MessageBoxButtons.OK, MessageBoxIcon.Error);
				this.ResumeLayout();
				return false;
			}

			XmlNode xClass = doc.SelectSingleNode("/class");
			XmlNodeList yClasses = m_actorDocument.SelectNodes("/actor_system/classes/class");

			XmlDocument docFullActor = BuildFullActor(m_actorDocument);

			bool result = BuildMenu(xClass, yClasses);
			this.ResumeLayout(true);
			return result;
		}

		string GetKeyValue(XmlNode node, string strKey)
		{
			string strRetval = null;
			if(node != null)
			{
				XmlNode attr = node.Attributes.GetNamedItem(strKey);
				if(attr != null)
				{
					strRetval = attr.Value;
				}
			}
			return strRetval;
		}


		XmlDocument BuildFullActor(XmlDocument xmlActorClass)
		{
			XmlDocument xmlFullActor = new XmlDocument();

			XmlNodeList documentRootClasses = xmlActorClass.SelectNodes("/actor_system/classes/class");

			foreach(XmlNode sourceClass in documentRootClasses)
			{
				_BuildFullActor(sourceClass, xmlFullActor);
			}

			return xmlFullActor;
		}

		int m_nestLevel = 0;
		bool NestPrint(string s)
		{
			for(int c = 0; c < m_nestLevel; c++)
				Output.Print("    ");
			Output.Print(s);
			return true;
		}

		bool _BuildFullActor(XmlNode sourceClass, XmlNode destNode)
		{
			NestPrint("Class : " + sourceClass.Attributes.GetNamedItem("name").Value + "\n");
			m_nestLevel++;

			XmlNodeList sourceActors = sourceClass.SelectNodes("actor");

			foreach(XmlNode sourceActor in sourceActors)
			{
				NestPrint("Actor : " + sourceActor.Attributes.GetNamedItem("name").Value + "\n");
				m_nestLevel++;
				XmlNodeList sourceActorChildren = sourceActor.ChildNodes;

				foreach(XmlNode sourceActorChild in sourceActorChildren)
				{
					string sourceActorChildName = sourceActorChild.Name;

					switch(sourceActorChildName)
					{
						case "param":
						{
							NestPrint("Parameter name = " + sourceActorChild.Attributes.GetNamedItem("name").Value + "\n");
							break;
						}
						case "class":
						{
							_BuildFullActor(sourceActorChild, destNode);
							break;
						}
					}
				}
				m_nestLevel--;
			}
			m_nestLevel--;
			return false;
		}


		Control CreateControlFromParam(XmlNode param, string strValue)
		{
			string paramName = GetKeyValue(param, "name");
			
			ActorInt newControl = new ActorInt(this);				// TODO: Make new actor based on yParam attribute "type".
			newControl.ParameterChanged += new EventHandler(ParameterChanged);

			newControl.ActorName = paramName;
			newControl.ActorValue = strValue;
			m_controlManager.AddControl(newControl, null);		// TODO: Pass in an attribute.

			return newControl;
		}

		bool MakeComboBoxes(XmlNodeList xmlClasses, XmlNode xmlClass, XmlNode xmlActor)
		{
			ArrayList strClassNames = new ArrayList();
			ArrayList strActorNames = new ArrayList();

			foreach(XmlNode n in xmlClasses)
				strClassNames.Add(GetKeyValue(n, "name"));
			foreach(XmlNode n in xmlClass.SelectNodes("actor"))
				strActorNames.Add(GetKeyValue(n, "name"));

			ActorDualCombo actorDualCombo = new ActorDualCombo(this, strClassNames, strActorNames);

			actorDualCombo.ActorName1 = "Class";
			actorDualCombo.ActorValue1 = GetKeyValue(xmlClass, "name");

			actorDualCombo.ActorName2 = "Actor";
			actorDualCombo.ActorValue2 = GetKeyValue(xmlActor, "name");

			m_controlManager.AddControl(actorDualCombo, null);		// TODO: Pass in an attribute.

			return true;
		}


		bool BuildMenu(XmlNode xClass, XmlNodeList yClasses)
		{
			// Get the class name in both x and y.											./class
			string xClassName	= GetKeyValue(xClass, "name");
			XmlNode yClass		= FindNodeByKeyValue(yClasses, "name", xClassName);

			if(yClass == null)									// Look for a class that matches the input, otherwise
			{													// just pick the first class in the definition.
				yClass = yClasses[0];
			}

			if(yClass != null)
			{
				string yClassName	= GetKeyValue(yClass, "name");

				if(yClassName != null)
				{

					// Get the actor in both x and y.												./class/actor
					XmlNode xActor		= (xClass == null)	? null : xClass.SelectSingleNode("actor");
					string xActorName	= (xActor == null)	? null : xActor.Attributes.GetNamedItem("name").Value;

					XmlNode yActor = FindNodeByKeyValue(yClass.SelectNodes("actor"), "name", xActorName);

					if(yActor == null)							// Look for a actor that matches the input, otherwise
					{											// just pick the first actor in the definition.
						yActor = yClass.SelectSingleNode("actor");
					}

					string yActorName = GetKeyValue(yActor, "name");

					MakeComboBoxes(yClasses, yClass, yActor);

					if(yActorName != null)
					{
						// Get a list of 'param's in x.													./class/actor/param[]
						XmlNodeList xParams = (xActor == null) ? null : xActor.SelectNodes("param");

						// Get a list of entries in y.													./class/actor/*[]
						XmlNodeList yParams = (yActor == null) ? null : yActor.ChildNodes;

						if(yParams != null)
						{

							foreach(XmlNode yNode in yParams)
							{
								// TODO: Check if it's a "param" or a "class". "param"s should be recursed.
								if(yNode.Name == "param")
								{
									XmlNode yParam = yNode;
									string yParamName = GetKeyValue(yParam, "name");

									XmlNode xParam = FindNodeByKeyValue(xParams, "name", yParamName);

									string strValue;

									if(xParam != null && xParam.Attributes.GetNamedItem("value") != null)
										strValue = xParam.Attributes.GetNamedItem("value").Value;
									else if(yParam.Attributes.GetNamedItem("default") != null)
										strValue = yParam.Attributes.GetNamedItem("default").Value;
									else
										strValue = "";

									Control newControl = CreateControlFromParam(yParam, strValue);
								}
								else if(yNode.Name == "classes")
								{
									// TODO: Decide here if we want it.
									XmlNode xChildClass = xActor.SelectSingleNode("class");
									XmlNodeList yChildClasses = yNode.SelectNodes("class");
									return BuildMenu(xChildClass, yChildClasses);
								}
							}
						}
						else
							return false;
					}
					else
					{
						// TODO: Actor is missing a 'name' attribute.
					}
				}
				else
				{
					// TODO: Class is missing a 'name' attribute.
				}
			}
			else
			{
				// TODO: No classes.
			}
			return true;
		}

		XmlNode FindNodeByKeyValue(XmlNodeList xmlList, string strName, string strValue)
		{
			foreach(XmlNode n in xmlList)
			{
				if(n.Attributes.GetNamedItem(strName).Value == strValue)
					return n;
			}
			return null;
		}

		/// <summary>
		/// Given a node list and a key, make a list of strings holding the value of every
		/// node that holds the key.
		/// </summary>
		/// <param name="nodeList">List of XML nodes</param>
		/// <param name="strKey">The key on which to search</param>
		/// <returns></returns>
		ArrayList GetXmlValueList(XmlNodeList nodeList, string strKey)
		{
			ArrayList arrayResult = new ArrayList();

			foreach(XmlNode n in nodeList)
			{
				if(n.Attributes.GetNamedItem(strKey) != null)
				{
					arrayResult.Add(n.Attributes.GetNamedItem(strKey).Value);
				}
			}
			return arrayResult;
		}




		bool AddActorStringToTable(Hashtable table, XmlNodeList nodeList, string strActorString)
		{
			////////////////////////////////////////////////////////////////////////////////
			/// Build a hash table with any key-value pairs taken from the actor string.
			foreach (XmlNode node in nodeList)
			{
				string strValue = node.Attributes.GetNamedItem("value").Value;
				try
				{
					table.Add(node.Attributes.GetNamedItem("name").Value, node.Attributes.GetNamedItem("value").Value);
				}
				catch(Exception ex)
				{
					// TODO: Draw text to the panel describing a bad string.
					MessageBox.Show(this, "\n\nTEMPORARY CODE ***\n\nBAD STRING passed to AddActorStringToTable\nSystem report:\n"+ex.ToString(), "BROKEN", MessageBoxButtons.OK, MessageBoxIcon.Error);
					return false;
				}
			}
			return true;
		}




		public class ControlManager
		{
			private Control m_parentControlPanel;
			private ArrayList m_controlList = new ArrayList();

			public ControlManager(Control parentControlPanel)
			{
				m_parentControlPanel = parentControlPanel;
			}

			private class ControlProperties
			{
				public ControlProperties(Control control, XmlAttribute attrValue)
				{
					m_control = control;
					m_attrValue = attrValue;
				}

				Control m_control;
				XmlAttribute m_attrValue;				// The attribute that holds the value of this control.

				public Control Control{get{return m_control;}}
				public XmlAttribute AttrValue{get{return m_attrValue;}}

			}


			public bool AddControl(Control control, XmlAttribute attrValue)
			{
				System.Drawing.Point pt = new System.Drawing.Point(0, 0);
				ControlProperties cp = new ControlProperties(control, attrValue);

				Control lastControl = null;

				if(m_controlList.Count != 0)
				{
					lastControl = (m_controlList[m_controlList.Count - 1] as ControlProperties).Control;
					pt.X = lastControl.Location.X;
					pt.Y = lastControl.Location.Y + lastControl.Height;
				}

				control.Location = pt;
				m_controlList.Add(cp);
				m_parentControlPanel.Controls.Add(control);

				return true;
			}

			public bool ClearControls()
			{
				foreach(ControlProperties cp in m_controlList)
				{
					m_parentControlPanel.Controls.Remove(cp.Control);
				}

				m_controlList.Clear();

				return true;
			}
		}



		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.controlPanel = new System.Windows.Forms.Panel();
			this.SuspendLayout();
			// 
			// controlPanel
			// 
			this.controlPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.controlPanel.BackColor = System.Drawing.SystemColors.Control;
			this.controlPanel.Location = new System.Drawing.Point(8, 8);
			this.controlPanel.Name = "controlPanel";
			this.controlPanel.Size = new System.Drawing.Size(424, 600);
			this.controlPanel.TabIndex = 0;
			// 
			// ActorSystemForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.BackColor = System.Drawing.SystemColors.GrayText;
			this.ClientSize = new System.Drawing.Size(448, 622);
			this.Controls.Add(this.controlPanel);
			this.ForeColor = System.Drawing.SystemColors.ControlText;
			this.Name = "ActorSystemForm";
			this.Text = "ActorSystemForm";
			this.ResumeLayout(false);

		}
		#endregion

		private void ParameterChanged(object sender, EventArgs e)
		{
			Debug.WriteLine("Parameter changed.");
		}
	}
}


/*
			try
			{
				doc.Load(pathFileName);
			}
			catch (Exception ex)
			{
				this.ResumeLayout();
				throw new ApplicationException("Can't load the file '" + pathFileName + "'\n" + ex.Message);
			}

			XmlNode child = doc.SelectSingleNode("/root");

			XmlNode globalConfigNode = doc.SelectSingleNode("/root/global_config");
			m_allowLockFileCreation	= GetAttributeBool(globalConfigNode, "allow_lockfile_creation");

			XmlNodeList nodeList = doc.SelectNodes("/root/panel");
			AssetControlForm panel;

			foreach (XmlNode node in nodeList)
			{
*/

