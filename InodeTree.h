#ifndef InodeTreeH
#define InodeTreeH

#include "SelfGrowthTree.h"

/*
 *  A tree that can organize by itself which size is 
 *  constrainted.
 */
class CInodeTree
{
public:
 
  CInodeTree() {    
  }  

  void Init_DIND_Tree();
  void Init_TIND_Tree();

  int New_DIND_Addr(int Count);
	int New_TIND_Addr(int Count);
  /*
   *  This function is for testing performance of full filling
   *  a two level tree with a limited size.
   */
  int Create_Full_Tree(int TreeLevel);

  void Dump_DIND_Tree();
  void Dump_TIND_Tree();

  CSelfGrowthTree* Get_DIND_Tree();
  CSelfGrowthTree* Get_TIND_Tree(); 

public:
  alloc_index_fn    AllocIndex;

private:
  int        _DIND_Block;
  int        _TIND_Block;

  CSelfGrowthTree*  _DIND_Tree;
  CSelfGrowthTree*  _TIND_Tree;
};

#endif