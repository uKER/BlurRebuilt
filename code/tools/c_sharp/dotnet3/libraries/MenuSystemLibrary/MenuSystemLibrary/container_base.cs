
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Collections;
using Global;
using System.Diagnostics;
using System.Threading;



namespace MenuSystemLibrary
{
	public partial  class Container_Base : UserControl
	{
		readonly bool m_bCreateChildren;
		readonly protected MenuSystem m_owner;
		public XmlElement m_xObject;
		public bool m_bSaveValue;

		private Button m_conflictedButton = null;

		protected Button CreateConflictedButton(string strText, int Left, int Top, int Width, int Height)
		{
			if(m_conflictedButton == null)
			{
				m_conflictedButton = new Button();
				m_conflictedButton.Parent = this;
				m_conflictedButton.BringToFront();
				this.Enabled = this.m_bShadowEnabled;
			}
			m_conflictedButton.Text = strText;

			m_conflictedButton.Location = new Point(Left, Top);
			m_conflictedButton.Size = new Size(Width, Height);

			m_conflictedButton.Visible = true;

			m_conflictedButton.Click += new EventHandler(m_conflictedButton_Click);

			return m_conflictedButton;
		}

		void m_conflictedButton_Click(object sender, EventArgs e)
		{
			ValueChanged("value", this.ObjectValue, false);
			m_conflictedButton.Visible = false;
		}

		/// <summary>Reflects the "readonly" key in the Xml definition. Marks the container to be uneditable in the GUI.</summary>
		bool m_bXmlRequestReadOnly = false;
		/// <summary>Reflects whether the control is readonly in the GUI.</summary>

		bool m_bAlwaysEnabled = false;
		XmlElement m_xDependsOn = null;
		bool m_bDependencyTrueMeansEnable = true;
		internal string strType = null;
		static Font m_readonlyFont = null;
		internal List<Container_RadioButton> m_containerRadioButtons = null;
		internal Container_Base()
		{
			InitializeComponent();
		}
		internal bool m_bShadowEnabled = true;
		int childGapLeft;
		int childGapTop;
		int childGapRight;
		int childGapBottom;

		int m_width;
		int m_height;
		int m_minWidth;
		int m_minHeight;
		int m_maxWidth;
		int m_maxHeight;

		internal Container_Base(MenuSystem owner, XmlElement xObject, bool bCreateChildren) : this()
		{
			m_xObject = xObject;
			m_bCreateChildren = bCreateChildren;
			m_owner = owner;
		}

		private void Container_Base_Load(object sender, EventArgs e)
		{
			if(DesignMode == true)
				return;

			strType = XmlHelper.GetKeyValue(m_xObject, "type");

			m_width = XmlHelper.GetKeyInt(m_xObject, "width", 0);
			m_height = XmlHelper.GetKeyInt(m_xObject, "height", 0);
			m_minWidth = XmlHelper.GetKeyInt(m_xObject, "minWidth", 0);
			m_minHeight = XmlHelper.GetKeyInt(m_xObject, "minHeight", 0);
			m_maxWidth = XmlHelper.GetKeyInt(m_xObject, "maxWidth", int.MaxValue);
			m_maxHeight = XmlHelper.GetKeyInt(m_xObject, "maxHeight", int.MaxValue);

			int width, height;

			if(m_width != 0)
			{
				width = m_width;
			}
			else
				width = this.Width;

			if(m_height != 0)
			{
				height = m_height;
			}
			else
				height = this.Height;

			width = Clamp(width, m_minWidth, m_maxWidth);
			height = Clamp(height, m_minHeight, m_maxHeight);

			this.Width = width;
			this.Height = height;

			string strDependsOn = XmlHelper.GetKeyValue(m_xObject, "depends_on");
			if(strDependsOn != null)
			{
				if(strDependsOn.StartsWith("!"))
				{
					m_bDependencyTrueMeansEnable = false;
					strDependsOn = strDependsOn.Substring(1);
				}

				m_xDependsOn = XmlHelper.GetNodeFromKeyPath(m_xObject, "name", strDependsOn) as XmlElement;
				// TODO: Detect and report a broken dependency.
				XmlHelper.AddAttribute(m_xDependsOn, "has_dependent", "true");
			}

			bool bSaveFlag = XmlHelper.GetKeyBool(m_xObject, "save", true);

			if(bSaveFlag == false)
			{
				if(m_readonlyFont == null)
				{
					m_readonlyFont = new Font(this.Font.FontFamily, this.Font.Size, FontStyle.Italic);
				}
				this.Font = m_readonlyFont;
			}

			this.SetStyle(ControlStyles.Selectable, false);		// Disallow receiving focus.

			m_bSaveValue = XmlHelper.GetKeyBool(m_xObject, "save", true);

			m_bAlwaysEnabled = XmlHelper.GetKeyBool(m_xObject, "always_enabled", false);

			m_bXmlRequestReadOnly = XmlHelper.GetKeyBool(m_xObject, "readonly", false);
		}

