using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;


namespace global
{

	public class SmartNotify
	{
		List<SmartNotifyWindow> m_windows;
		const int FADE_TIME = 100;

		Timer m_timer = null;

		private SmartNotify()
		{
			m_windows = new List<SmartNotifyWindow>();

			m_timer = new Timer(new TimerCallback(m_timer_Tick), null, Timeout.Infinite, 100);
			//m_timer.Tick += new EventHandler(m_timer_Tick);
		}


		void m_timer_Tick(object sender)
		{
			List<SmartNotifyWindow> deleteList = new List<SmartNotifyWindow>(m_windows.Count);

			foreach(SmartNotifyWindow snw in m_windows)
			{
				snw.m_iVisibility++;
				float fOpacity = 1.0f * snw.m_iVisibility / FADE_TIME;

				if(fOpacity < 0.0f)
					fOpacity = -fOpacity;

				SmartNotifyWindow.MyDelegateCallback del = new SmartNotifyWindow.MyDelegateCallback(snw.OpacityCallback);
				IAsyncResult ar = snw.BeginInvoke(del, new object[]{fOpacity});
				snw.EndInvoke(ar);

				if(snw.m_iVisibility > FADE_TIME)
				{
					deleteList.Add(snw);
				}
			}

			foreach(SmartNotifyWindow snw in deleteList)
			{
				m_windows.Remove(snw);
				snw.Close();
			}

			if(m_windows.Count == 0)
			{
				m_timer.Change(Timeout.Infinite, Timeout.Infinite);
			}

		}




		static SmartNotify m_smartNotify = null;


		//public delegate SmartNotifyWindow defGetTheFucker();

//		public SmartNotifyWindow GetTheFucker()
//		{
//			return new SmartNotifyWindow("Fucker");
//		}

		public static bool Add(string strMessage, System.Windows.Forms.Form owner)
		{
			if(m_smartNotify == null)
			{
				m_smartNotify = new SmartNotify();
			}

//			SmartNotifyWindow snw = owner.Invoke(new defGetTheFucker(m_smartNotify.GetTheFucker)) as SmartNotifyWindow;
			SmartNotifyWindow snw = new SmartNotifyWindow(strMessage);
			snw.Visible = true;

			snw.Show(owner);
			snw.m_iVisibility = -FADE_TIME;

			m_smartNotify.m_windows.Add(snw);

//			m_smartNotify.m_timer.Start();			// Might already be running. Check this is okay.
			m_smartNotify.m_timer.Change(100, 100);

			return true;

		}
	}
}