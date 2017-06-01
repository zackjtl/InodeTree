#include <stdlib.h>  
#include <stdio.h>
#include <string>
#include <ctime>
#include <iostream>

using namespace std;

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
