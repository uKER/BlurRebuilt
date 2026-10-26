using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace LuaDebugger
{
    /// <summary>
    /// Interaction logic for UserTreeListView.xaml
    /// </summary>
    public partial class UserTreeListView : UserControl
    {
        public UserTreeListView()
        {
            InitializeComponent();

            TLV.AddHandler(Thumb.DragDeltaEvent, new DragDeltaEventHandler(Thumb_DragDelta), true);
        }

        void Thumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            TLV.InvalidateVisual();

            Console.WriteLine("Thumb_DragDelta");
        }
    }

    public class UserItemsPresenter : ItemsPresenter
    {
        protected override void OnRender(DrawingContext drawingContext)
        {
            Console.WriteLine("OnRender");

            Pen myPen = new Pen(Brushes.LightGray, 1);

            for (int i = 0; i < ActualHeight; i += 14)
            {
                drawingContext.DrawLine(myPen, new Point(0, i), new Point(ActualWidth, i));
            }

            ScrollViewer sv = this.Parent as ScrollViewer;
            DockPanel dp = sv.Parent as DockPanel;

            GridViewHeaderRowPresenter gvhrp = dp.Children[0] as GridViewHeaderRowPresenter;

            double xPos = 0;

            foreach (var c in gvhrp.Columns)
            {
                xPos += c.ActualWidth;

                drawingContext.DrawLine(myPen, new Point(xPos, 0), new Point(xPos, ActualHeight));
            }

            base.OnRender(drawingContext);
        }
    }
}
