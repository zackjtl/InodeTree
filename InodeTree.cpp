#include <stdlib.h>  
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
#include <ctime>
#include <iostream>

using namespace std;

/*
 *  Here we suppose that one indirect block can only locate 4 addresses.
 *  Actually that is the block size divided by 4.
 */

////#define MAX_TREE_SIZE 1024
const int max_tree_size_default = 32;
int MAX_TREE_SIZE = max_tree_size_default;

#define is_leaf(x) (x->get_depth() == 0)
#define is_head(x) (x->get_level() == 0)
#define is_null(x) (x == NULL)

//#define _TREE_DEBUG_ 

#ifdef _TREE_DEBUG_
#define dbg_printf(format, ...) printf(format "debug: ", __VA_ARGS__)
//#define dbg_printf printf
#else
#define dbg_printf(format, ...) 
#endif

typedef int (*alloc_index_fn)();

static int maxBlock = 0xFFFFFFF0;
static int currBlock = 1;

int AllocBlock() {
  if (currBlock >= 0xFFFFFFFE) {
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
    
    dbg_printf("... Self-Organized Tree Constructed, Index = %d, Level = %d, Depth = %d, Parent = %07x\n",
                  Index, Level, Depth, Parent);

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

  bool is_node_empty() {
    if (_Depth == 0) {
      return _Data.size() == 0 ? true : false;
    }
    else {
      return _SubTree.size() == 0 ? true : false;
    }
  }


  bool is_tree_full() {
    return IsTreeFull(this);
  } 

  int get_level() {
    return _Level;
  }

  void add_sub_tree(CSelfGrowthTree* SubTree) {
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

  void push_data(int data) {
    _Data.push_back(data);
  }

  int get_data_remain() {
    return _MaxNodes - _Data.size();
  }
 
  /*
   *  Returns the leaf created data count.
   */
  int new_leaves(int want_count) {
    int remain = want_count;  
    CSelfGrowthTree* ret;

    ret = NewLeaves(_CurrLeaf, remain);

    if (ret != NULL) {
      _CurrLeaf = ret;
    }
    ////dbg_printf("After NewLeaves operate, _CurrLeaf = %7x\n", _CurrLeaf);

    return (want_count - remain);
  }

  void build_init_tree() {
    _CurrLeaf = NewSubTree(this);   
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
      return full; 
    }
  }

  static CSelfGrowthTree* NewLeaves(CSelfGrowthTree* CurrPos, int& Count) {
    CSelfGrowthTree* parent;  
    CSelfGrowthTree* node = CurrPos;
  
    while (Count != 0) {
      if (!is_leaf(node) && node->is_node_empty()) {
        node = NewSubTree(node);
      }      
      parent = node->get_parent();

      if (is_leaf(node) && (!node->is_node_full())) {      
        int space = node->get_data_remain();
        vector<int>& data = node->get_data();
      
        while (space && Count) {
          int value = node->AllocIndex();        

          if (value > 0) {
            data.push_back(value);
            --space;
            --Count;
          }
          else {
            return NULL;
          }
        }
      }
      if (Count == 0) {
        return node; 
      }
      else if (!is_null(parent) && (!parent->is_node_full())) {    
        node = NewSubTree(parent);        
      }
      else {
        if (!is_null(parent) && (!is_head(parent))) {
          node = parent;
        }
        else {
          return NULL;
        }
      }           
    } // while end
  }
    
  static CSelfGrowthTree* NewSubTree(CSelfGrowthTree* Parent) {         
    /* The allocation function must handle number exceed */    
    int index = Parent->AllocIndex();   
    int depth = Parent->get_depth() - 1;    
    
    if (index < 0) {
      return NULL;
    }
    ////assert(index >= 0);
    if (Parent->get_depth() > 0) {                
      dbg_printf("Create sub tree with level %d, depth %d, parent = %07x\n", Parent->get_level() + 1, depth, Parent);      

      CSelfGrowthTree* subTree = new CSelfGrowthTree(index, Parent->get_level() + 1, MAX_TREE_SIZE, depth, Parent);       
      subTree->AllocIndex = Parent->AllocIndex;
      Parent->push_data(index);              
      
      dbg_printf("New sub tree's ptr = %07x\n", subTree);         
      Parent->add_sub_tree(subTree);

      return NewSubTree(subTree);
    }
    else if (Parent->get_depth() == 0) {
      dbg_printf("Leaf node created [%07x]\n", Parent);                  
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

  int New_DIND_Addr(int Count) {
    dbg_printf("Create %d new address in the Double Indirect link table\n", Count);
    int created = _DIND_Tree->new_leaves(Count);
    return created;
  }

 int New_TIND_Addr(int Count) {
    dbg_printf("Create %d new address in the Tripple Indirect link table\n", Count);
    int created = _TIND_Tree->new_leaves(Count);
    return created;
  }
  /*
   *  This function is for testing performance of full filling
   *  a two level tree with a limited size.
   */
  int Create_Full_Tree(int TreeLevel) {
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

enum eTreeLevel {TREE_DIND=2, TREE_TIND=3};
bool TestMode = false;
bool DumpTree = false;
int TreeLevel = TREE_DIND;
int BuildNumber = 0;
bool HelpMode = false;

bool PRS(int argc, char* argv[])
{
  for (int i = 1; i < argc; ) {
    char* arg = argv[i];

    if (arg[0] != '-') {
      perror("Invalid input argument");
      return false;
    }
    else if (arg[1] == 't') {
      TestMode = true;
    }
    else if (arg[1] == 'n') {
      BuildNumber = atoi(argv[i+1]);
      ++i;  
    }
    else if (arg[1] == 'l') {
      int lv = atoi(argv[i+1]);
      if (lv != 2 && lv != 3) {
        perror("Invalid tree level input");
        return false;
      }
      TreeLevel = lv;
      ++i;
    }
    else if (arg[1] == 's') {
      MAX_TREE_SIZE = atoi(argv[i+1]);
      ++i;
    }    
    else if (arg[1] == 'd') {
      DumpTree = true;
    }
    else if (arg[1] == 'h') {
      HelpMode = true;
    }
    ++i;
  }
  return true;
}

void disp_help()
{
  printf( "-t: Enable Test Mode (Full fill tree with limited tree size)\n"
          "-s: The tree size limitation. If not indicated, the default is %d\n"           
          "-n: For non-test mode, the number of leaf node to build.\n"
          "-l: The tree level. 2 or 3 is available for selecting DIND or TINT tree.\n"
          "    Default is DIND Tree.\n"
          "-h: Show this help document.\n"
          "-d: Dump the tree at final.\n", max_tree_size_default);
}

void disp_params()
{        
  printf("****************\n");
  printf("Parameters: \n");
  printf(TestMode ? "Test Mode\n" : "Normal Mode\n");
  printf(TreeLevel == TREE_DIND ? "Build DIND Tree\n" : "Build TIND Tree\n");
  printf(DumpTree ? "Dump Tree\n" : "No Dump Tree\n");
  printf("Tree Size Limit: %d\n", MAX_TREE_SIZE);                  
  if (!TestMode) {
    printf("Build Number Of Nodes: %d\n", BuildNumber);
  }  
  printf("****************\n");
}

void disp_elapsed(clock_t begin, clock_t end) 
{
  int total_ns = (((float)end - begin) / CLOCKS_PER_SEC) * 1000000;
  int ns = total_ns % 1000;
  int total_ms = total_ns / 1000;
  int ms = total_ms % 1000;
  int total_sec = total_ms / 1000;

  printf("Elapsed: %d (second) : %d (ms) : %d (ns) \n", total_sec, ms, ns);    
}

void disp_hel() {
  
}

int main(int argc, char* argv[]) 
{
  char select = 0;
  char want_dump = 0;
  
  if (!PRS(argc, argv)) {
    return -1;
  }

  if (HelpMode) {
    disp_help();
    return 0;
  }

  disp_params();
  
  CInodeTree tree;
  tree.AllocIndex = AllocBlock;
  if (TreeLevel == TREE_DIND) {
    tree.Init_DIND_Tree();
  }
  else {
    tree.Init_TIND_Tree();
  }
  int count = 0;
  clock_t begin, end;
  begin = clock();  

  if (TestMode) {
    count = tree.Create_Full_Tree(TreeLevel);
  }
  else {
    if (TreeLevel == TREE_DIND) {
      count = tree.New_DIND_Addr(BuildNumber);
    }
    else {
      count = tree.New_TIND_Addr(BuildNumber);      
    }          
  }
  end = clock();
  if ((!TestMode) && (count != BuildNumber)) {
    printf("** Warning: The tree size is insufficient to build wanted number of addresses**\n"
           " Remaining %d nodes had not built.\n", BuildNumber - count);
  }    
  if (DumpTree) {
    if (TreeLevel == TREE_DIND) {
      tree.Dump_DIND_Tree();    
    }
    else {
      tree.Dump_TIND_Tree(); 
    }
  }

  printf("Have built %d addresses into the tree\n", count);
  disp_elapsed(begin, end); 

  return 0;        
}
