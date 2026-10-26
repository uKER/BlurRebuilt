#ifndef KDTREE_H
#define KDTREE_H

//#include <vector>

class KdTreeNode;

typedef Btl::Vector<KdTreeNode* > KdTreeNodeVector;

enum Axes
{
  X_AXIS = 0,
  Y_AXIS = 1,
  Z_AXIS = 2
};

class KdTreeFindNode
{
public:
  KdTreeFindNode(void)
  {
    mNode = 0;
    mDistance = 0;
  }
  KdTreeNode  *mNode;
  float        mDistance;
};

class KdTreeNode
{
public:

  KdTreeNode(float x,float y,float z,float radius,void *userData,unsigned int index)
  {
    mX = x;
    mY = y;
    mZ = z;
    mRadius = radius;
    mUserData = userData;
    mIndex = index;
    mLeft = 0;
    mRight = 0;
  };

	~KdTreeNode(void)
  {
  }


  void add(KdTreeNode *node,Axes dim)
  {
    switch ( dim )
    {
      case X_AXIS:
        if ( node->getX() <= getX() )
        {
          if ( mLeft )
            mLeft->add(node,Y_AXIS);
          else
            mLeft = node;
        }
        else
        {
          if ( mRight )
            mRight->add(node,Y_AXIS);
          else
            mRight = node;
        }
        break;
      case Y_AXIS:
        if ( node->getY() <= getY() )
        {
          if ( mLeft )
            mLeft->add(node,Z_AXIS);
          else
            mLeft = node;
        }
        else
        {
          if ( mRight )
            mRight->add(node,Z_AXIS);
          else
            mRight = node;
        }
        break;
      case Z_AXIS:
        if ( node->getZ() <= getZ() )
        {
          if ( mLeft )
            mLeft->add(node,X_AXIS);
          else
            mLeft = node;
        }
        else
        {
          if ( mRight )
            mRight->add(node,X_AXIS);
          else
            mRight = node;
        }
        break;
    }

  }

  float        getX(void) const { return mX;  }
  float        getY(void) const {  return mY; };
  float        getZ(void) const { return mZ; };
  float        getRadius(void) const { return mRadius; };
  unsigned int getIndex(void) const { return mIndex; };
  void *       getUserData(void) const { return mUserData; };

  void search(Axes axis,const float *pos,float radius,unsigned int &count,unsigned int maxObjects,KdTreeFindNode *found)
  {

    float dx = pos[0] - getX();
    float dy = pos[1] - getY();
    float dz = pos[2] - getZ();

    KdTreeNode *search1 = 0;
    KdTreeNode *search2 = 0;

    switch ( axis )
    {
      case X_AXIS:
       if ( dx <= 0 )     // JWR  if we are to the left
       {
        search1 = mLeft; // JWR  then search to the left
        if ( -dx < radius )  // JWR  if distance to the right is less than our search radius, continue on the right as well.
          search2 = mRight;
       }
       else
       {
         search1 = mRight; // JWR  ok, we go down the left tree
         if ( dx < radius ) // JWR  if the distance from the right is less than our search radius
	  			search2 = mLeft;
        }
        axis = Y_AXIS;
        break;
      case Y_AXIS:
        if ( dy <= 0 )
        {
          search1 = mLeft;
          if ( -dy < radius )
    				search2 = mRight;
        }
        else
        {
          search1 = mRight;
          if ( dy < radius )
    				search2 = mLeft;
        }
        axis = Z_AXIS;
        break;
      case Z_AXIS:
        if ( dz <= 0 )
        {
          search1 = mLeft;
          if ( -dz < radius )
    				search2 = mRight;
        }
        else
        {
          search1 = mRight;
          if ( dz < radius )
    				search2 = mLeft;
        }
        axis = X_AXIS;
        break;
    }

    float r2 = radius*radius;
    float m  = dx*dx+dy*dy+dz*dz;

    if ( m < r2 )
    {
      switch ( count )
      {
        case 0:
          found[count].mNode = this;
          found[count].mDistance = m;
          break;
        case 1:
          if ( m < found[0].mDistance )
          {
            if ( maxObjects == 1 )
            {
              found[0].mNode = this;
              found[0].mDistance = m;
            }
            else
            {
              found[1] = found[0];
              found[0].mNode = this;
              found[0].mDistance = m;
            }
          }
          else if ( maxObjects > 1)
          {
            found[1].mNode = this;
            found[1].mDistance = m;
          }
          break;
        default:
          if ( 1 )
          {
            bool inserted = false;

            for (unsigned int i=0; i<count; i++)
            {
              if ( m < found[i].mDistance ) // if this one is closer than a pre-existing one...
              {
                // insertion sort...
                unsigned int scan = count;
                if ( scan >= maxObjects ) scan=maxObjects-1;
                for (unsigned int j=scan; j>i; j--)
                {
                  found[j] = found[j-1];
                }
                found[i].mNode = this;
                found[i].mDistance = m;
                inserted = true;
                break;
              }
            }

            if ( !inserted && count < maxObjects )
            {
              found[count].mNode = this;
              found[count].mDistance = m;
            }
          }
          break;
      }
      count++;
      if ( count > maxObjects )
      {
        count = maxObjects;
      }
    }


    if ( search1 )
  		search1->search( axis, pos,radius, count, maxObjects, found);

    if ( search2 )
	  	search2->search( axis, pos,radius, count, maxObjects, found);

  }

