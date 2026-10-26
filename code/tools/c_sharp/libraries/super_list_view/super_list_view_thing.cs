using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.Windows.Forms;

namespace SuperControl
{

	public class SuperListViewItemControl
	{
		public enum ControlType
		{
			ComboBox,
			DateTimePicker,
			Button
		}

		ControlType m_controlType;
		internal Control m_control;
		public object Tag;					/// User data.
		internal object m_object;			/// A user parameter that can be passed through to the ManageControl delegate.

		internal SuperListView.DelegateManageControl ManageControl;	/// This gets called to populate the control.



		public SuperListViewItemControl(SuperListView superListView, ControlType controlType, object myObject, SuperListView.DelegateManageControl dpc)
		{
			m_controlType = controlType;
			m_object = myObject;
			ManageControl = dpc;

			switch (m_controlType)
			{
				case ControlType.ComboBox:
					{
						m_control = superListView.comboBox;
					}
					break;
				case ControlType.DateTimePicker:
					{
						m_control = superListView.dateTimePicker;
					}
					break;
				case ControlType.Button:
					{
						m_control = superListView.button;
					}
					break;

				default:
					{
						throw new Exception("ControlType unsupported in SuperListViewThing.GetControl.");
					}
			}

		}


		// TODO: Remember the original value here so it can be put back if escape is pressed.
		// Something like string m_strOriginalValue = lvsi.Text

		internal Control PrepareControl(SuperListView superListView, ListViewItem.ListViewSubItem lvsi)
		{
			ManageControl(m_object, m_control, lvsi, true);
			return this.m_control;
		}
	}


}
