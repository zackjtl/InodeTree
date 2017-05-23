#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
using namespace std;
#define MAX_TREE_LEVEL 3

/*
 *  Here we suppose that one indirect block can only locate 4 addresses.
 *  Actually that is the block size divided by 4.
 */

#define MAX_TREE_SIZE 1024

#define LEAF  1
#define NON_LEAF 0

typedef int (*alloc_index_fn)();

static int maxBlock = 0xFFFFFFF0;
static int currBlock = 1;

int AllocBlock() {
  if (currBlock >= 9200000) {
    ////throw "Exceed block number to allocation";
    return -1;
  }
  else {
    return currBlock++;
  }
}

/*
 *  The tree node of the self organized tree
 */
class CSelfOrganizedTree
{
public:        
  CSelfOrganizedTree(int Index, int Level, int MaxNodes, int Depth, CSelfOrganizedTree* Parent) 
    : _Index(Index),
      _Level(Level),
      _MaxNodes(MaxNodes),
      _Depth(Depth),
      _Parent(Parent) {
    _CurrLeaf = NULL;                
#ifdef _TREE_DEBUG_
    printf("... Self-Organized Tree Constructed, Index = %d, Level = %d, Depth = %d, Parent = %07x\n",
                  Index, Level, Depth, Parent);
#endif    

  }
  #if 1
  ~CSelfOrganizedTree() {
    int width = _SubTree.size();

    for (int i = 0; i < width; ++i) {
      delete _SubTree[i];
    }
  }
#endif

  bool is_node_full() {
    if (_SubTree.size() >= _MaxNodes) {
      return true;
    }
    return false;
  }

  bool is_tree_full() {
    return IsTreeFull(this);
  }

  int get_level() {
    return _Level;
  }

  void push_back(CSelfOrganizedTree* SubTree) {
    _SubTree.push_back(SubTree);
  }

  CSelfOrganizedTree* get_parent() {
    return _Parent;
  }

  int get_depth() {
    return _Depth;
  }

  CSelfOrganizedTree* get_curr_leaf() {
    return _CurrLeaf;
  }

  int get_index() {
    return _Index;
  }

  int get_width() {
    return _SubTree.size();
  }

  CSelfOrganizedTree* get_sub_tree(int ID) {
    return _SubTree[ID];         
  }
 
  bool new_leaf() {
    //if (_CurrLeaf == NULL) {
    //  throw "Need to build initial the tree first.";   
    //}   
    CSelfOrganizedTree* tmp = NewLeaf(_CurrLeaf);

    if (tmp == NULL) {
      return false;
    }
    else {
      _CurrLeaf = tmp;
      return true;
    }    
  }

  void build_init_tree() {
    _CurrLeaf = NewSubTree(this, _Depth - 1);
  }

  void dump_tree() {
    DumpTree(this, 0);
  }


public:
  static bool IsTreeFull(CSelfOrganizedTree* Tree) {              
    if (Tree->get_depth() == 1) {
      return Tree->is_node_full();
    }
    else if (Tree->get_depth() > 1){ 
      if (Tree->is_node_full() == false) {
        return false;
      }
      int cnt = Tree->get_width();
      bool full = true;

      for (int i = 0; i < cnt; ++i) {
        if (IsTreeFull(Tree->get_sub_tree(i)) == false) {
          full = false;
          break;
        }
      }
      return full;
    }
  }

  static CSelfOrganizedTree* NewLeaf(CSelfOrganizedTree* CurrPos) {    
    CSelfOrganizedTree* parent = CurrPos->get_parent();      

    if (!parent->is_node_full()) {
#ifdef _TREE_DEBUG__            
      printf("Curr leaf [%07x]'s parent [%07x] is not full, create new sub tree in depth %d\n", CurrPos,  parent, parent->get_depth()-1);
#endif
      CSelfOrganizedTree* newLeaf = NewSubTree(parent, parent->get_depth() - 1);
#ifdef _TREE_DEBUG_      
      printf("Created new leaf [%07x] done\n", newLeaf);
#endif      
      return newLeaf;
    }
    else {
      if (parent->get_level() != 0)  {
#ifdef _TREE_DEBUG_
        printf("Curr leaf's parent [%07x] is full, call parent's parent to create new leaf\n", parent);
#endif
        return NewLeaf(parent);
      }
      else {
        throw("All full, can't add any leaf");
        return NULL;
      }
    }
  }
  
  static CSelfOrganizedTree* NewSubTree(CSelfOrganizedTree* Parent, int Depth) {         
    /* The allocation function must handle number exceed */    
    int index = Parent->AllocIndex();   
    
    if (index < 0) {
      return NULL;
    }
    ////assert(index >= 0);
#ifdef _TREE_DEBUG_
    printf("Create sub tree with level %d, depth %d, parrent = %07x\n", Parent->get_level() + 1, Depth, Parent);
#endif    
    CSelfOrganizedTree* subTree = new CSelfOrganizedTree(index, Parent->get_level() + 1, MAX_TREE_SIZE, Depth, Parent);       
    subTree->AllocIndex = Parent->AllocIndex;
#ifdef _TREE_DEBUG_    
    printf("New sub tree's ptr = %07x\n", subTree);   
#endif    
    Parent->push_back(subTree);
    
    /* The leaf */
    if (Depth == 0) {
#ifdef _TREE_DEBUG_                
      printf("Leaf node created [%07x], parent is [%07x]\n", subTree, Parent);            
#endif      
      return subTree;
    }
    else {
      return NewSubTree(subTree, Depth - 1);
    }
  }
 
