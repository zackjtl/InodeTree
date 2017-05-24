#include <stdlib.h>  
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
using namespace std;
#define MAX_TREE_LEVEL 3

/*
 *  Here we suppose that one indirect block can only locate 4 addresses.
 *  Actually that is the block size divided by 4.
 */

////#define MAX_TREE_SIZE 1024
int MAX_TREE_SIZE = 32;

#define LEAF  1
#define NON_LEAF 0

typedef int (*alloc_index_fn)();

static int maxBlock = 0xFFFFFFF0;
static int currBlock = 1;

int AllocBlock() {
  if (currBlock >= 18400000) {
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
class CSelfGrowthTree
{
public:        
  CSelfGrowthTree(int Index, int Level, int MaxNodes, int Depth, CSelfGrowthTree* Parent) 
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
  ~CSelfGrowthTree() {
    int width = _SubTree.size();

    for (int i = 0; i < width; ++i) {
      delete _SubTree[i];
    }
  }
#endif

  bool is_node_full() {
    if (_Depth == 0) {
      return _Data.size() >= _MaxNodes ? true : false;
    }            
    else {
      return _SubTree.size() >= _MaxNodes ? true : false;
    }
  }

  bool is_tree_full() {
    return IsTreeFull(this);
  }
  
  int get_level() {
    return _Level;
  }

  void push_back(CSelfGrowthTree* SubTree) {
    _SubTree.push_back(SubTree);
  }

  CSelfGrowthTree* get_parent() {
    return _Parent;
  }

  int get_depth() {
    return _Depth;
  }

  CSelfGrowthTree* get_curr_leaf() {
    return _CurrLeaf;
  }

  int get_index() {
    return _Index;
  }

  int get_width() {
    return _SubTree.size();
  }

  CSelfGrowthTree* get_sub_tree(int ID) {
    return _SubTree[ID];         
  }

  vector<CSelfGrowthTree*>&  get_sub_tree_array() {
    return _SubTree;
  }

  vector<int>& get_data() {
    return _Data;
  }
 
  bool new_leaf() {
    //if (_CurrLeaf == NULL) {
    //  throw "Need to build initial the tree first.";   
    //}   
    CSelfGrowthTree* tmp = NewLeaf(_CurrLeaf);

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
  static bool IsTreeFull(CSelfGrowthTree* Tree) {              
    if (Tree->get_depth() == 0) {
      return Tree->is_node_full();
    }
    else if (Tree->get_depth() > 0){ 
      if (Tree->is_node_full() == false) {
        return false;
      }
      int cnt = Tree->get_width();
      bool full = true;
#if 1
      vector<CSelfGrowthTree*>& subTree = Tree->get_sub_tree_array();
      vector<CSelfGrowthTree*>::iterator it = subTree.begin();
      vector<CSelfGrowthTree*>::iterator end = subTree.end();
      
      while (it != end) {
        if (IsTreeFull(*it) == false) {
          full = false;
          break;
        }              
        ++it;
      }      
#else
      for (int i = 0; i < cnt; ++i) {
        if (IsTreeFull(Tree->get_sub_tree(i)) == false) {
          full = false;
          break;
        }
      }
#endif     
      return full; 
    }
  }

  static CSelfGrowthTree* NewLeaf(CSelfGrowthTree* CurrPos) {    
    CSelfGrowthTree* parent = CurrPos->get_parent();      

    if (CurrPos->get_depth() == 0 && (!CurrPos->is_node_full())) {
#ifdef _TREE_DEBUG__            
      printf("Curr leaf [%07x] is not full, insert new data entry (depth %d)\n", CurrPos, CurrPos->get_depth());
#endif
      int value = CurrPos->AllocIndex();
      
      if (value > 0) {
        vector<int>& data = CurrPos->get_data();
        data.push_back(value);
        return CurrPos;
      }
      else {
        return NULL;
      }
    }
    else if (!parent->is_node_full()) {    
#ifdef _TREE_DEBUG_            
      printf("Curr node [%07x]'s parent [%07x] is not full, create new sub tree (depth %d)\n", CurrPos, parent, parent->get_depth() - 1);
#endif      
      CSelfGrowthTree* newLeaf = NewSubTree(parent, parent->get_depth() - 1);
#ifdef _TREE_DEBUG_      
      printf("Created new leaf [%07x] done\n", newLeaf);
#endif      
      return newLeaf;
    }
    else {
      if (parent->get_level() != 0)  {
#ifdef _TREE_DEBUG_
        printf("Curr node [%07x]'s parent [%07x] is full, call parent's parent to create new leaf\n", parent);
#endif
        return NewLeaf(parent);
      }
      else {
        throw("All full, can't add any leaf");
        return NULL;
      }
    }
  }
  
  static CSelfGrowthTree* NewSubTree(CSelfGrowthTree* Parent, int Depth) {         
    /* The allocation function must handle number exceed */    
    int index = Parent->AllocIndex();   
    
    if (index < 0) {
      return NULL;
    }
    ////assert(index >= 0);
    if (Parent->get_depth() > 0) {    
#ifdef _TREE_DEBUG_
      printf("Create sub tree with level %d, depth %d, parent = %07x\n", Parent->get_level() + 1, Depth, Parent);
#endif 
      CSelfGrowthTree* subTree = new CSelfGrowthTree(index, Parent->get_level() + 1, MAX_TREE_SIZE, Depth, Parent);       
      subTree->AllocIndex = Parent->AllocIndex;
#ifdef _TREE_DEBUG_    
      printf("New sub tree's ptr = %07x\n", subTree);   
#endif    
      Parent->push_back(subTree);

      return NewSubTree(subTree, Depth - 1);
    }
    else if (Parent->get_depth() == 0) {
      vector<int>& data = Parent->get_data();
      data.push_back(index); 
 #ifdef _TREE_DEBUG_                
      printf("Leaf node created [%07x], parent is [%07x]\n", subTree, Parent);                  
#endif            
      return Parent;
    }
  }
 
  static void DumpTree(CSelfGrowthTree* Tree, int ID) {           
    string spaces(Tree->get_level() * 17, ' ');

    if (ID == 0) {
      printf("[%07x] %04d ->", Tree,  Tree->get_index());
    } 
    else {
      printf("%s[%07x] %04d ->", spaces.c_str(), Tree, Tree->get_index());
    }

    if (Tree->get_depth() == 0) {
      string spaces2((Tree->get_level() + 1) * 17, ' ');
      
      vector<int>& data = Tree->get_data();
      int dataCnt = data.size();      
     
      for (int i = 0; i < dataCnt; ++i) {
        if (i == 0) {
          printf(" [%04x] %04d\n", i, data[i]);
        }
        else {
          printf("%s [%04x] %04d\n", spaces2.c_str(), i, data[i]);
        }
      }         
      printf("\n");
    }
    else {   
      vector<CSelfGrowthTree*>& subTree = Tree->get_sub_tree_array();
      vector<CSelfGrowthTree*>::iterator it = subTree.begin();
      vector<CSelfGrowthTree*>::iterator end = subTree.end();

      int index = 0;
      while (it != end) {
        if ((*it) != NULL) {
          DumpTree(*it, index);
        }              
        ++index;
        ++it;
      }           
    }
  }

public:
  /* The callback funciton used to allocate tree node index  */
  alloc_index_fn    AllocIndex;

private:  
  int                 _Index;
  CSelfGrowthTree*           _Parent;
  CSelfGrowthTree*           _CurrLeaf;  
  vector<CSelfGrowthTree*>   _SubTree;   
  vector<int>                   _Data;
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
    _DIND_Tree = new CSelfGrowthTree(_DIND_Block, 0, MAX_TREE_SIZE, 1, NULL);
    _DIND_Tree->AllocIndex = AllocIndex;
    printf("Build DIND_Tree\n");
    _DIND_Tree->build_init_tree();
  }

  void Init_TIND_Tree() {
    printf("---------------------------------\n");
    printf("Create Tripple Indirect Tree.. \n");
    _TIND_Block = AllocIndex();
    assert(_TIND_Block >= 0);

    _TIND_Tree = new CSelfGrowthTree(_TIND_Block, 0, MAX_TREE_SIZE, 2, NULL);
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
  int Create_DIND_Full_Addr() {
    int counter = 0;         
    while (!_DIND_Tree->is_tree_full()) {
      if (_DIND_Tree->new_leaf() == false) {
        break;
      }
      //_DIND_Tree->dump_tree();            
      ++counter;
    }
    return counter;
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

  CSelfGrowthTree* Get_DIND_Tree() {
    return _DIND_Tree;
  }

  CSelfGrowthTree* Get_TIND_Tree() {
    return _TIND_Tree;    
  }
 


public:
  alloc_index_fn    AllocIndex;

private:
  int        _DIND_Block;
  int        _TIND_Block;

  CSelfGrowthTree*  _DIND_Tree;
  CSelfGrowthTree*  _TIND_Tree;
};

int main(int argc, char* argv[]) 
{
  char select = 0;
  char want_dump = 0;
  
  CInodeTree tree;
  
  if (argc >= 2) {
    MAX_TREE_SIZE = atoi(argv[1]);
  }

#if 1
  while (1) {              
    cout << "Which depth tree to create ? (2/3) or others to break. \n";
    cin >> select;
    cout << "Want to dump result ? (y/n)\n";
    cin >> want_dump;
    cout << endl;
  
    if (select != '2'  && select != '3') {
      break;
    }
    clock_t begin, end;

    if (select == '2') {
      tree.AllocIndex = AllocBlock;     
      tree.Init_DIND_Tree();      

      begin = clock();   
      int count = tree.Create_DIND_Full_Addr();
      end = clock();
      printf("---------------------------------\n");
      printf("%d addresses were added into the DIND tree.\n", count);      

      if (want_dump == 'y' || want_dump == 'Y') {
        tree.Dump_DIND_Tree(); 
      }       
    }
    else if (select == '3') {
      tree.AllocIndex = AllocBlock;     
      tree.Init_TIND_Tree();      

      begin = clock();   
      int count = tree.Create_TIND_Full_Addr();
      end = clock();
      printf("---------------------------------\n");
      printf("%d addresses were added into the TIND tree.\n", count);      

      if (want_dump == 'y' || want_dump == 'Y') {
        tree.Dump_TIND_Tree();
      }       
            
    }
    int total_ns = (((float)end - begin) / CLOCKS_PER_SEC) * 1000000;
    int ns = total_ns % 1000;
    int total_ms = total_ns / 1000;
    int ms = total_ms % 1000;
    int total_sec = total_ms / 1000;

    printf("Elapsed: %d (second) : %d (ms) : %d (ns) \n", total_sec, ms, ns);   
    break;
  }       
 #else
  tree.AllocIndex = AllocBlock;
  tree.Init_TIND_Tree();
  clock_t start = clock();
  int count = tree.Create_TIND_Full_Addr();
  clock_t end = clock();
 
  tree.Dump_TIND_Tree();          

  printf("%d addresses were added into the TIND Tree.\n", count);
  printf("---------------------------------\n");

  int total_ns = (((float)end - start) / CLOCKS_PER_SEC) * 1000000;
  int ns = total_ns % 1000;
  int total_ms = total_ns / 1000;
  int ms = total_ms % 1000;
  int total_sec = total_ms / 1000;

  printf("Elapsed: %d (second) : %d (ms) : %d (ns) \n", total_sec, ms, ns);
#endif
  return 0;        
}