		int Clamp(int _value, int min, int max)
		{
			if(_value < min)
			{
				return min;
			}
			else if(_value > max)
			{
				return max;
			}
			return _value;
		}

		internal bool AddContainerRadioButton(Container_RadioButton crb)
		{
			if(m_containerRadioButtons == null)
			{
				m_containerRadioButtons = new List<Container_RadioButton>();
			}
			m_containerRadioButtons.Add(crb);
			return true;
		}

		/// <summary>
		/// This is called after the derived class has been constructed.
		/// </summary>
		/// <returns>true</returns>
		internal virtual bool PostConstructor()
		{
			// Set the name of this control.
			this.Name = XmlHelper.GetKeyValue(m_xObject, "name");


			string strTooltip = XmlHelper.GetKeyValue(m_xObject, "tooltip");
			//strTooltip = this.Name;
			if(strTooltip != null)
			{
				this.toolTip.SetToolTip(this.ObjectTooltipTarget, strTooltip);
				this.toolTip.ShowAlways = true;
			}


			string strName = XmlHelper.GetKeyValue(m_xObject, "display_name");
			if(strName != null)
			{
				this.ObjectName = strName;
			}
			else
			{
				strName = XmlHelper.GetKeyValue(m_xObject, "name");
				ObjectName = strName;
			}

			// Set the value of this control.
			bool bSaveFlag = XmlHelper.GetKeyBool(m_xObject, "save", true);

			if(bSaveFlag == true)
			{
				string strValue = XmlHelper.GetKeyValue(m_xObject, "value");
				if(strValue == null)
					strValue = MenuSystemLibrary.MenuSystem.UNINITIALISED_MARKER;
				this.ObjectValue = strValue;
			}
			else
			{
				string strDefault = XmlHelper.GetKeyValue(m_xObject, "default");
				if(strDefault == null)
					strDefault = MenuSystemLibrary.MenuSystem.UNINITIALISED_MARKER;
				this.ObjectValue = strDefault;
			}

			// Set the default status text of this control.
			string strStatus = XmlHelper.GetKeyValue(m_xObject, "type");
			strStatusText = strStatus;

			// Create the children of this control.
			if(m_bCreateChildren == true)
			{
				this.CreateMenuChildren(m_bCreateChildren);
			}

			// Store these values now since they can change if min or maxsize is set on this container.
			if(ChildContainer != null)
			{
				childGapLeft	= this.ChildContainer.Left;
				childGapTop		= this.ChildContainer.Top;
				childGapRight	= this.Width - this.ChildContainer.Right;
				childGapBottom	= this.Height - this.ChildContainer.Bottom;
			}

			return true;
		}

		/// <summary>
		/// This is called when a value in a container changes.
		/// If it is called by the OnChange event handler for the value because of user input, then m_ignoreChangedEventHandler is false
		/// and further processing is done.
		/// </summary>
		/// <param name="strValueName"></param>
		/// <param name="strValue"></param>
		/// <param name="bRefreshChildren"></param>
		/// <returns></returns>
		protected bool ValueChanged(string strValueName, string strValue, bool bRefreshChildren)
		{
			XmlHelper.AddAttribute(m_xObject, strValueName, strValue);

			if(XmlHelper.GetKeyBool(m_xObject, "has_dependent", false) == true)
			{
				m_owner.ProcessDependencies();
			}

			m_owner.m_bChanged = true;

			if(m_owner.m_ignoreChangedEventHandler == 0)
			{
//				Debug.WriteLine("Value changed.");
				if(bRefreshChildren == true)
				{
					this.CreateMenuChildren(true);
					// Update the layout of the whole menu.
					Debug.WriteLine("Updating layout ...");

					m_owner.LayoutAllChildren();
				}
				
				m_owner.MenuValueChanged(this);
				return true;
			}
			else
			{
//				Debug.WriteLine("Value changed. Ignored.");
			}

			return false;
		}

		#region Construction
		#region Cursor handling

		const int rightBorder = 3;
		const int bottomBorder = 3;

		LayoutCursor lc = new LayoutCursor();

