using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;

//using System;
//using System.Collections.ObjectModel;
//using System.ComponentModel;



namespace FileTagManager3.LoadOnDemand
{
    /// <summary>
    /// Base class for all ViewModel classes displayed by TreeViewItems.  
    /// This acts as an adapter between a raw data object and a TreeViewItem.
    /// </summary>
	public class TreeViewItemViewModel : INotifyPropertyChanged
	{
		#region Data

		static readonly TreeViewItemViewModel DummyChild = new TreeViewItemViewModel();

		readonly ObservableCollection<TreeViewItemViewModel> _children;
		readonly TreeViewItemViewModel _parent;

		bool _isExpanded;
		bool _isSelected;

		#endregion // Data

		#region Constructors

		protected TreeViewItemViewModel(TreeViewItemViewModel parent, bool lazyLoadChildren)
		{
			_parent = parent;

			_children = new ObservableCollection<TreeViewItemViewModel>();

			if(lazyLoadChildren)
				_children.Add(DummyChild);
		}

		// This is used to create the DummyChild instance.
		private TreeViewItemViewModel()
		{
		}

		#endregion // Constructors

		#region Presentation Members

		#region Children

		/// <summary>
		/// Returns the logical child items of this object.
		/// </summary>
		public ObservableCollection<TreeViewItemViewModel> Children
		{
			get { return _children; }
		}

		#endregion // Children

		#region HasLoadedChildren

		/// <summary>
		/// Returns true if this object's Children have not yet been populated.
		/// </summary>
		public bool HasDummyChild
		{
			get { return this.Children.Count == 1 && this.Children[0] == DummyChild; }
		}

		#endregion // HasLoadedChildren

		#region IsExpanded

		/// <summary>
		/// Gets/sets whether the TreeViewItem 
		/// associated with this object is expanded.
		/// </summary>
		public bool IsExpanded
		{
			get { return _isExpanded; }
			set
			{
				if(value != _isExpanded)
				{
					_isExpanded = value;
					this.OnPropertyChanged("IsExpanded");
				}

				// Expand all the way up to the root.
				//if(_isExpanded && _parent != null)
				//    _parent.IsExpanded = true;

				this.OnPropertyChanged("IsExpanded");
				//LazyLoadChildrenIfNecessary();
			}


		}

		#endregion // IsExpanded

		#region IsSelected

		/// <summary>
		/// Gets/sets whether the TreeViewItem 
		/// associated with this object is selected.
		/// </summary>
		public bool IsSelected
		{
			get { return _isSelected; }
			set
			{
				if(value != _isSelected)
				{
					_isSelected = value;
					this.OnPropertyChanged("IsSelected");
					//LazyLoadChildrenIfNecessary();
				}
			}
		}

		#endregion // IsSelected

		#region LoadChildren

		/// <summary>
		/// Invoked when the child items need to be loaded on demand.
		/// Subclasses can override this to populate the Children collection.
		/// </summary>
		protected virtual void LoadChildren()
		{
		}

		/// <summary>
		/// Always loads the children. Will stay that way until I sort out how to selectively
		/// refresh using databinding.
		/// </summary>
		protected void LazyLoadChildrenIfNecessary()
		{
			List<object> removeList = new List<object>();

			foreach(object o in this.Children)
			{
				removeList.Add(o);
			}

			foreach(TreeViewItemViewModel o in removeList)
			{
				this.Children.Remove(o);
			}
			//this.Children.Clear();
			this.LoadChildren();

			return;

			// Lazy load the child items, if necessary.
			if(this.HasDummyChild)
			{
				this.Children.Remove(DummyChild);

				this.LoadChildren();
			}
		}

		#endregion // LoadChildren

		#region Parent

		public TreeViewItemViewModel Parent
		{
			get { return _parent; }
		}

		#endregion // Parent

		#endregion // Presentation Members

		#region INotifyPropertyChanged Members

		public event PropertyChangedEventHandler PropertyChanged;

		protected virtual void OnPropertyChanged(string propertyName)
		{
			if(this.PropertyChanged != null)
				this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
		}

		#endregion // INotifyPropertyChanged Members
	}
}