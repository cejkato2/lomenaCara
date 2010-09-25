#ifndef DQUEUE_H
#define DQUEUE_H

template <class t>
class DQueue {
private:
  class Node {
    t data;
    Node *prev;
    Node *next;  
  } *first, *last;

  int size;

public:
  DQueue() {
    size = 0;
    first = NULL;
    last = NULL;
  }

  ~DQueue() {
    if (size>0) {
      //Node *p;
      /*TODO delete nodes*/
    }
  }

  t getHead() {
    return first;
  }

  t getTail() {
    return last;
  }

  void addHead(t val) {
    size++;
  }

  void addTail(t val) {
    size++;
  }

  void popHead() {
    size--;
  }

  void popTail() {
    size--;
  }

  bool isEmpty() {
    return (size == 0);
  }

  int count() {
    return size;
  }

};

#endif
