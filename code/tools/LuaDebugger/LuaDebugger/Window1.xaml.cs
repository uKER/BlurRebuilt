using System;
using System.Collections.Generic;
using System.Linq;
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

using Microsoft.Win32;

namespace LuaDebugger
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        public Window1()
        {
            InitializeComponent();

            Callstack.DataContext = Debugger.debugger.callstack;
            Locals.TLV.DataContext = Debugger.debugger.localVariables;
            FileTree.DataContext = Debugger.debugger.fileTree;
        }

        private void Button_Connect(object sender, RoutedEventArgs e)
        {
            Debugger.debugger.Connect(this);
        }

        private void Continue_Click(object sender, RoutedEventArgs e)
        {
            Debugger.debugger.Go();
        }

        private void BreakAll_Click(object sender, RoutedEventArgs e)
        {
            Debugger.debugger.BreakAll();
        }

        private void StepOut_Click(object sender, RoutedEventArgs e)
        {
            Debugger.debugger.StepOut();
        }

        private void StepOver_Click(object sender, RoutedEventArgs e)
        {
            Debugger.debugger.StepOver();
        }

        private void StepInto_Click(object sender, RoutedEventArgs e)
        {
            Debugger.debugger.StepInto();
        }

        public void ClearCurrentFile(string currentFile)
        {
            foreach (TabItem ti in FileTab.Items)
            {
                UserLuaFileViewer ulfv = ti.Tag as UserLuaFileViewer;

                if (ulfv.Filename.ToLower().CompareTo(currentFile.ToLower()) == 0)
                {
                    ulfv.InvalidateVisual();
                    return;
                }
            }
        }

        public void OpenFile(string filename, int line)
        {
            bool found = false;
            string shortName;

            if (filename.ToLower().StartsWith(Debugger.debugger.CodePath.ToLower()))
                shortName = filename.Substring(Debugger.debugger.CodePath.Length+1);
            else
                shortName = filename;

            foreach (TabItem ti in FileTab.Items)
            {
                UserLuaFileViewer ulfv = ti.Tag as UserLuaFileViewer;

                if (ulfv.Filename.ToLower().CompareTo(shortName.ToLower()) == 0)
                {
                    found = true;

                    if (line > 0)
                        ulfv.GotoLine(line);

                    FileTab.SelectedItem = ti;

                    return;
                }
            }

            if (found)
                return;

            // File we wanted wasn't open so we open one!!!!
            {
                TabItem ti = new TabItem();

                ti.Header = shortName;
                FileTab.Items.Add(ti);

                UserLuaFileViewer ulfv = new UserLuaFileViewer();
                ulfv.LoadFile(Debugger.debugger.CodePath, shortName);
                ulfv.Width = Double.NaN;
                ulfv.Height = Double.NaN;
                ulfv.VerticalAlignment = VerticalAlignment.Stretch;
                ulfv.HorizontalAlignment = HorizontalAlignment.Stretch;
                
                if (line > 0)
                    ulfv.GotoLine(line);

                ti.Content = ulfv;
                ti.Tag = ulfv;

                FileTab.SelectedIndex = FileTab.Items.Count - 1;
            }
        }

        private void OpenProject_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();

            ofd.Title = "Open Project File";
            ofd.Filter = "Lua Debugger Project Files (*.lproj)|*.lproj|All Files (*.*)|*.*";

            bool? ok = ofd.ShowDialog();

            if (ok == true)
            {
                if (Debugger.debugger.LoadData(ofd.FileName))
                {
                    //outlinerWindow.Show(dockPanel, DockState.DockLeft);
                    //projectExplorer.Show(outlinerWindow.Pane, outlinerWindow);
                    //autoWindow.Show(dockPanel);
                    //breakpointWindow.Show(autoWindow.Pane, DockAlignment.Right, 0.5);
                    //callstackWindow.Show(breakpointWindow.Pane, breakpointWindow);
                }
            }
        }

        private void SaveProject_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();

            sfd.Title = "Save Project File";
            sfd.Filter = "Lua Debugger Project Files (*.lproj)|*.lproj|All Files (*.*)|*.*";
            sfd.InitialDirectory = Debugger.debugger.CodePath;

            bool? ok = sfd.ShowDialog();

            if (ok == true)
            {
                Debugger.debugger.SaveData(sfd.FileName);
            }
        }

        private void TextBlock_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ClickCount > 1)
            {
                TextBlock tb = sender as TextBlock;

                if (tb.DataContext is Debugger.LuaFile)
                {
                    Debugger.LuaFile lf = tb.DataContext as Debugger.LuaFile;

                    OpenFile(lf.Path, -1);
                }
            }
        }

        private void ProjectContextMenu_Click(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.FolderBrowserDialog fbd = new System.Windows.Forms.FolderBrowserDialog();

            System.Windows.Forms.DialogResult dr = fbd.ShowDialog();

            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                Debugger.debugger.CodePath = fbd.SelectedPath;
            }
        }
    }
}
