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
  DQueue();
  ~DQueue();

  t getHead();
  t getTail();
  bool isEmpty();
  int count();
  void addHead(t val);
  void addTail(t val);

};

#endif
