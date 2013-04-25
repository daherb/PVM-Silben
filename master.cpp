#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <pvm3.h>
#include <ctime>
#include "consts.h"

using namespace std;

vector<string> word_list;
unordered_map<string,int> frequency_list;
unordered_map<string,string> tree_list;

void read_word_frequency_list(string filename)
{
  ifstream in;
  in.open(filename.c_str());
  string word;
  int count;
  while (in >> word >> count)
    {
      word_list.push_back(word);
      frequency_list[word] = count;
    }
}
int main(int argc, char ** argv)
{
  ofstream log;
  log.open("/tmp/silben-master.log");
  int sbuf = pvm_initsend(PvmDataDefault);
  log << "read frequency list" << endl;
  read_word_frequency_list(WORD_FREQ_LIST);
  log << "finished reading" << endl;
  log << "spawning threads" << endl;
  int host_count;
  pvm_config(&host_count,NULL,NULL);
  int task_count=host_count*PER_HOST_TASK_COUNT;
  // Task-IDs
  int tids[task_count];
  char **args = NULL;
  int r=pvm_spawn((char *)"silben_slave",args,PvmTaskDefault /*PvmTaskDebug*/,NULL,task_count,tids);
  if (r<task_count)
  {
    for(int ct=0;ct<task_count;ct++)
      printf("Spawn error %d\n",tids[ct]);
  }
  log << "finished spawning" << endl;
  time_t begin_time, end_time;
  time(&begin_time);
  char *word = (char *) malloc(48);
  char *tree = (char *) malloc(1024*1024);
  //  log << "looping" << endl;
  while (word_list.size())
    {
      // See if DATA is pending
      while (pvm_probe(-1,DATA))
	{
	  int recbuf=pvm_recv(-1,DATA);
	  log << "got DATA with " << recbuf << endl;
	  int block_size;
	  pvm_upkint(&block_size,1,1);
	  log << "Recieving data block of size " << block_size << endl;
	  for (int ct = 0; ct < block_size; ct++)
	    {
	      pvm_upkstr(word);
	      pvm_upkstr(tree);
	      string sword = string(word);
	      string stree = string(tree);
	      //	      log << "Unpacked " << sword << " \t" << sword.size() << " and " << stree << " \t" << stree.size() << endl;
	      tree_list[sword] = stree;
	    }
	}
      // Wait for a NEXT request
      //      log << "Waiting for NEXT" << endl;
      int recbuf=pvm_recv(-1,NEXT);
      log << "Received NEXT with " << recbuf << endl;
      int tid;
      pvm_bufinfo(recbuf,NULL,NULL,&tid);
      pvm_initsend(PvmDataDefault);
      int block_size = BLOCK_SIZE;
      if (word_list.size() < BLOCK_SIZE)
	block_size = word_list.size();
      log << "Sending data block of size " << block_size << endl;
      pvm_pkint(&block_size,1,1);
      for (int ct = 0; ct < block_size; ct++)
	{
	  string sword = word_list.back();
	  //	  log << "Pack " << sword << endl;
	  pvm_pkstr((char *) sword.c_str() );
	  word_list.pop_back();
	}
      int sbuf = pvm_send(tid,DATA);
      log << "send DATA with " << sbuf << endl;
    }
  // See if DATA is pending
  while (pvm_probe(-1,DATA))
    {
      int recbuf=pvm_recv(-1,DATA);
      log << "got DATA with " << recbuf << endl;
      int block_size;
      pvm_upkint(&block_size,1,1);
      log << "Recieving data block of size " << block_size << endl;
      for (int ct = 0; ct < block_size; ct++)
	{
	  pvm_upkstr(word);
	  pvm_upkstr(tree);
	  string sword = string(word);
	  string stree = string(tree);
	  //	  log << "Unpacked " << sword << " \t" << sword.size() << " and " << stree << " \t" << stree.size() << endl;
	  tree_list[sword] = stree;
	}
    }

  // Syncing up after end of data
  for (int ct = 0; ct < task_count; ct++)
    {
      // Sending empty data twice to be sure to be received and unblock receive
      pvm_initsend(PvmDataDefault);
      int null = 0;
      pvm_pkint(&null,1,1);
      pvm_send(tids[ct],DATA);
      pvm_initsend(PvmDataDefault);
      pvm_send(tids[ct],DATA);
      log << "Send empty DATA to " << tids[ct] << endl;
      // Waiting for last message
      int recbuf=pvm_recv(tids[ct],FINAL);
      log << "got FINAL with " << recbuf << endl;
      int block_size;
      pvm_upkint(&block_size,1,1);
      log << "Recieving data block of size " << block_size << endl;
      for (int ct2 = 0; ct2 < block_size; ct2++)
  	{
  	  pvm_upkstr(word);
  	  pvm_upkstr(tree);
  	  string sword = string(word);
  	  string stree = string(tree);
  	  log << "Unpacked " << sword << " \t" << sword.size() << " and " << stree << " \t" << stree.size() << endl;
  	  tree_list[sword] = stree;
  	}
      int tid;
      pvm_bufinfo(recbuf,NULL,NULL,&tid);
      pvm_initsend(PvmDataDefault);
      int sbuf = pvm_send(tids[ct],DIE);
      log << "send DIE with " << sbuf << endl;
    }
  time(&end_time);
  log << "Matching took " << difftime(end_time,begin_time) << " seconds" << endl;
  log << "Start writing output" << endl;
  ofstream ofile;
  int wordcount = 0;
  ofile.open(OUTPUT_FILE);
  for (unordered_map<string,int>::iterator it = frequency_list.begin(); it != frequency_list.end(); it++)
    {
      //      log << it->first << "\t" << tree_list[it->first] << "\t" << it->second << endl;
      ofile << it->first << "\t" << tree_list[it->first] << "\t" << it->second << endl;
    }
  ofile.close();
  log << "Finished writing output" << endl;
  log.close();
  free(word);
  free(tree);
  log << "Exiting" << endl;
  pvm_exit();
}
