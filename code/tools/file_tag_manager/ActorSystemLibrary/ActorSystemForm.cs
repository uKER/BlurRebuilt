using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

using System.Data;
using System.Xml;
using System.Diagnostics;
using System.IO;
//using filemask;
using RegexpHelpers;

namespace ActorSystem
{
	/// <summary>
	/// Summary description for ActorSystemForm.
	/// </summary>
//	public class ActorSystemForm : System.Windows.Forms.Form
	public class ActorSystemForm : System.Windows.Forms.UserControl
	{
		private System.Windows.Forms.Panel controlPanel;
		private System.ComponentModel.Container components;

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
			this.controlPanel.AutoScroll = true;
			this.controlPanel.BackColor = System.Drawing.SystemColors.Control;
			this.controlPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.controlPanel.Location = new System.Drawing.Point(0, 0);
			this.controlPanel.Name = "controlPanel";
			this.controlPanel.Size = new System.Drawing.Size(488, 488);
			this.controlPanel.TabIndex = 0;
			// 
			// ActorSystemForm
			// 
			this.Controls.Add(this.controlPanel);
			this.Name = "ActorSystemForm";
			this.Size = new System.Drawing.Size(488, 488);
			this.ResumeLayout(false);

		}
		#endregion

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


		public delegate bool DelegateActorNodeChanged(XmlNode newActorNode);
		public DelegateActorNodeChanged ActorNodeChanged;

		public delegate bool DelegateActorStringChanged(string newActorString);
		public DelegateActorStringChanged ActorStringChanged;

		DebugWindow Output;
		XmlDocument m_actorDefinitionDocument;
		XmlDocument m_actorStringDocument;
		XmlDocument m_currentActorStringDocument;

		Hashtable m_enumTable = new Hashtable();

		int TOP_BORDER = 15;
		int BOTTOM_BORDER = 10;
		int LEFT_BORDER = 50;
		int RIGHT_BORDER = 10;

		ActorParameterGroup m_rootActorParameterGroup;
		bool m_destroyEverything = false;

		bool m_timerOn = false;
		bool m_animate = true;

		int gravSpeed = 11;

