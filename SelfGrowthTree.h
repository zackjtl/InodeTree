#ifndef SelfGrowthTreeH
#define SelfGrowthTreeH

#include <vector>
typedef int (*alloc_index_fn)();

//#define _TREE_DEBUG_

const int max_tree_size_default = 32;

#ifdef _TREE_DEBUG_
#define dbg_printf(format, ...) printf(format "debug: ", __VA_ARGS__)
//#define dbg_printf printf
#else
#define dbg_printf(format, ...) 
#endif

/*
 *  The tree node of the self organized tree
 */
class CSelfGrowthTree
{
public:        
  CSelfGrowthTree(int Index, int Level, int MaxNodes, int Depth, CSelfGrowthTree* Parent);
  ~CSelfGrowthTree();
  
  bool is_node_full();
  bool is_node_empty();

  bool is_tree_full();
  int get_level();
  void add_sub_tree(CSelfGrowthTree* SubTree);

  CSelfGrowthTree* get_parent();

  int get_depth();

  CSelfGrowthTree* get_curr_leaf();

  int get_index();

  int get_width();

  CSelfGrowthTree* get_sub_tree(int ID);

  std::vector<CSelfGrowthTree*>&  get_sub_tree_array();

  std::vector<int>& get_data();

  void push_data(int data);

  int get_data_remain();
  int new_leaves(int want_count);

  void build_init_tree();
  void dump_tree();   

public:
	/*
	 *	The static methods.
	 */	
  static bool IsTreeFull(CSelfGrowthTree* Tree);
  static CSelfGrowthTree* NewLeaves(CSelfGrowthTree* CurrPos, int& Count);    
  static CSelfGrowthTree* NewSubTree(CSelfGrowthTree* Parent); 
  static void DumpTree(CSelfGrowthTree* Tree, int ID);

public:
  /* The callback funciton used to allocate tree node index  */
  alloc_index_fn    AllocIndex;
  static int				MAX_TREE_SIZE;

private:  
  int                 _Index;
  CSelfGrowthTree*           _Parent;
  CSelfGrowthTree*           _CurrLeaf;  
  std::vector<CSelfGrowthTree*>   _SubTree;   
  std::vector<int>                   _Data;
  int                 _Level;
  int                 _Depth;
  int                 _MaxNodes;
};

#endif
