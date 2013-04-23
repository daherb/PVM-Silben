#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <pvm3.h>
#include <string.h>
#include "consts.h"

using namespace std;

template<class T> 
class Node {
public:
  Node();
  ~Node();
  vector<Node<T> *> children;
  T value;
};
  
template<class T> 
class Tree {
public:
  Tree();
  ~Tree();
  Node<T> *root;
  void print();
  void print(string pre, string post, string npre, string npost, Node<T> *node);
  string serialize();
  string serialize(Node<T> *node);
private:
  //  void clean_up(Node<T> *n);
};
    
class Matcher {
public:
  Matcher();
  ~Matcher();
  void read_syllables(string filename);
  string match(string word);
private:
  Tree<string> syllable_tree;
  void get_prefixes(string word, vector<string> *matches, Node<string> *current = NULL);
  Tree<string> *build_part_tree(string word);
  void prune(Node<string> *n);
};

template<class T>
Node<T>::Node()
{
  //  children = new vector<Node<T> *>();
}

template<class T>
Node<T>::~Node()
{
  //  delete children;
}

template<class T>
Tree<T>::Tree()
 {
   root = NULL;
 }

template<class T>
Tree<T>::~Tree()
 {
   delete(root);
 }

template<class T>
void Tree<T>::print()
{
  if (root != NULL)
    {
      print("", "", "  ", "", root);
    }
}

template<class T>
void Tree<T>::print(string pre, string post, string npre, string npost, Node<T> *node)
{
  cout << pre << node->value << post << endl;
  for (typename vector<Node<T> *>::iterator it = node->children.begin() ; it != node->children.end(); ++it)
    {
      print(pre + npre, post + npost, npre, npost, *it);
    }
} 

template<class T>
string Tree<T>::serialize()
{
  if (root != NULL)
    {
      return serialize(root);
    }
}

template<class T>
string Tree<T>::serialize(Node<T> *node)
{
  string stree;
  stree = node->value;
  if (node->children.size() > 0)
    {
      for (typename vector<Node<T> *>::iterator it = node->children.begin() ; it < node->children.end(); ++it)
	{
	  stree += " " + serialize(*it);
	}
      stree = "(" + stree + ")";
    }
  return stree;
}

//template<class T>
//void Tree<T>::clean_up(Node<T> *n)
//{
  // if (n != NULL)
  //   {
  //     if (n->children != NULL)
  // 	{
  // 	  while (n->children->size()>0)
  // 	    {
  // 	      Node<T> *last = n->children->back();
  // 	      clean_up(last);
  // 	      n->children->pop_back();
  // 	    }
  // 	  delete n->children;
  // 	  n->children = NULL;
  // 	}
  //     delete n;
  //     n = NULL;
  //   }
//}

Matcher::Matcher()
{
  
}

Matcher::~Matcher()
{
}
void Matcher::read_syllables(string filename)
{
  ifstream in;
  const char *fn = filename.c_str();
  in.open(fn);
  string syllable;
  syllable_tree.root = new Node<string>();
  syllable_tree.root->value = "";
  Node<string> *current;
  while(in >> syllable)
    {
      current = syllable_tree.root;
      bool found = false;
      for (int sylpos = 0; sylpos < syllable.length(); sylpos++)
	{
	  found = false;
	  for (typename vector<Node<string> *>::iterator it = current->children.begin() ; it != current->children.end(); ++it)
	    {
	      if ((*it)->value == syllable.substr(sylpos,1))
		{
		  found = true;
		  current = *it;
		  break;
		}
	    } 
	  if (!found)
	    {
	      Node<string> *n = new Node<string>();
	      n->value = syllable.substr(sylpos,1);
	      if (n != NULL)
		current->children.push_back(n);
	      else
		cerr << "FeuerFeuerFeuer 175" << endl;
	      current = n;
	      if (sylpos == syllable.length()-1)
		{
		  // Add terminator
		  n = new Node<string>();
		  n->value = "$";
		  if (n != NULL)
		    current->children.push_back(n);
		  else
		    cerr << "FeuerFeuerFeuer 185" << endl;
		  current = n;
		  // Add whole syllable
		  n = new Node<string>();
		  n->value = syllable;
		  if (n != NULL)
		    current->children.push_back(n);
		  else
		    cerr << "FeuerFeuerFeuer 193" << endl;
		  current = n;
		}
	    }
	}
    }
  in.close();
  //  syllable_tree.print();
  //  cout << syllable_tree.serialize() << endl;
}

string Matcher::match(string word)
{
  Tree<string> *part = build_part_tree(word);
  part->print();
  prune(part->root);
  part->print();
  return part->serialize();
}

