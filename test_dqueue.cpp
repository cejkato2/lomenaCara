#include <iostream>
#include "dqueue.h"
#include <vector>

int main(int argc, char **argv) {
  DQueue<int> *t = new DQueue<int>();
  t->addHead(1);
  t->addTail(2);
  t->addHead(0);
  t->addTail(3);

  std::cout << "test1" << std::endl;
  std::vector<int>::iterator it;
  std::vector<int> v;
  v = t->getData();
  for (it=v.begin(); it!=v.end(); ++it) {
    std::cout << *it << std::endl;
  }

  t->popHead();
  t->popTail();

  std::cout << "test2" << std::endl;
  v = t->getData();
  for (it=v.begin(); it!=v.end(); ++it) {
    std::cout << *it << std::endl;
  }
  
  t->popTail();
  t->popTail();

  std::cout << "test3" << std::endl;
  v = t->getData();
  for (it=v.begin(); it!=v.end(); ++it) {
    std::cout << *it << std::endl;
  }

  t->addHead(1);
  t->addTail(2);

  DQueue<int> *t2;
  //t2 = new DQueue<int>(t);
  t2 = t->copy();
  std::cout << "t2" << std::endl;
  v = t2->getData();
  for (it=v.begin(); it!=v.end(); ++it) {
    std::cout << *it << std::endl;
  }
  v = t->getData();
  for (it=v.begin(); it!=v.end(); ++it) {
    std::cout << *it << std::endl;
  }
  t->popHead();
  t->popHead();

  std::cout << "test4" << std::endl;
  v = t->getData();
  for (it=v.begin(); it!=v.end(); ++it) {
    std::cout << *it << std::endl;
  }
  v = t2->getData();
  for (it=v.begin(); it!=v.end(); ++it) {
    std::cout << *it << std::endl;
  }

  delete t;
  delete t2;

  std::cout << "ordering test" << std::endl;
  DQueue<int> *oq = new DQueue<int>();
  oq->insertSorted(5);
  oq->insertSorted(1);
  oq->insertSorted(6);
  v = oq->getData();
  for (it=v.begin(); it!=v.end(); ++it) {
    std::cout << *it << std::endl;
  }
  return(0);
}