		internal void LayoutChildren()
		{
			lc.Reset();

			bool bReadOnly = GetReadOnlyFromDependency();

			this._MakeReadOnly(bReadOnly);

			#region For unhidden controls that have one or more of their ancestors hidden.

			if((this.m_bShadowEnabled == true))
			{
				List<Control> listControlsToEnable = new List<Control>();
				Control cbCurrent = this;// as Container_Base;
				while(cbCurrent != null)
				{
					if(cbCurrent.Enabled == false)
						listControlsToEnable.Insert(0, cbCurrent);
					cbCurrent = cbCurrent.Parent;// as Container_Base;
				}
				foreach(Control control in listControlsToEnable)
				{
					control.Enabled = true;
				}
			}
			//			else
			//				this.Enabled = false;

			#endregion For unhidden controls that have one or more of their ancestors hidden.

			if(ChildContainer != null)
			{
				foreach(Container_Base childContainer in this.ChildContainer.Controls)
				{
					if(childContainer != null)
					{
						childContainer.LayoutChildren();		// Lay out it's children and determine it's size.
						// TODO: Rearrange this code so that the layout string can affect the root container too.
						ExecuteLayoutString(XmlHelper.GetKeyValue(childContainer.m_xObject, "pre_format"));
						childContainer.Location = new System.Drawing.Point(lc.X, lc.Y);

						lc.Print(childContainer.Width, childContainer.Height);
						ExecuteLayoutString(XmlHelper.GetKeyValue(childContainer.m_xObject, "post_format"));
					}
					else
					{
						throw new Exception("Unexpected child control in LayoutChildren.");
					}

				}

				int w = (lc._maxX + rightBorder);					// Required width of child container.
				int h = (lc._maxY + bottomBorder);
				int width = w + childGapRight + childGapLeft;		// Required width of this container to show all of child container.
				int height = h + childGapBottom + childGapTop;

				// If the width is set in the template, override the required width ...
				if(this.m_width != 0)
				{
					width = m_width;
				}

				width = Clamp(width, m_minWidth, m_maxWidth);

				if(this.m_height != 0)
				{
					height = m_height;
				}
				height = Clamp(height, m_minHeight, m_maxHeight);

				this.Width = width;
				this.Height = height;
			}
			return;
		}

		private bool GetReadOnlyFromDependency()
		{
			bool bReadonly = false;

			if((m_bXmlRequestReadOnly == true) ||
			(
			((m_owner != null) && (m_owner.m_bReadOnly == true)) &&			// if parent is readonly AND ...
			(m_bAlwaysEnabled == false)))									// we don't want it 'always on'
				bReadonly = true;											// make it read only.
			else	// (bReadOnly == false)
			{
				if(this.m_xDependsOn != null)
				{
					if(XmlHelper.GetKeyBool(this.m_xDependsOn, "value", true) == this.m_bDependencyTrueMeansEnable)
					{
						bReadonly = false;
					}
					else
					{
						bReadonly = true;
					}
				}
			}
			return bReadonly;
		}

		/// <summary>
		/// Some controls have dependencies, i.e. their 'readonly' state depends on the value of another control.
		/// This function recursively sets the 'readonly' state of all child containers if they are dependent on
		/// other containers.
		/// </summary>
		internal void ProcessDependencies()
		{
			if(ChildContainer != null)
			{
				foreach(Container_Base cb in this.ChildContainer.Controls)
				{
					if(cb != null)
					{
						cb.ProcessDependencies();
					}
					else
					{
						throw new Exception("Unexpected child control in LayoutChildren.");
					}
				}
			}

			this._MakeReadOnly(GetReadOnlyFromDependency());
		}


		void ExecuteLayoutString(string strFormatString)
		{
			if(strFormatString != null)
			{
				// No point preprocessing this because it's typically only called once per menu creation,
				// so preprocessing would be slower.
				strFormatString = strFormatString.Replace(" ", "");
				strFormatString = strFormatString.Replace("\t", "");
				strFormatString = strFormatString.Replace("\r", "");
				strFormatString = strFormatString.Replace("\n", "");

				string[] split = strFormatString.Split(new Char[] { ';' });

				foreach(string s in split)
				{
					if(s == "cr")
						lc.CR();
					else if(s.StartsWith("xtab"))
					{
						string strInt = s.Trim("xtab()".ToCharArray());
						lc.xtab(int.Parse(strInt));
					}
					else if(s.StartsWith("ytab"))
					{
						string strInt = s.Trim("ytab()".ToCharArray());
						lc.ytab(int.Parse(strInt));
					}
				}
			}
		}

		#endregion

		/// <summary>
		/// Remove all the controls from the child container of this control.
		/// </summary>
		/// <returns>False if there is no child container.</returns>
		protected bool RemoveControls()
		{
			if(ChildContainer != null)
			{
				ChildContainer.Controls.Clear();
				return true;
			}
			else
				return false;
		}