  static void DumpTree(CSelfOrganizedTree* SubTree, int ID) { 
    string spaces(SubTree->get_level() * 16, ' ');

    if (ID == 0) {
      printf("[%07x] %03d", SubTree,  SubTree->get_index());
    } 
    else {
      printf("%s[%07x] %03d", spaces.c_str(), SubTree, SubTree->get_index());
    }
    if (SubTree->get_depth() == 0) {
      printf("\n");
    }
    else {
      printf(" ->");
    }

    for (int i = 0; i < SubTree->get_width(); ++i) {
      CSelfOrganizedTree* sub_subTree = SubTree->get_sub_tree(i);
      if (sub_subTree != NULL) {            
        DumpTree(sub_subTree, i);
      }      
    }
    if (SubTree->get_depth() == 1) {
      printf("\n");
    }
  
  }

public:
  /* The callback funciton used to allocate tree node index  */
  alloc_index_fn    AllocIndex;

private:  
  int                 _Index;
  CSelfOrganizedTree*           _Parent;
  CSelfOrganizedTree*           _CurrLeaf;  
  vector<CSelfOrganizedTree*>   _SubTree;   
  int                 _Level;
  int                 _Depth;
  int                 _MaxNodes;
};
/*
 *  A tree that can organize by itself which size is 
 *  constrainted.
 */
class CInodeTree
{
public:
 
  CInodeTree() {
    
  }  

  void Init_DIND_Tree() {
     _DIND_Block = AllocIndex();              
    assert(_DIND_Block >= 0); 
    printf("---------------------------------\n");
    printf("Create Double Indirect Tree..\n");
    _DIND_Tree = new CSelfOrganizedTree(_DIND_Block, 0, MAX_TREE_SIZE, 2, NULL);
    _DIND_Tree->AllocIndex = AllocIndex;
    printf("Build DIND_Tree\n");
    _DIND_Tree->build_init_tree();
  }

  void Init_TIND_Tree() {
    printf("---------------------------------\n");
    printf("Create Tripple Indirect Tree.. \n");
    _TIND_Block = AllocIndex();
    assert(_TIND_Block >= 0);

    _TIND_Tree = new CSelfOrganizedTree(_TIND_Block, 0, MAX_TREE_SIZE, 3, NULL);
    _TIND_Tree->AllocIndex = AllocIndex;
    printf("Build TIND_Tree\n");
    _TIND_Tree->build_init_tree();    
  }

  void New_DIND_Addr(int Count) {
#ifdef _TREE_DEBUG_          
    printf("Create %d new address in the Double Indirect link table\n", Count);
#endif
    while (Count) {
      _DIND_Tree->new_leaf();                 
      --Count;
    } 
  }

 void New_TIND_Addr(int Count) {
#ifdef _TREE_DEBUG_         
    printf("Create %d new address in the Tripple Indirect link table\n", Count);
#endif
    while (Count) {
      _TIND_Tree->new_leaf();   
      ////_TIND_Tree->dump_tree();          
      --Count;
    } 
  }

 int Create_TIND_Full_Addr() {
  int counter = 0;         
  while (!_TIND_Tree->is_tree_full()) {
    if (_TIND_Tree->new_leaf() == false) {
      break;
    }
    //_TIND_Tree->dump_tree();            
    ++counter;
  }
  return counter;
 }

  void Dump_DIND_Tree() {
    _DIND_Tree->dump_tree();
  }

  void Dump_TIND_Tree() {
    _TIND_Tree->dump_tree();
  }

  CSelfOrganizedTree* Get_DIND_Tree() {
    return _DIND_Tree;
  }

  CSelfOrganizedTree* Get_TIND_Tree() {
    return _TIND_Tree;    
  }
 


public:
  alloc_index_fn    AllocIndex;

private:
  int        _DIND_Block;
  int        _TIND_Block;

  CSelfOrganizedTree*  _DIND_Tree;
  CSelfOrganizedTree*  _TIND_Tree;
};

int main(int argc, char* argv[]) 
{
  CInodeTree tree;
  tree.AllocIndex = AllocBlock;

  tree.Init_DIND_Tree();   
  printf("---------------------------------\n"); 
  try {
    tree.New_DIND_Addr(3);
 }
  catch (...) {
    printf("!!Too mush entries to add into DIND tree!!\n");
  }
  printf("---------------------------------\n");
  tree.Dump_DIND_Tree();
  
  printf("=================================\n");
  tree.Init_TIND_Tree();

  int count = tree.Create_TIND_Full_Addr();
  printf("%d addresses were added into the TIND tree.\n", count);
  printf("---------------------------------\n");
  ////tree.Dump_TIND_Tree();


  return 0;        
}