  float distanceSquared(const float *pos) const
  {
    float dx = pos[0] - mX;
    float dy = pos[1] - mY;
    float dz = pos[2] - mZ;
    return dx*dx+dy*dy+dz*dz;
  }


private:

  void setLeft(KdTreeNode *left) { mLeft = left; };
  void setRight(KdTreeNode *right) { mRight = right; };

	KdTreeNode *getLeft(void)         { return mLeft; }
	KdTreeNode *getRight(void)        { return mRight; }

  unsigned int    mIndex;
  void           *mUserData;
  float           mX;
  float           mY;
  float           mZ;
  float           mRadius;
  KdTreeNode     *mLeft;
  KdTreeNode     *mRight;
};


class KdTree
{
public:
  KdTree(void)
  {
    mRoot = 0;
  }

  ~KdTree(void)
  {
    reset();
  }

  unsigned int search(const float *pos,float radius,unsigned int maxObjects,KdTreeFindNode *found) const
  {
    if ( !mRoot )	return 0;
    unsigned int count = 0;
    mRoot->search(X_AXIS,pos,radius,count,maxObjects,found);
    return count;
  }

  void reset(void)
  {
    mRoot = 0;
    for (int i=0; i<mObjects.Size(); i++)
    {
      KdTreeNode *node = mObjects[i];
      delete node;
    }
    mObjects.Clear();
  }

  unsigned int add(float x,float y,float z,float radius,void *userData)
  {
    unsigned int ret = mObjects.Size();
    KdTreeNode *node = new KdTreeNode( x, y, z, radius, userData, ret );
    mObjects.PushBack(node);
    if ( mRoot )
    {
      mRoot->add(node,X_AXIS);
    }
    else
    {
      mRoot = node;
    }
    return ret;
  }

  unsigned int getNearest(const float *pos,float radius,bool &_found) const // returns the nearest possible neighbor's index.
  {
    unsigned int ret = 0;

    _found = false;
    KdTreeFindNode found[1];
    unsigned int count = search(pos,radius,1,found);
    if ( count )
    {
      KdTreeNode *node = found[0].mNode;
      ret = node->getIndex();
      _found = true;
    }
    return ret;

  }

  KdTreeNode * getNode(int index)
  {
    KdTreeNode *ret = 0;
    if ( index < mObjects.Size() )
    {
      ret = mObjects[index];
    }
    return ret;
  }

private:
  KdTreeNode      *mRoot;
  KdTreeNodeVector mObjects;
};


#endif
