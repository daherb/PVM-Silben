#include <unordered_map>
#include <iostream>
#include <fstream>
#include <pvm3.h>
#include <mcheck.h>
#include "consts.h"

using namespace std;

unordered_map<string,int> frequency_list;
unordered_map<string,string> tree_list;

void read_frequency_list(string filename)
{
  ifstream in;
  in.open(filename.c_str());
  string word;
  int count;
  while (in >> word >> count)
    {
      frequency_list[word] = count;
    }
}
int main(int argc, char ** argv)
{
  ofstream log;
  log.open("/tmp/silben-master.log");
  int sbuf = pvm_initsend(PvmDataDefault);
  log << "read frequency list" << endl;
  read_frequency_list(WORD_FREQ_LIST);
  log << "finished reading" << endl;
  log << "spawning threads" << endl;
  int host_count;
  pvm_config(&host_count,NULL,NULL);
  int task_count=host_count*PER_HOST_TASK_COUNT;
  // Task-IDs
  int tids[task_count];
  int r=pvm_spawn((char *)"silben_slave",NULL,PvmTaskDefault,NULL,task_count,tids);
  if (r<task_count)
  {
    for(int ct=0;ct<task_count;ct++)
      printf("Spawn error %d\n",tids[ct]);
  }
  log << "finished spawning" << endl;
  unordered_map<string,int>::iterator it = frequency_list.begin();
  char *word = (char *) malloc(48);
  char *tree = (char *) malloc(1024*1024);
  log << "looping" << endl;
  while (it != frequency_list.end())
    {
      // See if DATA is pending
      if (pvm_probe(-1,DATA))
	{
	  int recbuf=pvm_recv(-1,DATA);
	  log << "got DATA " << endl;
	  int block_size;
	  pvm_upkint(&block_size,1,1);
	  log << "Recieving data block of size " << block_size << endl;
	  for (int ct = 0; ct < block_size; ct++)
	    {
	      pvm_upkstr(word);
	      pvm_upkstr(tree);
	      string sword = string(word);
	      string stree = string(tree);
	      log << "Unpacked " << sword << " \t" << sword.size() << " and " << stree << " \t" << stree.size() << endl;
	      tree_list[sword] = stree;
	    }
	}
      // Wait for a NEXT request
      log << "Waiting for NEXT" << endl;
      int recbuf=pvm_recv(-1,NEXT);
      log << "Received NEXT with " << recbuf << endl;
      int tid;
      pvm_bufinfo(recbuf,NULL,NULL,&tid);
      pvm_initsend(PvmDataDefault);
      int block_size = BLOCK_SIZE;
      if (frequency_list.size() < BLOCK_SIZE)
	block_size = frequency_list.size();
      log << "Sending data block of size " << block_size << endl;
      pvm_pkint(&block_size,1,1);
      for (int ct = 0; ct < block_size; ct++)
	{
	  log << "Pack " << it->first << endl;
	  pvm_pkstr((char *)it->first.c_str());
	  it++;
	}
      int sbuf = pvm_send(tid,DATA);
      log << "send DATA with " << sbuf << endl;
    }
  log << "Exiting" << endl;
  free(word);
  free(tree);
  log.close();
}