		/// <summary>
		/// Adds a control to the child container of this control.
		/// </summary>
		/// <param name="control">The control to add.</param>
		/// <returns>False if there is no child container.</returns>
		protected bool AddControl(Control control)
		{
			this.ChildContainer.Controls.Add(control);
			return true;
		}


		/// <summary>
		/// Create all the child controls beneath this control.
		/// </summary>
		/// <param name="bCreateChildren">Recursively create children.</param>
		/// <returns>False if there is no child container.</returns>
		protected bool CreateMenuChildren(bool bCreateChildren)
		{
//			return false;
			if (this.ChildContainer != null)
			{
				this.RemoveControls();

				// Create all the new children for this control.
				List<XmlElement> xChildElements = this.xChildren;	// There should be no null elements in this list.

				if (xChildElements != null)
				{
					foreach (XmlElement xChild in xChildElements)
					{
						if(xChild != null && xChild.Name == "container")
						{
							Container_Base newControl = m_owner.CreateMenuContainer(this, xChild, bCreateChildren);

							if(newControl != null)
							{
								m_owner.m_ignoreChangedEventHandler++;
								this.AddControl(newControl);
								m_owner.m_ignoreChangedEventHandler--;
							}
						}
					}
				}

				return true;
			}
			return false;
		}

		#endregion Construction

		#region Interface
		/// <summary>
		/// This is overriden in derived containers if they can have child containers.
		/// The derived class must return any children that are relevant to it's current state.
		/// </summary>
		internal virtual List<XmlElement>xChildren { get { return null; } }

		/// <summary>
		/// This is overriden in derived containers if they can have child containers.
		/// The derived class must return the control into which it places it's child containers.
		/// </summary>
		internal virtual Control ChildContainer { get { return null; } }

		// Cannot make these abstract without problems in the form designer. Grrr!

		/// <summary>
		/// This must be overriden in derived classes. It is the 'name' property for every container.
		/// </summary>
		virtual internal string ObjectName
		{
			set	// Removed the 'get' accessor because we now have 'display_name' and 'name' key-value pairs and I don't want any confusion between the two.
			{	// Therefore as far as the GUI is concerned, the name cannot be returned and is of no significance beyond what is shown on screen.
				throw new Exception("You must override the ObjectName property and give it a 'set' accessor in object type: '"+this.GetType().ToString()+"'\nYou are trying to 'set' the value to: '"+(value == null ? "NULL" : value.ToString())+"'");
			}
		}

		/// <summary>
		/// This must be overriden in derived classes. It is the 'value' property for every container.
		/// </summary>
		virtual internal string ObjectValue
		{
			get
			{
				throw new Exception("You must override the ObjectValue property and give it a 'get' accessor in object type: '" + this.GetType().ToString() + "'");

			}
			set
			{
				throw new Exception("You must override the ObjectValue property and give it a 'set' accessor in object type: '" + this.GetType().ToString() + "'\nYou are trying to 'set' the value to: '" + (value == null ? "NULL" : value.ToString()) + "'");
			}
		}

		/// <summary>
		/// This must be overriden in derived classes.
		/// </summary>
		virtual internal Control ObjectTooltipTarget
		{
			get
			{
				throw new Exception("You must override the ObjectValue property and give it a 'get' accessor in object type: '" + this.GetType().ToString() + "'");

			}
		}

		protected virtual string GetExtraStatusText()
		{
			return null;
		}

		private void _MakeReadOnly(bool bReadOnly)
		{
			this.m_bShadowEnabled = !bReadOnly;					// Default behaviour.
			MakeReadonly(bReadOnly);
		}


		protected virtual void MakeReadonly(bool bReadonly)
		{
			this.Enabled = !bReadonly;
		}

		#endregion Interface

		/// <summary>
		/// Generates and sets text in the status bar.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void Container_Base_Enter(object sender, EventArgs e)
		{
			StatusText = strStatusText + "   " + GetExtraStatusText();
		}

		/// <summary>
		/// Sets text in the status bar.
		/// </summary>
		/// <param name="strStatus">The text to set.</param>
		/// <returns></returns>
		private bool SetStatus(string strStatus)
		{
			//return m_owner.SetStatus(strStatus);
			return false;
		}

		string strStatusText = "NOTHING";

		private string StatusText
		{
			set
			{
				SetStatus(value + "    " + GetFullPath());
			}
		}

		private string GetFullPath()
		{
			return XmlHelper.FullNodePathByKey(this.m_xObject, null, "name", ".");
		}

		private void Container_Base_ClientSizeChanged(object sender, EventArgs e)
		{

		}
	}
}
