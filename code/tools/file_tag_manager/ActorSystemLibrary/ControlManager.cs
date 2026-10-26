using System;
using System.Windows.Forms;
using System.Collections;
using System.Xml;

namespace ActorSystem
{
	/// <summary>
	/// Summary description for ControlManager.
	/// </summary>
	public class ControlManager
	{
		public GroupBox m_groupBox = new GroupBox();

		private ArrayList m_controlList = new ArrayList();

		public ControlManager()
		{
//			m_parentControlPanel = parentControlPanel;
		}

		private class ControlProperties
		{
//			public ControlProperties(Control control, XmlAttribute attrValue)
			public ControlProperties(Control control, object thing)				// TODO: Maybe. Find a use for 'thing'.
			{
				m_control = control;
//				m_attrValue = attrValue;
			}

			Control m_control;
//			XmlAttribute m_attrValue;				// The attribute that holds the value of this control.

			public Control Control{get{return m_control;} set{m_control=value;}}
//			public XmlAttribute AttrValue{get{return m_attrValue;}}

			public bool m_dying = false;

			public int m_life = 0;

		}


		public bool AddControl(Control control, Object thing)
		{
			System.Drawing.Point pt = new System.Drawing.Point(10, 10);
			ControlProperties cp = new ControlProperties(control, thing);

			Control lastControl = null;

			if(m_controlList.Count != 0)
			{
				lastControl = (m_controlList[m_controlList.Count - 1] as ControlProperties).Control;
				pt.X = lastControl.Location.X;
				pt.Y = lastControl.Location.Y + lastControl.Height;
			}

			control.Location = pt;
			m_controlList.Add(cp);
//			m_parentControlPanel.Controls.Add(control);
			m_groupBox.Controls.Add(control);

			return true;
		}

		public bool ClearControls()
		{
			foreach(ControlProperties cp in m_controlList)
			{
//				m_parentControlPanel.Controls.Remove(cp.Control);
				m_groupBox.Controls.Remove(cp.Control);
				cp.Control = null;
//				cp.Control.Dispose();
			}

			m_controlList.Clear();

			return true;
		}

		public bool SlowClearControls()
		{
			foreach(ControlProperties cp in m_controlList)
			{
				cp.m_dying = true;
				cp.m_life = 4;
			}

			return true;
		}

		public bool UpdateControls(ref int width, ref int height)
		{
			ControlProperties lastDyingControlProperties = null;
			ControlProperties lastControlProperties = null;

			width = 0;
			height = 0;

			foreach(ControlProperties cp in m_controlList)
			{
				if(lastControlProperties != null)
				{
					if(cp.Control.Location.Y > lastControlProperties.Control.Location.Y + lastControlProperties.Control.Height)
					{
						System.Drawing.Point pt = cp.Control.Location;

						pt.Y -= 10; if(pt.Y < lastControlProperties.Control.Location.Y + lastControlProperties.Control.Height) pt.Y = lastControlProperties.Control.Location.Y + lastControlProperties.Control.Height;
						cp.Control.Location = pt;
						cp.Control.Invalidate();
					}
				}
				else if(cp.Control.Location.Y > 0)
				{
					System.Drawing.Point pt = cp.Control.Location;

					pt.Y -= 10; if(pt.Y < 0) pt.Y = 0;

					cp.Control.Location = pt;
					cp.Control.Invalidate();
				}

				if(cp.Control.Width > width)
					width = cp.Control.Width;

				height = cp.Control.Location.Y + cp.Control.Height;
				
				if(cp.m_dying == true)
				{
					lastDyingControlProperties = cp;
				}

				
				
				lastControlProperties = cp;


			}

			if(lastDyingControlProperties != null)
			{
				lastDyingControlProperties.m_life--;
				System.Drawing.Point pt = lastDyingControlProperties.Control.Location;

				pt.X -= 70;

				lastDyingControlProperties.Control.Location = pt;
				lastDyingControlProperties.Control.Invalidate();

				if(lastDyingControlProperties.m_life == 0)
				{
//					m_parentControlPanel.Controls.Remove(lastDyingControlProperties.Control);
					m_groupBox.Controls.Remove(lastDyingControlProperties.Control);
					m_controlList.Remove(lastDyingControlProperties);
				}
			}
			return true;

		}
	}
}
