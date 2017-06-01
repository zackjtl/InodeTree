#include <stdlib.h>  
#include <stdio.h>
#include <assert.h>
#include <string>
#include <iostream>
#include "SelfGrowthTree.h"

using namespace std;

/*
 *  Here we suppose that one indirect block can only locate 4 addresses.
 *  Actually that is the block size divided by 4.
 */

int CSelfGrowthTree::MAX_TREE_SIZE = max_tree_size_default;

#define is_leaf(x) (x->get_depth() == 0)
#define is_head(x) (x->get_level() == 0)
#define is_null(x) (x == NULL)

/*
 *  The methods implementation of CSelfGrowthTree
 */     
CSelfGrowthTree::CSelfGrowthTree(int Index, int Level, int MaxNodes, int Depth, CSelfGrowthTree* Parent) 
  : _Index(Index),
    _Level(Level),
    _MaxNodes(MaxNodes),
    _Depth(Depth),
    _Parent(Parent) {
  _CurrLeaf = NULL;                
  
  dbg_printf("... Self-Organized Tree Constructed, Index = %d, Level = %d, Depth = %d, Parent = %07x\n",
                Index, Level, Depth, Parent);

}

CSelfGrowthTree::~CSelfGrowthTree() {
  int width = _SubTree.size();

  for (int i = 0; i < width; ++i) {
    delete _SubTree[i];
  }
}

bool CSelfGrowthTree::is_node_full() {
  if (_Depth == 0) {
    return _Data.size() >= _MaxNodes ? true : false;
  }            
  else {
    return _SubTree.size() >= _MaxNodes ? true : false;
  }
}

bool CSelfGrowthTree::is_node_empty() {
  if (_Depth == 0) {
    return _Data.size() == 0 ? true : false;
  }
  else {
    return _SubTree.size() == 0 ? true : false;
  }
}

bool CSelfGrowthTree::is_tree_full() {
  return IsTreeFull(this);
} 

int CSelfGrowthTree::get_level() {
  return _Level;
}

void CSelfGrowthTree::add_sub_tree(CSelfGrowthTree* SubTree) {
  _SubTree.push_back(SubTree);
}

CSelfGrowthTree* CSelfGrowthTree::get_parent() {
  return _Parent;
}

int CSelfGrowthTree::get_depth() {
  return _Depth;
}

CSelfGrowthTree* CSelfGrowthTree::get_curr_leaf() {
  return _CurrLeaf;
}

int CSelfGrowthTree::get_index() {
  return _Index;
}

int CSelfGrowthTree::get_width() {
  return _SubTree.size();
}

CSelfGrowthTree* CSelfGrowthTree::get_sub_tree(int ID) {
  return _SubTree[ID];         
}

vector<CSelfGrowthTree*>& CSelfGrowthTree::get_sub_tree_array() {
  return _SubTree;
}

vector<int>& CSelfGrowthTree::get_data() {
  return _Data;
}

void CSelfGrowthTree::push_data(int data) {
  _Data.push_back(data);
}

int CSelfGrowthTree::get_data_remain() {
  return _MaxNodes - _Data.size();
}

/*
 *  Returns the leaf created data count.
 */
int CSelfGrowthTree::new_leaves(int want_count) {
  int remain = want_count;  
  CSelfGrowthTree* ret;

  ret = NewLeaves(_CurrLeaf, remain);

  if (ret != NULL) {
    _CurrLeaf = ret;
  }
  ////dbg_printf("After NewLeaves operate, _CurrLeaf = %7x\n", _CurrLeaf);

  return (want_count - remain);
}

void CSelfGrowthTree::build_init_tree() {
  _CurrLeaf = NewSubTree(this);   
}

void CSelfGrowthTree::dump_tree() {
  DumpTree(this, 0);
}

/*
 *	The static function to check is a tree's node is full.
 */
bool CSelfGrowthTree::IsTreeFull(CSelfGrowthTree* Tree) 
{              
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

/*
 *	The static function to create leaves with amount of user specific.
 */
CSelfGrowthTree* CSelfGrowthTree::NewLeaves(CSelfGrowthTree* CurrPos, int& Count) 
{
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

/*
 *	The static function create a sub tree attached under a tree node.
 */    
CSelfGrowthTree* CSelfGrowthTree::NewSubTree(CSelfGrowthTree* Parent) 
{         
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

/*
 *	The static function to dump a tree (or sub-tree).
 */     
void CSelfGrowthTree::DumpTree(CSelfGrowthTree* Tree, int ID) 
{           
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
