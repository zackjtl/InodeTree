#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
using namespace std;
#define MAX_TREE_LEVEL 3
#define MAX_DIN_TREE_SIZE  4
#define MAX_TIN_TREE_SIZE 4

#define LEAF  1
#define NON_LEAF 0

typedef int (*alloc_index_fn)();

static int maxBlock = 1024;
static int currBlock = 1;

int AllocBlock() {
  if (currBlock < maxBlock) {
    return currBlock++;
  }
  else {
    throw "Exceed block number to allocation";
  }
}

/*
 *  The tree node of the self organized tree
 */
class CSubTree
{
public:        
  CSubTree(int Index, int Level, int MaxNodes, int Depth, CSubTree* Parent) 
    : _Index(Index),
      _Level(Level),
      _MaxNodes(MaxNodes),
      _Depth(Depth),
      _Parent(Parent) {
    _CurrLeaf = NULL;                
    printf("... SubTree Constructed, Index = %d, Level = %d, Depth = %d, Parent = %07x\n",
                  Index, Level, Depth, Parent);

  }
  #if 1
  ~CSubTree() {
    int width = _SubTree.size();

    for (int i = 0; i < width; ++i) {
      delete _SubTree[i];
    }
  }
#endif

  bool IsFull() {
    if (_SubTree.size() >= _MaxNodes) {
      return true;
    }
    return false;
  }

  int GetLevel() {
    return _Level;
  }

  void PushBack(CSubTree* SubTree) {
    _SubTree.push_back(SubTree);
  }

  void InitialData() {
    _Data.resize(_MaxNodes, 0);
  }

  int GetLeafSize() {
    return _Data.size();
  }

  vector<int>& GetLeafData() {
    return _Data;
  }
  
  CSubTree* GetParent() {
    return _Parent;
  }

  int GetDepth() {
    return _Depth;
  }

  CSubTree* GetCurrLeaf() {
    return _CurrLeaf;
  }

  int GetIndex() {
    return _Index;
  }

  int GetWidth() {
    return _SubTree.size();
  }

  CSubTree* GetSubTree(int ID) {
    return _SubTree[ID];         
  }
 
  bool NewLeaf(CSubTree* CurrPos) {    
    CSubTree* parent = CurrPos->GetParent();      

    if (!parent->IsFull()) {
      printf("Curr leaf [%07x]'s parent [%07x] is not full, create new sub tree in depth %d\n", CurrPos,  parent, parent->GetDepth()-1);
      _CurrLeaf = NewSubTree(parent, parent->GetDepth() - 1);
      printf("Created new leaf [%07x] done\n", _CurrLeaf);
      return true;
    }
    else {
      if (parent->GetLevel() != 0)  {
        printf("Curr leaf's parent [%07x] is full, call parent's parent to create new leaf\n", parent);
        return NewLeaf(parent);
      }
      else {
        printf("All full, can't add any leaf\n");
        return false;
      }
    }
  }
#if 1 
  CSubTree* NewSubTree(CSubTree* Parent, int Depth) {     
    
    /* The allocation function must handle number exceed */    
    int index = AllocIndex();   
    assert(index >= 0);

    printf("Create sub tree with level %d, depth %d, parrent = %07x\n", Parent->GetLevel() + 1, Depth, Parent);
    CSubTree* subTree = new CSubTree(index, Parent->GetLevel() + 1, _MaxNodes, Depth, Parent);       
    printf("New sub tree's ptr = %07x\n", subTree);
    Parent->PushBack(subTree);
    
    /* The leaf */
    if (Depth == 0) {
      printf("Leaf node created [%07x], parent is [%07x]\n", subTree, Parent);            
      ////_Data.resize(_MaxNodes);
      _CurrLeaf = subTree;
      return _CurrLeaf;
    }
    else {
      return NewSubTree(subTree, Depth - 1);
    }
  }
  #endif
 
  void DumpTree(CSubTree* SubTree, int ID) { 
    string spaces(SubTree->GetLevel() * 16, ' ');

    if (ID == 0) {
      printf("[%07x] %03d", SubTree,  SubTree->GetIndex());
    } 
    else {
      printf("%s[%07x] %03d", spaces.c_str(), SubTree, SubTree->GetIndex());
    }
    if (SubTree->GetDepth() == 0) {
      printf("\n");
    }
    else {
      printf(" ->");
    }

    for (int i = 0; i < SubTree->GetWidth(); ++i) {
      CSubTree* sub_subTree = SubTree->GetSubTree(i);
      if (sub_subTree != NULL) {            
        DumpTree(sub_subTree, i);
      }      
    }
    if (SubTree->GetDepth() == 1) {
      printf("\n");
    }
  
  }

public:
  /* The callback funciton used to allocate tree node index  */
  alloc_index_fn    AllocIndex;

private:  
  int                 _Index;
  vector<int>         _Data;
  CSubTree*           _Parent;
  CSubTree*           _CurrLeaf;  
  vector<CSubTree*>   _SubTree; 
  int                 _Level;
  int                 _Depth;
  int                 _MaxNodes;
};
/*
 *  A tree that can organize by itself which size is 
 *  constrainted.
 */
class CSelfOrganizedTree
{
public:
 
  CSelfOrganizedTree() {
    
  }  
 
  void Initial() {
    _DINBlock = AllocIndex();              
    assert(_DINBlock >= 0);
 
    printf("Create Double Indirect Tree..\n");
    _DINTree = new CSubTree(_DINBlock, 0, MAX_DIN_TREE_SIZE, 1, NULL);
#if 1    
    _DINTree->AllocIndex = AllocBlock;
    printf("Build DINTree\n");
    _DINTree->NewSubTree(_DINTree, 0);
    printf("---------------------------------\n");
    printf("Create Tripple Indirect Tree.. \n");
    _TINBlock = AllocIndex();
    assert(_TINBlock >= 0);

    _TINTree = new CSubTree(_TINBlock, 0, MAX_TIN_TREE_SIZE, 2, NULL);
    _TINTree->AllocIndex = AllocBlock;
    printf("Build TINTree\n");
    _TINTree->NewSubTree(_TINTree, 1);


    _TINTree->DumpTree(_TINTree, 0);
    printf("---------------------------\n");

    for (int i = 0; i < 10; ++i) {                  
      CSubTree* currLeaf = _TINTree->GetCurrLeaf();
      bool status = _TINTree->NewLeaf(currLeaf);    

      if (status) {
        printf("Create new leaf success\n");      
      }
      else {
        printf("Create new leaf failed!!\n");
      }
      _TINTree->DumpTree(_TINTree, 0);
      printf("---------------------------\n");
    }
    
    _TINTree->DumpTree(_TINTree, 0); 
    printf("\n");


#endif
  }


public:
  alloc_index_fn    AllocIndex;

private:
  int        _DINBlock;
  int        _TINBlock;

  CSubTree*  _DINTree;
  CSubTree*  _TINTree;
};

int main(int argc, char* argv[]) 
{
  CSelfOrganizedTree tree;
  tree.AllocIndex = AllocBlock;
  tree.Initial();
 

  return 0;        
}
