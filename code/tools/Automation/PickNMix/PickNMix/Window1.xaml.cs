using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using XtInterop;

namespace PickNMix
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        private Database database = new Database();

        public Window1()
        {
            InitializeComponent();
        }

        private void RefreshData()
        {
            GameConfig.Text = database.GameDefinitionFile;
            SorceryPath.Text = database.SorceryDatabase;

            Types.ItemsSource = database.Data;
        }

        private void Button_FindSorcery(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.OpenFileDialog ofd = new System.Windows.Forms.OpenFileDialog();
            ofd.Filter = "Sorcery Database (config.xtp)|config.xtp";
            ofd.AutoUpgradeEnabled = true;
            ofd.CheckFileExists = true;
            System.Windows.Forms.DialogResult dr = ofd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                database.SorceryDatabase = ofd.FileName;

                RefreshData();
            }
        }

        private void Button_FindGameDef(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.OpenFileDialog ofd = new System.Windows.Forms.OpenFileDialog();
            ofd.Filter = "Game Def (*.gdef)|*.gdef";
            ofd.AutoUpgradeEnabled = true;
            ofd.CheckFileExists = true;
            System.Windows.Forms.DialogResult dr = ofd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                database.GameDefinitionFile = ofd.FileName;

                RefreshData();
            }
        }

        private void MouseDoubleClick_ToggleItemUse(object sender, MouseButtonEventArgs e)
        {
            ListBox lb = sender as ListBox;

            PropertyInfo pi = typeof(ListBox).GetProperty("AnchorItem", BindingFlags.NonPublic | BindingFlags.Instance);
            if (pi != null)
            {
                object o1 = pi.GetValue(lb, null);

                Database.Record o = pi.GetValue(lb, null) as Database.Record;

                o.Use = !o.Use;
            }
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            foreach (Database.Record o in Available.SelectedItems)
            {
                o.Use = true;
            }
            Available.SelectedItems.Clear();
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            foreach (Database.Record o in Picked.SelectedItems)
            {
                o.Use = false;
            }
            Picked.SelectedItems.Clear();
        }

        private void MenuItem_OpenFile(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.OpenFileDialog ofd = new System.Windows.Forms.OpenFileDialog();
            ofd.Filter = "Pick n Mix file (*.pnm)|*.pnm";
            ofd.AutoUpgradeEnabled = true;
            ofd.CheckFileExists = true;
            System.Windows.Forms.DialogResult dr = ofd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                database.Load(ofd.FileName);

                RefreshData();
            }
        }

        private void MenuItem_SaveFile(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.SaveFileDialog ofd = new System.Windows.Forms.SaveFileDialog();
            ofd.Filter = "Pick n Mix file (*.pnm)|*.pnm";
            ofd.AutoUpgradeEnabled = true;
            System.Windows.Forms.DialogResult dr = ofd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                database.Save(ofd.FileName);
            }
        }
    }
}