void Matcher::get_prefixes(string word, vector<string> *matches, Node<string> *current)
{
  if (current == NULL) 
    current = syllable_tree.root;
  for (vector<Node<string> *>::iterator it = current->children.begin(); it != current->children.end(); it++)
    {
      if ((*it)->value == word.substr(0,1))
	{
	  get_prefixes(word.substr(1),matches,*it);
	}
      else if ((*it)->value == "$")
	{
	  matches->push_back((*it)->children.at(0)->value);
	  //	  cout << "Prefix: " << (*it)->children->at(0)->value << endl;
	}
    }
}

Tree<string> * Matcher::build_part_tree(string word)
{
  //  cout << "call " << word << endl;
  Tree<string> *parts = new Tree<string>();
  parts->root=new Node<string>();
  parts->root->value="";
  if (word == "")
    {
      //      cout << "if" << endl;
      Node<string> *n = new Node<string>();
      n->value = "$";
      if (n != NULL)
	parts->root->children.push_back(n);
      else
	cerr << "FeuerFeuerFeuer 242" << endl;
      //      cout << parts->serialize() << endl;
    }
  else
    {
      //      cout << "else" << endl;
      vector<string> *prefixes = new vector<string>();
      //      cout << "Prefixes of " << word << endl;
      get_prefixes(word,prefixes);
      //      cout << "size " <<prefixes->size() << endl;
      for (vector<string>::iterator it = prefixes->begin(); it != prefixes->end() ; it++)
	{
	  //	  cout << "for " << endl;
	  //	  cout << "  " << *it << endl;
	  Node<string> *n = new Node<string>();
	  n->value = *it;
	  if (n != NULL)
	    parts->root->children.push_back(n);
	  else
	    cerr << "FeuerFeuerFeuer 261" << endl;
	  Tree<string> *sub = build_part_tree(word.substr(it->length()));
	  //	  delete(sub);
	  //	  n->children = new vector<Node<string> *>(*sub->root->children);
	  n->children = sub->root->children;
	  delete sub;
	}
      delete prefixes;
    }
  return parts;
}

void Matcher::prune(Node<string> *n)
{
  for (typename vector<Node<string> *>::iterator it = n->children.begin(); it != n->children.end(); it++)
    {
      prune(*it);
    }
  bool erased = true;
  while(erased)
    {
      erased = false;
      for (typename vector<Node<string> *>::iterator it = n->children.begin(); it != n->children.end(); it++)
	{
	  
	  if ((*it)->children.size()==0 && (*it)->value != "$"){
	    //	    cerr << "Erase " << (*it)->value << endl;
	    n->children.erase(it);
	    erased = true;
	  }
	}
    }
}

int main(int argc, char ** argv)
{
  int master=pvm_parent();
  int tid=pvm_mytid();
  ofstream log;
  //  const char *logname=string("/tmp/silben-slave" + std::to_string(tid) + ".log").c_str();
  const char *logname="/tmp/silben-slave.log"; //string("/tmp/silben-slave.log").c_str();
  log.open(logname);
  Matcher m;
  char *word = (char*) malloc(48);
  string tree;
  log << "reading syllable list" << endl;
  m.read_syllables(SYLLABLE_LIST);
  log << "finished reading" << endl;
  while(!pvm_probe(master,DIE))
    {
      pvm_initsend(PvmDataDefault);
      int sbuf = pvm_send(master, NEXT);
      log << "send NEXT with " << sbuf << endl;
      // Get words
      int recbuf=pvm_recv(-1, DATA);
      log << "received DATA with " << recbuf << endl;
      pvm_initsend(PvmDataDefault);
      int block_size;
      pvm_upkint(&block_size,1,1);
      log << "Recieving data block of size " << block_size << endl;
      pvm_pkint(&block_size,1,1);
      for (int ct=0; ct < block_size; ct++)
  	{
  	  log << "Trying to unpack" << endl;
  	  // Unpack word
  	  pvm_upkstr(word);
  	  log << "Unpacked " << word << endl;
  	  // Split word
  	  log << "Trying to match" << endl;
  	  tree = m.match(string(word));
  	  log << "Matched " << word << " to " << tree << endl;
  	  log << word << "\t" << tree << endl;
  	  // Pack tree
  	  pvm_pkstr(word);
  	  pvm_pkstr((char *)tree.c_str());
  	  // Stop on die-message
  	  if (pvm_probe(master,DIE))
  	    break;
  	}
      // Send tree
      sbuf=pvm_send(master,DATA);
      log << "send DATA with " << sbuf << endl;
    }
  pvm_recv(master, DIE);
  log << "received DIE" << endl;
  pvm_exit();
  // word = "nicht";
  // tree = m.match(string(word));
  // cout << "Matched " << word << " to " << tree << endl;
  log.close();
  //  free(word);
  return 0;
}
