// $Id: file_sys.cpp,v 1.7 2019-07-09 14:05:44-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <iomanip>
using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {0};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
  DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");

   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   inode_ptr temp = root;
   root = root->contents->mkdir("/");
   temp = nullptr;
   cwd = root;
}
inode_state::~inode_state(){
  root->contents->rmr();
}

const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   this_type = type;
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}
inode::~inode(){

  contents = nullptr;
}
int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}
directory::~directory(){
  dirents.clear();
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
   throw file_error ("is a " + error_file_type());
}

void base_file::writefile (const wordvec&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::remove (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&) {
   throw file_error ("is a " + error_file_type());
   
}

inode_ptr base_file::mkfile (const string&) {
   throw file_error ("is a " + error_file_type());
}


size_t plain_file::size() const {
   size_t size {0};
   auto i = data.begin();

   while(i != data.end()){
      size += i->length();
      ++i;
   }
   if(size > 0)
     size += data.size() -1 ;
   return size;
}

const wordvec& plain_file::readfile() const {
   
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   wordvec newFile;
   newFile = words;
   while(newFile.size() >2){
      data.push_back(newFile.back());
      newFile.pop_back();
   }
}

size_t directory::size() const {
   size_t size {0};
   size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::remove (const string& filename) { 
   dirents.erase(filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   if(contains(dirname)){
      cout << "directory already exists" << endl;
      return nullptr;
   }
   
   inode_ptr n = make_shared<inode>(file_type::DIRECTORY_TYPE);
   add_entry(dirname,n);
   n->contents->add_entry(".",n);
   if(dirname.compare("/") == 0)
      n->contents->add_entry("..",n);
   else
      n->contents->add_entry("..",dirents["."]);

   DEBUGF ('i', dirname);
   n->contents->changeName(dirname);
   return n;
}

inode_ptr directory::mkfile (const string& filename) {
   inode_ptr n = make_shared<inode>(file_type::PLAIN_TYPE);
   add_entry(filename,n);
   return n;
}
void directory::add_entry(const string& key, inode_ptr value) {
   dirents[key] = value;
}

void plain_file::add_entry(const string&, inode_ptr) {

}

void directory::changeName(const string name){
   this->currName= name;
}

void plain_file::changeName(const string name){
   this->currName = name;
}

void base_file::changeName(const string name){
   this->currName = name;
}

string inode_state::getName(){
    return cwd->contents->getName();
}
string inode_state::getDir(){
   string output ="";
   inode_ptr s = cwd;
   if (root == cwd)
      return root->contents->getName();
   while(root != cwd){
      output = "/" + cwd->contents->getName() + output;
      cwd = cwd->contents->get("..");
   }
   cwd = s;
   return output;
}
void inode_state::mkdir(const string& str){
   cwd->contents->mkdir(str);
}
void inode_state::mkfile(const wordvec& words){
   inode_ptr newFile = cwd->contents->mkfile(words[1]);
   newFile->contents->writefile(words);

}
void inode_state::cd(const string& str){
   inode_ptr temp = root->contents->find(str);
   if(cwd->contents->contains(str) &&
      cwd->contents->get(str)->this_type == file_type::DIRECTORY_TYPE)
      cwd = cwd->contents->get(str);
   else if(temp != nullptr && temp->this_type 
           == file_type::DIRECTORY_TYPE)
      cwd = temp;
   else
      cout << " no directory found" << endl;
  
}
bool directory::contains(const string& name){
   try{
      dirents.at(name);
      return true;
    }
   catch(const std::out_of_range& oor){
      return false;
   }
}
inode_ptr directory::get(const string& name){
   return dirents[name];
}

bool base_file::contains(const string&){

}
inode_ptr base_file::get(const string&){
}
void inode_state::ls(const string& str){
    inode_ptr temp = cwd;
    if (str.compare(".") ==0){
      cout <<getDir()<< ":" <<endl ;
      cwd->contents->ls();
    }else if (str.compare("..") == 0){
      cwd = cwd->contents->get("..");
      cout <<getDir()<< ":" <<endl ;
      cwd->contents->ls(); 
    }else{
       inode_ptr p = root->contents->find(str);
       if(p == nullptr){ 
          cout << str << " Does not exit" << endl;
          return;
       }  
       cwd = p;
       cout <<getDir()<< ":" <<endl ;
       cwd->contents->ls();
    }
    cwd = temp;
}
void inode_state::lsr(const string& str){
    inode_ptr curr;
    curr = root;
    curr->contents->lsr(cwd->contents->getName());     
}
void directory::lsr(const string& end){
   if(end.length() >0 )
      printDir();
   vector<base_file_ptr> v;
   auto itor = dirents.begin();
    while(itor != dirents.end() ){ 
       cout << setw(8)<< dirents[itor->first]->get_inode_nr()
            << setw(8)<< dirents[itor->first]->contents->size() 
            << "  " << itor->first;
       if(dirents[itor->first]->this_type == file_type::DIRECTORY_TYPE
          && itor->first.compare("..") != 0
          && itor->first.compare(".") != 0){
          cout<< "/"<<endl;
          v.push_back(dirents[itor->first]->contents);
       }
       else
          cout<< endl;
      ++itor;  
    }
    if(currName.compare(end) ==0)
       return; 
    while (!v.empty()){ 
      v.back()->lsr(end);
      v.pop_back();
    }    
}
void directory::printDir(){
  int count = 0;
   string dir;
   inode_ptr temp = dirents["."];
   while(temp->contents->getName().compare("/") !=0 ){
    count ++;
     dir = "/" + temp->contents->getName() + dir;
     temp = temp->contents->get("..");
     if (count >20)
        break;
   }
   if(count == 0)
      cout << "/";
   cout << dir <<":" <<endl; ;
}
void directory::ls(){
    auto itor = dirents.begin();
    while(itor != dirents.end() ){ 
       cout << setw(8)<< dirents[itor->first]->get_inode_nr()
            << setw(8)<< dirents[itor->first]->contents->size() 
            << "  " << itor->first;
       if(dirents[itor->first]->this_type == file_type::DIRECTORY_TYPE
          && itor->first.compare("..") != 0
          && itor->first.compare(".") != 0)
          cout<< "/"<<endl;
       else
          cout<< endl;
      ++itor;  
    }
}
void base_file::ls(){
}
void inode_state::readfile(const string& str){
   cwd->contents->readfile(str);
}
const void directory::readfile(const string& name){
   if(!contains(name)){
      cout << "cat: " <<name << ": No such file or directory" << endl;
      return;
   }
   wordvec output = dirents[name]->contents->readfile();
   auto i = output.rbegin();
   while(i != output.rend()){
      cout << *i << " ";
      ++i;
   }
   cout << endl;
}
const void base_file::readfile(const string&){
}
inode_ptr directory::find(const string& str){ // call on root
   if(str.compare("/") ==0)
     return dirents[".."];
   if(contains(str))   
     return dirents[str];
   else{
     auto itor = dirents.begin();
     ++itor;
     ++itor; 
     while(itor != dirents.end()){
        if(dirents[itor->first]->this_type == file_type::DIRECTORY_TYPE)
           return dirents[itor->first]->contents->find(str);
      ++itor;
     }
      return nullptr;
    }
      
}
inode_ptr base_file::find(const string&){
}
void base_file::lsr(const string&){
}

void inode_state::rm(const string& s){
   if(cwd->contents->contains(s)){
      cwd->contents->remove(s);   
   }
   else
      cout << "rm: cannot remove '"<< s <<
       "': No such file or directory"<< endl; 
}

void inode_state::rmr(const string& s){
   inode_ptr temp = root->contents->find(s);
   inode_ptr temp2 = temp->contents->get("..");
   temp->contents->rmr();
   temp2->contents->remove(s);   
}

void directory::rmr(){
   auto i = dirents.begin();
   ++i;
   ++i;
   while(i != dirents.end()){
      if(dirents[i->first]->this_type == file_type::DIRECTORY_TYPE)
         dirents[i->first]->contents->rmr();
     ++i;  
   }
      dirents.clear();
}
void base_file::rmr(){
}
