using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using DataLib.DataModel;
using Alienbrain;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Windows.Data;

namespace FileTagManager3.LoadOnDemand
{
	public class AbDataViewModel : TreeViewItemViewModel
	{
		public readonly AbData m_abData;

		public AbDataViewModel(AbData abData, AbDataViewModel parent)
			: base(parent, true)
		{
			m_abData = abData;

			this.PropertyChanged += new System.ComponentModel.PropertyChangedEventHandler(AbDataViewModel_PropertyChanged);
		}

		void AbDataViewModel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			if(e.PropertyName == "IsSelected")
			{
				// TODO: Update the data source.
				if(this.IsSelected == true)
				{
					this.LazyLoadChildrenIfNecessary();
				}
			}
			else if(e.PropertyName == "IsExpanded")
			{
				if(this.IsExpanded == true)
				{
					this.LazyLoadChildrenIfNecessary();
				}
			}
		}

		// Used by tree view.
		public string Name
		{
			get { return m_abData.Name; }
		}

		//static ObservableCollection<AbData> listViewContents = new ObservableCollection<AbData>();

		ObservableCollection<AbData> m_abObservableListFiles = new ObservableCollection<AbData>();

		public ObservableCollection<AbData> AbObservableListFiles
		{
			get { return m_abObservableListFiles; }
			set { m_abObservableListFiles = value; }
		}

		protected override void LoadChildren()
		{
			m_abData.GetChildren();

			foreach(AbData abData in m_abData.m_abListFolders)
			{
				base.Children.Add(new AbDataViewModel(abData, this));
			}

			m_abObservableListFiles.Clear();

			foreach(AbData abData in m_abData.m_abListFiles)
			{
				if(abData.Name.EndsWith(AbData.TAG_EXTENSION) == false && abData.Name.EndsWith(AbData.LOCK_EXTENSION) == false)
					m_abObservableListFiles.Add(abData);
			}
		}
	}
}

