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
using DataLib.DataModel;
using Alienbrain;
using System.Diagnostics;
using System.Collections.ObjectModel;
using System.Xml;

namespace FileTagManager3.LoadOnDemand
{
	/// <summary>
	/// Interaction logic for LoadOnDemandUserControl.xaml
	/// </summary>
	public partial class LoadOnDemandUserControl : UserControl
	{
		ObservableCollection<AbData> m_abObservableListFiles = new ObservableCollection<AbData>();

		public ObservableCollection<AbData> AbObservableListFiles
		{
			get { return m_abObservableListFiles; }
			set
			{
				m_abObservableListFiles = value;
				Debug.WriteLine("Set AbObservableListFiles in LoadOnDemandUserControl.");
			}
		}


		public LoadOnDemandUserControl()
		{

			InitializeComponent();

			if(System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
			{
				Content = "In Design Mode";
			}
			else
			{
				AbData.m_alienbrainInterface = AlienBrainInterface.CreateInterface();
				AbData.m_alienbrainInterface.Initialise(AlienbrainRequest.Priority.Normal);
				//string strRootFolder = m_alienbrainInterface.LoadProject(AlienbrainRequest.Priority.Normal, null);
				//string strRootFolder1 = AbData.m_alienbrainInterface.LoadProject(AlienbrainRequest.Priority.Normal, "PGR4");
				string strRootFolder1 = AbData.m_alienbrainInterface.LoadProject(AlienbrainRequest.Priority.Normal, null);
				//string strRootFolder2 = MainWindow.m_alienbrainInterface.LoadProject(AlienbrainRequest.Priority.Normal, "Nightwing");

				AlienbrainInfo abInfo1 = AbData.m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, strRootFolder1, true);
				//AlienbrainInfo abInfo2 = MainWindow.m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, strRootFolder2, true);

				List<AbData> list = new List<AbData>();
				list.Add(new AbData(abInfo1));
				//list.Add(new AbData(abInfo2));

				ProjectViewModel viewModel = new ProjectViewModel(list);

				base.DataContext = viewModel;
			}
		}


		object m_selectedItem = null;

		public object SelectedItem
		{
			get { return m_selectedItem; }
		}

		private void TreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
		{
			AbDataViewModel dataView = e.NewValue as AbDataViewModel;
			AbObservableListFiles = dataView.AbObservableListFiles;

			m_selectedItem = e.NewValue;
		}

		/// <summary>
		/// Walks up the tree until it finds a file matching strTemplateFilename.
		/// If found, it loads the file as an Xml document and returns it.
		/// </summary>
		/// <param name="strTemplateFilename"></param>
		/// <returns></returns>
		internal XmlDocument GetTemplate(string strTemplateFilename)
		{
			throw new NotImplementedException();
		}
	}
}