		public ActorSystemForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}


		public bool Initialise(string pathFilename)
		{
			//
			// TODO: Add any constructor code after InitializeComponent call
			//
			//Output = new DebugWindow();
			if(Output != null)
				Output.Show();
			InitTimer();

			m_actorDefinitionDocument = new XmlDocument();

			try
			{
				m_actorDefinitionDocument.Load(pathFilename);
			}
			catch (Exception ex)
			{
				MessageBox.Show(this, "Cannot load the XML actor document\n'"+pathFilename+"'\nSystem error:"+ex.ToString(), "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
				throw(new Exception("Cannot load the XML file : '"+pathFilename+"'"));
			}

			PopulateEnumList();

			SetGlobalFilemask();

			return true;
		}

		string m_globalFilemask;

		public string Filemask{get{return m_globalFilemask;}}

		bool SetGlobalFilemask()
		{
			XmlNode xFilemask = m_actorDefinitionDocument.SelectSingleNode("actor_system/options/filemask");

			if(xFilemask != null)
			{
				m_globalFilemask = XmlHelper.GetKeyValue(xFilemask, "value");

				if(m_globalFilemask != null)
				{
					m_globalFilemask = FilemaskHandler.ToRegexFromList(m_globalFilemask);
					return true;
				}
			}
			return false;
		}



		bool PopulateEnumList()
		{
			XmlNodeList xEnums = m_actorDefinitionDocument.SelectNodes("actor_system/enums/enum");

			if(xEnums != null)
			{
				foreach (XmlNode n in xEnums)
				{
					string strEnumName = XmlHelper.GetKeyValue(n, "name");
					Debug.WriteLine("Enum name: "+strEnumName);
					ArrayList strEnumValues = new ArrayList();
					string strEntries = n.InnerText;
					string strCurrentValue = "";
					bool instring = false;

					for(int c = 0; c<strEntries.Length; c++)
					{
						if(strEntries[c] == '\"')
						{
							if(instring == true)
							{
								strEnumValues.Add(strCurrentValue);
								strCurrentValue = "";
								instring = false;
							}
							else
							{
								instring = true;
							}
						}
						else
						{
							if(instring == true)
							{
								strCurrentValue += strEntries[c];
							}
							else
							{
							}
						}
					}
					m_enumTable.Add(strEnumName, strEnumValues);
				}
			}
			return true;
		}
					

		public bool SetString(string strActorString)
		{
			if(m_actorDefinitionDocument == null)
				return false;

			XmlNode root = m_actorDefinitionDocument;
			XmlNode clone = root.CloneNode(true);

			m_actorStringDocument = clone as XmlDocument;

			// Delete any controls in the form.
			if(m_rootActorParameterGroup != null)
			{
				this.controlPanel.Controls.Remove(m_rootActorParameterGroup);
				m_rootActorParameterGroup.Dispose();
			}

			// TODO: Parse the input.
			XmlDocument docCurrentActor = new XmlDocument();

			if(strActorString != null && strActorString.Length > 0)
			{
				System.IO.StringReader sr = new System.IO.StringReader(strActorString);
				try
				{
					docCurrentActor.Load(sr);
				}
				catch(Exception ex)
				{
					// TODO: Draw text to the panel describing a bad string.
					MessageBox.Show(this, "\n\nTEMPORARY CODE ***\n\nBAD STRING passed to SetString\nSystem report:\n"+ex.ToString(), "BROKEN", MessageBoxButtons.OK, MessageBoxIcon.Error);
					//					return false;
				}
			}

			m_currentActorStringDocument = docCurrentActor;

			BuildFullActor(m_actorStringDocument, m_currentActorStringDocument);

			m_rootActorParameterGroup = BuildMenu(m_actorStringDocument.SelectSingleNode("actor_system/actor"));
			this.controlPanel.Controls.Add(m_rootActorParameterGroup);
			this.ResumeLayout(true);

//			Debug.WriteLine(m_actorStringDocument.InnerXml);
			return true;
		}


		bool BuildFullActor(XmlDocument actorDefinitionDocument, XmlDocument currentActorDocument)
		{
			XmlNode actorDefinitionRootActor = actorDefinitionDocument.SelectSingleNode("/actor_system/actor");

			XmlNode currentActorRoot = currentActorDocument.SelectSingleNode("/actor");
			_BuildFullActor(actorDefinitionRootActor, currentActorRoot);

			return true;
		}


		bool _BuildFullActor(XmlNode sourceActor, XmlNode currentActor)
		{
			XmlNodeList sourceActorChildren		= sourceActor.ChildNodes;

			foreach(XmlNode sourceActorChild in sourceActorChildren)			// TODO: Do I need to test for sourceActorChildren being null?
			{
				switch(sourceActorChild.Name)
				{
					case "param":
					{
						XmlNode sourceParam = sourceActorChild;

						string strParamValue = null;

						string strSourceParamName = XmlHelper.GetKeyValue(sourceParam, "name");

						if(strSourceParamName != null)
						{
							if(currentActor != null)
							{
								XmlNodeList currentChildParams = currentActor.SelectNodes("param");
								XmlNode currentParam = XmlHelper.FindNodeByKeyValue(currentChildParams, "name", strSourceParamName);
								if(currentParam != null)
								{
									strParamValue = XmlHelper.GetKeyValue(currentParam, "value");
								}
							}
						}

						if(strParamValue == null)
							strParamValue = XmlHelper.GetKeyValue(sourceParam, "default");
						if(strParamValue == null)
							strParamValue = "MISSING DEFAULT";
						XmlHelper.AddAttribute(sourceParam, "value", strParamValue);
						break;
					}
					case "classes":
					{
						// Get the current 'class' for this 'classes' and put it into 'classes'.
						// Try currentClasses.current_class, then sourceClasses.current_class then sourceClasses.class[0].name then a default rogue string.
						// Then recurse each class in sourceClasses.

						XmlNode sourceClasses		= sourceActorChild;

						XmlNodeList sourceClassList	= sourceClasses.SelectNodes("class");
						string strSourceClassesName	= XmlHelper.GetKeyValue(sourceClasses, "name");
						XmlNode currentClasses		= XmlHelper.FindChildNode(currentActor, "classes", "name", strSourceClassesName);
						////////////////////////////
						string strCurrentClassName	= XmlHelper.GetKeyValue(currentClasses, "current_class");

						if(strCurrentClassName == null)
							strCurrentClassName = XmlHelper.GetKeyValue(sourceClasses, "current_class");

						if(strCurrentClassName == null && currentClasses != null)
							strCurrentClassName = XmlHelper.GetKeyValue(currentClasses.SelectSingleNode("class"), "name");

						if(strCurrentClassName == null)
							strCurrentClassName = XmlHelper.GetKeyValue(sourceClasses.SelectSingleNode("class"), "name");

						if(strCurrentClassName == null)
							strCurrentClassName = "*** NO CURRENT CLASS ***";

						XmlHelper.AddAttribute(sourceClasses, "current_class", strCurrentClassName);

						if(sourceClassList != null)
						{
							foreach(XmlNode sourceClass in sourceClassList)
							{
								string strSourceClassName = XmlHelper.GetKeyValue(sourceClass, "name");
								XmlNode currentClass = XmlHelper.FindChildNode(currentClasses, "class", "name", strSourceClassName);

								XmlNodeList sourceActorList = sourceClass.SelectNodes("actor");

								// Get the current actor for this class and put into sourceClass.
								// Try currentClass.current_actor, sourceClass.current_actor, sourceClass.actor[0].name then a default rogue string.
								string strCurrentActorName = XmlHelper.GetKeyValue(currentClass, "current_actor");

								if(strCurrentActorName == null)
									strCurrentActorName = XmlHelper.GetKeyValue(sourceClass, "current_actor");

								if(strCurrentActorName == null && currentClass != null)
									strCurrentActorName = XmlHelper.GetKeyValue(currentClass.SelectSingleNode("actor"), "name");

								if(strCurrentActorName == null)
									strCurrentActorName = XmlHelper.GetKeyValue(sourceClass.SelectSingleNode("actor"), "name");

								if(strCurrentActorName == null)
									strCurrentActorName = "*** NO CURRENT ACTOR ***";

								XmlHelper.AddAttribute(sourceClass, "current_actor", strCurrentActorName);

								if(sourceActorList != null)
								{
									foreach(XmlNode sourceChildActor in sourceActorList)
									{
										string strActorName = XmlHelper.GetKeyValue(sourceChildActor, "name");
										
										XmlNode currentChildActor = XmlHelper.FindChildNode(currentClass, "actor", "name", strActorName);

										_BuildFullActor(sourceChildActor, currentChildActor);
									}
								}
							}
						}
						///////////////////

						break;
					}
				}
			}

			return true;
		}


		Control CreateControlFromParam(XmlNode param, string strValue)
		{
			string strParamType = XmlHelper.GetKeyValue(param, "type");
			string paramName = XmlHelper.GetKeyValue(param, "name");
			
			ActorParameter newControl = null;

			switch(strParamType)
			{
				case "int":
				{
					newControl = new ActorInt(this, param);
					newControl.ParameterChanged += new EventHandler(ParameterChanged);
					break;
				}

				case "bool":
				{
					newControl = new ActorBool(this, param);
					newControl.ParameterChanged += new EventHandler(ParameterChanged);
					break;
				}


				case "enum":
				{
					string strValues = XmlHelper.GetKeyValue(param, "values");
					if(strValues != null)
					{
						ArrayList strEntries = m_enumTable[strValues] as ArrayList;

						if(strEntries != null)
						{
							newControl = new ActorDropbox(this, param, strEntries);

							newControl.ParameterChanged += new EventHandler(ParameterChanged);
						}
					}

					break;
				}

				default:
				{
					newControl = new ActorInt(this, param);
					newControl.ParameterChanged += new EventHandler(ParameterChanged);
					break;
				}
			}

			if(newControl != null)
			{
				newControl.ActorName = paramName;
				newControl.ActorValue = strValue;
			}

			return newControl;
		}


		ActorDualCombo MakeComboBoxes(XmlNode xmlActor)
		{
			ArrayList strClassNames = new ArrayList();
			ArrayList strActorNames = new ArrayList();

			XmlNode xmlClass = xmlActor.ParentNode;					// The class to which this actor belongs.
			XmlNode xmlClasses = xmlClass.ParentNode;				// The classes to which this actor belongs.

			foreach(XmlNode n in xmlClasses)
				strClassNames.Add(XmlHelper.GetKeyValue(n, "name"));
			foreach(XmlNode n in xmlClass)
				strActorNames.Add(XmlHelper.GetKeyValue(n, "name"));

			ActorDualCombo actorDualCombo = new ActorDualCombo(this, strClassNames, strActorNames, xmlClasses, xmlClass);
			actorDualCombo.classDropbox.ParameterChanged += new EventHandler(classDualCombo_ActorChanged);
			actorDualCombo.actorDropbox.ParameterChanged += new EventHandler(actorDualCombo_ActorChanged);

			actorDualCombo.classDropbox.ActorName = "Class";
			actorDualCombo.classDropbox.ActorValue = XmlHelper.GetKeyValue(xmlClass, "name");

			actorDualCombo.actorDropbox.ActorName = "Actor";
			actorDualCombo.actorDropbox.ActorValue = XmlHelper.GetKeyValue(xmlActor, "name");

			actorDualCombo.sourceXmlNode = xmlClasses;
			return actorDualCombo;
		}


		int GetIdeal_Y(UserControl.ControlCollection controlCollection)
		{
			int bottomY = TOP_BORDER;

			if(controlCollection != null)
			{
				foreach(Control control in controlCollection)
				{
					bottomY += control.Height;
				}
			}

			return bottomY;
		}
			

		ActorParameterGroup BuildMenu(XmlNode xActor)
		{
			ActorParameterGroup apg = new ActorParameterGroup();

			XmlNodeList xActorChildren		= xActor.ChildNodes;	// 'Actor' and 'classes' definitions.

			int width = 0, height = 0;

			foreach(XmlNode xActorChild in xActorChildren)			// TODO: Do I need to test for sourceActorChildren being null?
			{
				switch(xActorChild.Name)
				{
					case "param":
					{
						XmlNode xParam = xActorChild;
						// Blah make the parameter.
						string strValue = XmlHelper.GetKeyValue(xParam, "value");
						if(strValue == null)
							strValue = "*** BAD VALUE ***";
						Control newControl = CreateControlFromParam(xParam, strValue);
						// Set X and Y
						newControl.Location = new Point(LEFT_BORDER, GetIdeal_Y(apg.ChildControls));
						apg.ChildControls.Add(newControl);

						if(newControl.Width > width) width = newControl.Width;
						height += newControl.Height;
						break;
					}
					case "classes":
					{
						XmlNode xClasses = xActorChild;
						ActorParameter actorDualCombo = ConstructClassActorCombo(xClasses);
						actorDualCombo.Location = new Point(LEFT_BORDER, GetIdeal_Y(apg.ChildControls));

						apg.ChildControls.Add(actorDualCombo);

						if(actorDualCombo.Width > width) width = actorDualCombo.Width;
						height += actorDualCombo.Height;
					}
					break;
				}
			}
			apg.Width = (width + LEFT_BORDER + RIGHT_BORDER);
			apg.Height = (height + TOP_BORDER + BOTTOM_BORDER);
			apg.m_idealHeight = (height + TOP_BORDER + BOTTOM_BORDER);

			return apg;
		}


		XmlNode GetClassActorComboActor(XmlNode xClasses)
		{
			string strCurrentClassName = XmlHelper.GetKeyValue(xClasses, "current_class");
			if(strCurrentClassName != null)
			{
				XmlNode xClass = XmlHelper.FindChildNode(xClasses, "class", "name", strCurrentClassName);
				if(xClass != null)
				{
					string strCurrentActorName = XmlHelper.GetKeyValue(xClass, "current_actor");
					if(strCurrentActorName != null)
					{
						XmlNode xChildActor = XmlHelper.FindChildNode(xClass, "actor", "name", strCurrentActorName);

						return xChildActor;
					}
				}
			}
			return null;
		}


		ActorParameter ConstructClassActorCombo(XmlNode xClasses)
		{
			XmlNode xChildActor = GetClassActorComboActor(xClasses);
			ActorParameter actorParameter = null;
			if(xChildActor != null)
			{
				ActorDualCombo actorDualCombo = MakeComboBoxes(xChildActor);

				ActorParameterGroup actorDualComboChildren = BuildMenu(xChildActor);

				if(actorDualCombo.Width < actorDualComboChildren.Width + LEFT_BORDER + RIGHT_BORDER)
					actorDualCombo.Width = actorDualComboChildren.Width + LEFT_BORDER + RIGHT_BORDER;

				actorDualCombo.Height += actorDualComboChildren.Height;

				actorDualComboChildren.Location = new Point(actorDualCombo.classDropbox.Location.X, actorDualCombo.classDropbox.Location.Y + actorDualCombo.classDropbox.Height);
				actorDualCombo.groupBox.Controls.Add(actorDualComboChildren);
				actorDualCombo.actorParameterGroup = actorDualComboChildren;
				actorParameter = actorDualCombo;
			}
			return actorParameter;
		}

		///////////////////////////////////////////////////////////////////////////////////////
		///

		public XmlNode GetActorNode()
		{
			XmlNode actorNode = m_actorStringDocument.SelectSingleNode("actor_system/actor");
			return actorNode;
		}


		public void ShowActorString()
		{
			if(Output != null)
				Output.Clear();
			
			string strTemp = m_actorStringDocument.OuterXml;

			DoEvent_ActorStringChanged();
			DoEvent_ActorNodeChanged();

			strTemp = strTemp.Replace("><", ">\n<");
			if(Output != null)
				Output.Print(strTemp);
		}

		bool DoEvent_ActorStringChanged()
		{
			if(ActorStringChanged != null)
			{
				XmlNode actorNode = m_actorStringDocument.SelectSingleNode("actor_system/actor");
				if(actorNode != null)
					return ActorStringChanged(actorNode.OuterXml);
				else
					return ActorStringChanged("");
			}
			return false;
		}

		bool DoEvent_ActorNodeChanged()
		{
			if(ActorNodeChanged != null)
			{
				XmlNode actorNode = m_actorStringDocument.SelectSingleNode("actor_system/actor");
				if(actorNode != null)
					return ActorNodeChanged(actorNode.Clone());
				else
					return ActorNodeChanged(null);
			}
			return false;
		}



		private void ParameterChanged(object sender, EventArgs e)
		{
			ActorParameter ap = sender as ActorParameter;

			XmlNode n = ap.ValueNode;

			XmlHelper.AddAttribute(n, "value", ap._ActorValue);

			ShowActorString();
		}


		private void classDualCombo_ActorChanged(object sender, EventArgs e)
		{
			ActorParameter ap = sender as ActorParameter;
			XmlNode n = ap.ValueNode;
			XmlHelper.AddAttribute(n, "current_class", ap._ActorValue);				// Put the new value for this dropdown into the xml data.

			ActorDualCombo adc = ap.Parent.Parent as ActorDualCombo;
			adc.state = ActorDualCombo.State.Killing;								// Flag this combo's child parameter group for death.
			XmlNode xmlActor = GetClassActorComboActor(adc.sourceXmlNode);			// Find the replacement actor.

			adc.actorDropbox.comboBox.Items.Clear();								// Update the ACTOR dropdown.
			XmlNode xmlClass = xmlActor.ParentNode;

			foreach(XmlNode actorNode in xmlClass)
				adc.actorDropbox.comboBox.Items.Add(XmlHelper.GetKeyValue(actorNode, "name"));

			adc.actorDropbox.ValueNode = xmlActor.ParentNode;						// ACTOR dropdown has a new 'value' target.
			adc.actorDropbox.ActorValue = XmlHelper.GetKeyValue(xmlActor, "name");

			ActorParameterGroup actorDualComboChildren = BuildMenu(xmlActor);		// Make the replacement child group.
			adc.nextActorParameterGroup = actorDualComboChildren;

			adc.actorParameterGroup.m_idealHeight = 0;								// Collapse the child group.

			ShowActorString();														// Debug.
		}


		private void actorDualCombo_ActorChanged(object sender, EventArgs e)
		{
			ActorParameter ap = sender as ActorParameter;
			XmlNode n = ap.ValueNode;
			XmlHelper.AddAttribute(n, "current_actor", ap._ActorValue);

			ActorDualCombo adc = ap.Parent.Parent as ActorDualCombo;
			adc.state = ActorDualCombo.State.Killing;
			XmlNode xmlActor = GetClassActorComboActor(adc.sourceXmlNode);
			ActorParameterGroup actorDualComboChildren = BuildMenu(xmlActor);
			adc.nextActorParameterGroup = actorDualComboChildren;

			adc.actorParameterGroup.m_idealHeight = 0;

			ShowActorString();
		}


		void DestroyEverything()
		{
			this.controlPanel.Controls.Remove(m_rootActorParameterGroup);
			m_rootActorParameterGroup.Dispose();
			m_rootActorParameterGroup = BuildMenu(m_actorStringDocument.SelectSingleNode("actor_system/actor"));
			this.controlPanel.Controls.Add(m_rootActorParameterGroup);
		}


		#region Timer stuff.
		int timermax = 5;
		private Timer timer;
		void InitTimer()
		{

			timer = new Timer();
			timer.Interval = 10;
			timer.Start();
			timer.Tick += new EventHandler(timer_Tick);
		}
		
		int testtimer = 0;

		private void timer_Tick(object sender, EventArgs e)
		{
			if(m_destroyEverything == true)
			{
				DestroyEverything();
				m_destroyEverything = false;
			}
			if(m_rootActorParameterGroup != null)
			{
				this.SuspendLayout();
				UpdateMenu(m_rootActorParameterGroup);
				this.ResumeLayout();
//				m_destroyEverything = true;
			}
			if(m_timerOn == true)
			{
				testtimer++;
				if(testtimer > timermax)
				{
					testtimer = 0;
				}
			}

		}

		private void timerToggle_Click(object sender, System.EventArgs e)
		{
			MenuItem mi = sender as MenuItem;

			m_timerOn = !m_timerOn;
			mi.Checked = m_timerOn;
		}

		private void menuItem2_Click(object sender, System.EventArgs e)
		{
			MenuItem mi = sender as MenuItem;

			m_animate = !m_animate;
			mi.Checked = m_animate;
		}
		#endregion Timer stuff.

		int GravitateTo(int from, int to)
		{
			if(m_animate == false)
				return to;


			int retval = from;
			if(retval < to)
			{
				retval += gravSpeed;
				if(retval > to)
					retval = to;
			}
			else if (retval > to)
			{
				retval-= gravSpeed;
				if(retval < to)
					retval = to;
			}
			return retval;
		}

		bool UpdateMenu(ActorParameterGroup apg)
		{
			if(apg.ChildControls != null && apg.m_idealHeight != 0)
			{
				int num = apg.ChildControls.Count;
				apg.m_idealHeight = GetIdeal_Y(apg.ChildControls) + TOP_BORDER ;//+ BOTTOM_BORDER;
			}

			apg.Height = GravitateTo(apg.Height, apg.m_idealHeight);

			if(apg.ChildControls != null)
			{
				int numControls = apg.ChildControls.Count;

				int y_offset = TOP_BORDER;

				for(int c = 0; c < numControls; c++)
				{
					Control childControl = apg.ChildControls[c];

					ActorParameter ap = childControl as ActorParameter;

					if(ap != null)
					{
						ap.Location = new Point(ap.Location.X, y_offset);
						y_offset += ap.Height;
					}

					ActorDualCombo adc = childControl as ActorDualCombo;

					if(adc != null)
					{
						ActorParameterGroup childApg = adc.actorParameterGroup as ActorParameterGroup;
						UpdateMenu(childApg);
						adc.Height = childApg.Height + adc.actorDropbox.Height + TOP_BORDER + BOTTOM_BORDER;

						if(adc.state == ActorDualCombo.State.Killing && childApg.Height == 0)
						{
							adc.state = ActorDualCombo.State.Normal;
							adc.groupBox.Controls.Remove(childApg);
							childApg.Dispose();

							adc.nextActorParameterGroup.Location = new Point(adc.classDropbox.Location.X, adc.classDropbox.Location.Y + adc.classDropbox.Height);
							adc.nextActorParameterGroup.Height = 0;
							adc.groupBox.Controls.Add(adc.nextActorParameterGroup);
							adc.actorParameterGroup = adc.nextActorParameterGroup;

						}
					}
				}
			}
			return true;
		}
	}
}



