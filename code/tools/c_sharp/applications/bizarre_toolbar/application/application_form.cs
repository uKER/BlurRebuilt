using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using Windows.Message;

namespace application
{
	public partial class BizarreToolbarManager : BaseForm, IMessageFilter
	{
		BaseForm m_formWithMouseIn = null;

		List<Toolbar> m_bizarreToolbarList;

		GadgetManager m_gadgetManager;

		public BizarreToolbarManager() : base()
		{
			InitializeComponent();

			//m_bizarreToolbarList = new List<BizarreToolbar>();
			m_bizarreToolbarList = new List<Toolbar>();
			//Toolbar bizarreToolbar = new Toolbar(14, new BizarreToolbarClient());
			Toolbar bizarreToolbar = new Toolbar(14, null);

			Rectangle clientBounds = bizarreToolbar.GetNewClientRect(bizarreToolbar.Bounds, 0, 0, 0, 5);
			bizarreToolbar.panelClientArea.Bounds = clientBounds;

			m_bizarreToolbarList.Add(bizarreToolbar);
			bizarreToolbar.Show();

			m_gadgetManager = new GadgetManager(this);
		}

		Toolbar bizarreToolbarWithMouseIn = null;
		public void Application_Idle(object sender, EventArgs e)
		{
			bizarreToolbarWithMouseIn = null;

			foreach(Toolbar bizarreToolbar in m_bizarreToolbarList)
			{
				if(bizarreToolbar.Bounds.Contains(MousePosition))
					bizarreToolbarWithMouseIn = bizarreToolbar;

				bizarreToolbar.DoIt();
				bizarreToolbar.ChildHasFocus = false;		// Let the Gadgets update this for the next frame themselves.
				bizarreToolbar.UpdatePos();
			}

			m_gadgetManager.UpdateGadgets(m_formWithMouseIn, bizarreToolbarWithMouseIn);

			System.Threading.Thread.Sleep(10);

		}

		#region IMessageFilter Members

		public bool PreFilterMessage(ref Message m)
		{
			Debug.WriteLine("MSG:hWND : " + m.Msg.ToString() + "  " + m.HWnd.ToString());

			if(m.Msg == (int)WM.MouseMove)
				m_formWithMouseIn = GetBaseFormFromHandle(m.HWnd);		// If m_formWithMouseIn changes here, we can send a MouseEnter event.

				//Debug.Assert(m_formWithMouseIn != null);	// 'NotifyIcon' may cause problems.

				if(m_formWithMouseIn != null)
				{
					bool bResult = m_formWithMouseIn.ProcessCustomMessage(ref m);
					Debug.WriteLine(m_formWithMouseIn.Name + " " + m.HWnd.ToString());
					return bResult;
				}

			return false;
		}

		BaseForm GetBaseFormFromHandle(IntPtr handle)
		{
			Control c = Control.FromChildHandle(handle);

			while((c != null) && (c is BaseForm != true))	// 'NotifyIcon' context menu is not parented to a form.
				c = c.Parent;

			return c as BaseForm;
		}


		#endregion

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Application.Exit();
		}

		private void contextMenuStripNotifyIcon_MouseMove(object sender, MouseEventArgs e)
		{

		}
	}
}