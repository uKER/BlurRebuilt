// <copyright file="MemoryStructures.cs" company="Bizarre Creations">
//     Copyright (c) Bizarre Creations. All rights reserved.
// </copyright>
// <author>Roger Perkins</author>

namespace MemoryTracker2
{
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;

    /// <summary>
    /// Holds informations regarding a single memory allocation
    /// </summary>
    public class MemAllocItem
    {
        /// <summary>
        /// Gets or sets the ContextID
        /// </summary>
        public int ContextID
        {
            get; set;
        }

        /// <summary>
        /// Gets or sets the Heap
        /// </summary>
        public int Heap
        {
            get;
            set;
        }

        public uint AllocAddress
        {
            get;
            set;
        }

        public string AllocAddressString
        {
            get
            {
                return string.Format("{0:x}", AllocAddress);
            }
        }

        public bool IsInSmallBlock
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the AllocationSize
        /// </summary>
        public int AllocationSize
        {
            get; set;
        }

        /// <summary>
        /// Gets or sets the AllocationWaste
        /// </summary>
        public int AllocationWaste
        {
            get; set;
        }

        /// <summary>
        /// Gets or sets the Attribs
        /// </summary>
        public int Attribs
        {
            get; set;
        }

        /// <summary>
        /// Gets or sets the Tag
        /// </summary>
        public int Tag
        {
            get; set;
        }

        /// <summary>
        /// Gets or sets the CallStackDepth
        /// </summary>
        public int CallStackDepth
        {
            get; set;
        }

        /// <summary>
        /// Gets or sets the Address
        /// </summary>
        public uint Address
        {
            get; set;
        }

         /// <summary>
        /// Gets or sets the CallStack array
        /// </summary>
        public NameResolutionItem[] CallStackNRI
        {
            get; set;
        }

        public string TopFunctionName
        {
            get
            {
                if (CallStackNRI == null || CallStackNRI.Length == 0)
                    return "?";

                if (CallStackNRI[0].Address != 0)
                {
                    string name;

                    if (CallStackNRI.Length > 1)
                        name = CallStackNRI[1].Name;
                    else
                        name = CallStackNRI[0].Name;

                    //if (!useOffset && (name == "operator new[]"))
                    //{
                    //    name = stack[1].Name;
                    //}

                    return name;
                }
                else
                {
                    return "?";
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether the allocation was in physical memory
        /// </summary>
        public bool IsPhysical
        {
            get
            {
                return this.Attribs >= 1;
            }
        }

        /// <summary>
        /// Gets the type of the allocation
        /// </summary>
        public string Type
        {
            get
            {
                return this.IsPhysical ? "P" : "V";
            }
        }
    }

    /// <summary>
    /// Holds information about a context, which is a grouping
    /// </summary>
    public class ContextNode : INotifyPropertyChanged
    {
        /// <summary>
        /// Holds an index to a context node's parent context, else -1 if one doesn't exist
        /// </summary>
        private int parentIndex;

        /// <summary>
        /// Holds a collection of all child context nodes
        /// </summary>
        private ObservableCollection<ContextNode> children;

        /// <summary>
        /// If false, items from this context are not shown in the list of allocations
        /// </summary>
        private bool show;
        
        /// <summary>
        /// Holds the total number of allocations tracked within this context
        /// </summary>
        private int allocationCount;
        
        /// <summary>
        /// Holds the total number of allocations tracked within this context and all of it's children
        /// </summary>
        private int subAllocationCount;

        public ContextNode()
        {
            this.children = new ObservableCollection<ContextNode>();

            this.Show = true;

            this.AllocationCount = 0;
            this.AllocationCountMax = 0;

            this.SubAllocationCount = 0;

            this.AllocatedMemory = 0;
            this.AllocatedMemoryMax = 0;

            this.WastedMemory = 0;
        }

        /// <summary>
        /// An event that we can fire if we change any values to notify others of the change
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Gets or sets the ParentIndex
        /// </summary>
        public int ParentIndex
        {
            get
            {
                return this.parentIndex;
            }

            set
            {
                this.parentIndex = value;
            }
        }

        public ObservableCollection<ContextNode> Children
        {
            get
            {
                return this.children;
            }

            set
            {
                this.children = value;
            }
        }

        /// <summary>
        /// Gets or sets the FullName of this context
        /// </summary>
        public string FullName
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the short name of this context
        /// </summary>
        public string ShortName
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets a value indicating whether we should filter out items tracked under this context
        /// </summary>
        public bool Show
        {
            get
            {
                return this.show;
            }

            set
            {
                this.show = value;

                if (this.children.Count > 0)
                {
                    foreach (ContextNode cn in this.children)
                    {
                        cn.Show = value;
                    }
                }

                this.Notify("Show");
            }
        }

        /// <summary>
        /// Gets or sets the amount of allocated memory tracked by this context
        /// </summary>
        public int AllocatedMemory
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the largest amount of memory we've seen in this context
        /// </summary>
        public int AllocatedMemoryMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the amount of memory wasted in this context
        /// </summary>
        public int WastedMemory
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the number of allocations tracked by this context
        /// </summary>
        public int AllocationCount
        {
            get
            {
                return this.allocationCount;
            }

            set
            {
                this.allocationCount = value;
                this.Notify("AllocationCount");
                this.Notify("ContextTreeString");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this context is expanded in the tree view
        /// </summary>
        public bool Expanded
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the number of allocations tracked by this context and it's children
        /// </summary>
        public int SubAllocationCount
        {
            get
            {
                return this.subAllocationCount;
            }

            set
            {
                this.subAllocationCount = value;
                this.Notify("SubAllocationCount");
                this.Notify("ContextTreeString");
            }
        }

        /// <summary>
        /// Gets or sets a value holding the most allocations at one time in this context
        /// </summary>
        public int AllocationCountMax
        {
            get;
            set;
        }

        public ObservableCollection<ContextNode> ChildrenList
        {
            get
            {
                return this.children;
            }
        }

        //public IEnumerable<ContextNode> Children
        //{
        //    get
        //    {
        //        return this.children;
        //    }
        //}

        public string ContextTreeString
        {
            get
            {
                if ((this.SubAllocationCount + this.AllocationCount) > 0)
                {
                    if (this.SubAllocationCount > 0)
                    {
                        return string.Format("{0} ({1})({2})", this.ShortName, this.AllocationCount, this.SubAllocationCount + this.AllocationCount);
                    }
                    else
                    {
                        return string.Format("{0} ({1})", this.ShortName, this.AllocationCount);
                    }
                }
                else
                {
                    return this.ShortName;
                }
            }
        }

        public override string ToString()
        {
            return this.FullName;
        }

        /// <summary>
        /// Adds a context node to the list of child context node's on this context
        /// </summary>
        /// <param name="cn">The context to add as a child</param>
        public void AddChild(ContextNode cn)
        {
            this.children.Add(cn);
        }

        /// <summary>
        /// Notify any subscrubed listeners that we have changed the value of a property
        /// </summary>
        /// <param name="propname">The name of the property that has been changed</param>
        protected void Notify(string propname)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propname));
            }
        }
    }

    /// <summary>
    /// Holds the association between and address and a function name
    /// </summary>
    public class NameResolutionItem
    {
        /// <summary>
        /// Gets or sets the Address
        /// </summary>
        public uint Address
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the Line
        /// </summary>
        public int Line
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the Filename
        /// </summary>
        public string Filename
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the Name
        /// </summary>
        public string Name
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the name of function
        /// </summary>
        /// <returns>The Name of the function</returns>
        public override string ToString()
        {
            return this.Name;
        }
    }
}