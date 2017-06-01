#include <stdlib.h>  
#include <stdio.h>
#include <assert.h>
#include <string>
#include <iostream>
#include "InodeTree.h"

using namespace std;
/*
 *  The methods implementation of CInodeTree
*/
void CInodeTree::Init_DIND_Tree() 
{
   _DIND_Block = AllocIndex();              
  assert(_DIND_Block >= 0); 
  printf("---------------------------------\n");
  printf("Create Double Indirect Tree..\n");
  _DIND_Tree = new CSelfGrowthTree(_DIND_Block, 0, CSelfGrowthTree::MAX_TREE_SIZE, 1, NULL);
  _DIND_Tree->AllocIndex = AllocIndex;
  printf("Build DIND_Tree\n");
  _DIND_Tree->build_init_tree();
}

void CInodeTree::Init_TIND_Tree() 
{
  printf("---------------------------------\n");
  printf("Create Tripple Indirect Tree.. \n");
  _TIND_Block = AllocIndex();
  assert(_TIND_Block >= 0);

  _TIND_Tree = new CSelfGrowthTree(_TIND_Block, 0, CSelfGrowthTree::MAX_TREE_SIZE, 2, NULL);
  _TIND_Tree->AllocIndex = AllocIndex;
  printf("Build TIND_Tree\n");
  _TIND_Tree->build_init_tree();    
}

int CInodeTree::New_DIND_Addr(int Count) 
{
  dbg_printf("Create %d new address in the Double Indirect link table\n", Count);
  int created = _DIND_Tree->new_leaves(Count);
  return created;
}

int CInodeTree::New_TIND_Addr(int Count) 
{
  dbg_printf("Create %d new address in the Tripple Indirect link table\n", Count);
  int created = _TIND_Tree->new_leaves(Count);
  return created;
}
/*
 *  This function is for testing performance of full filling
 *  a two level tree with a limited size.
 */
int CInodeTree::Create_Full_Tree(int TreeLevel) 
{
  int counter = 0;    
  int want_per_loop = 200;//MAX_TREE_SIZE > 100 ? 100 : MAX_TREE_SIZE;
  int created = TreeLevel == 2 ? _DIND_Tree->new_leaves(want_per_loop) :
                                 _TIND_Tree->new_leaves(want_per_loop);    
  counter += created;

  while (created) {
    created = TreeLevel == 2 ? _DIND_Tree->new_leaves(want_per_loop) :
                               _TIND_Tree->new_leaves(want_per_loop);
    counter += created;
  }
  return counter;   
}

void CInodeTree::Dump_DIND_Tree() {
  _DIND_Tree->dump_tree();
}

void CInodeTree::Dump_TIND_Tree() {
  _TIND_Tree->dump_tree();
}

CSelfGrowthTree* CInodeTree::Get_DIND_Tree() {
  return _DIND_Tree;
}

CSelfGrowthTree* CInodeTree::Get_TIND_Tree() {
  return _TIND_Tree;    
}