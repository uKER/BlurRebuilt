using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace application
{

	class GadgetManager
	{
		//List<FrameForm> m_gadgets = new List<FrameForm>();
		
		List<GadgetFrame> m_gadgetFrames = new List<GadgetFrame>();

		//BizarreToolbarPluginBaseFrame m_frame;
	

		BaseForm m_ownerBaseForm;
		System.Windows.Forms.Timer m_maintenanceTimer;

		public GadgetManager(BaseForm ownerBaseForm)
		{
			m_ownerBaseForm = ownerBaseForm;
			m_maintenanceTimer = new System.Windows.Forms.Timer();
			m_maintenanceTimer.Tick += MaintenanceTimerTick;

			CreateTestGadgetFrames();
		}

		private void CreateTestGadgetFrames()
		{
			int yOffset = 20;
			for (int c = 0; c < 4; c++)
			{
				GadgetFrame g = new GadgetFrame(10 + c * 10, new TestGadget());
				g.m_dockedTo = null;
				m_gadgetFrames.Add(g);
				g.Show();							// TODO: Move BoundsF initialisation from g.Load event.
				g.BoundsF = new RectangleF(50, yOffset, 200, 150);
				yOffset += g.Height + 20;
			}
		}

		void MaintenanceTimerTick(object Sender, EventArgs e)
		{
		}

		private int CompareGadgetFrameByY(GadgetFrame a, GadgetFrame b)
		{
			if (a.BoundsF.Y > b.BoundsF.Y)
				return 1;
			else if (a.BoundsF.Y == b.BoundsF.Y)
				return 0;

			return -1;
		}

		public bool UpdateGadgets(BaseForm gadgetFrameWithMouseIn, Toolbar bizarreToolbarWithMouseIn)
		{
			m_gadgetFrames.Sort(CompareGadgetFrameByY);					// Sort by Y coordinate.

			foreach (GadgetFrame gadgetFrame in m_gadgetFrames)
			{
				double oldFrameOpacity = gadgetFrame.NewOpacity;
				
				if(gadgetFrame == gadgetFrameWithMouseIn || gadgetFrame.m_baseGadget == gadgetFrameWithMouseIn)
				{
					gadgetFrame.HasBorder = true;

					if(gadgetFrame.IsBeingDragged == true)
					{
						if(gadgetFrame.m_dockedTo == null || bizarreToolbarWithMouseIn != null)
							gadgetFrame.m_dockedTo = bizarreToolbarWithMouseIn;
						else
						{
							if(gadgetFrame.m_dockedTo.Bounds.IntersectsWith(gadgetFrame.Bounds) == false)
								gadgetFrame.m_dockedTo = null;
						}

						gadgetFrame.Owner = gadgetFrame.m_dockedTo;

						if(gadgetFrame.m_dockedTo == null)
						{
							Rectangle clientBounds = gadgetFrame.GetNewClientRect(gadgetFrame.Bounds, 9, 9, 9, 9);
							gadgetFrame.panelClientArea.Bounds = clientBounds;
						}
						else
						{
							Rectangle clientBounds = gadgetFrame.GetNewClientRect(gadgetFrame.Bounds, 7, 0, 7, 0);
							gadgetFrame.panelClientArea.Bounds = clientBounds;
						}
					}
					gadgetFrame.m_targetOpacity = gadgetFrame.m_baseGadget.NewOpacity;

					if(gadgetFrame.m_dockedTo != null && (gadgetFrame.Focused == true || gadgetFrame.m_baseGadget.Focused == true))
						gadgetFrame.m_dockedTo.ChildHasFocus = true;

				}
				else
				{
					gadgetFrame.HasBorder = false;
					gadgetFrame.m_targetOpacity = 0.0;
				}
				//gadgetFrame.HasBorder = gadgetFrame.Visible;


				gadgetFrame.NewOpacity = Helpers.Gravitate(gadgetFrame.NewOpacity, gadgetFrame.m_targetOpacity, 0.1f, 0.1f);



				if(oldFrameOpacity != gadgetFrame.NewOpacity)
					gadgetFrame.Invalidate();


				gadgetFrame.UpdatePos();

			}
			return true;
		}
	}
}
