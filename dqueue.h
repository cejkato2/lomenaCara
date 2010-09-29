#ifndef DQUEUE_H
#define DQUEUE_H
#include <vector>
#ifdef DEBUG
#include <iostream>
#endif

#define QUEUE_SORT_DIRECTION *n>*it

template <class t>
class DQueue {
public:
  struct Node {
    t data;
    Node *prev;
    Node *next;  
    Node(t val) {
      data = val;
      prev = NULL;
      next = NULL;
    }
    friend bool operator<(const Node &a, const Node &b) {
      return (a.data < b.data);
    }
    friend bool operator>(const Node &a, const Node &b) {
      return (a.data > b.data);
    }
  };
private:
  Node *first, *last;

  int size;

public:
  DQueue() {
    size = 0;
    first = NULL;
    last = NULL;
  }

  ~DQueue() {
    if (size>0) {
      Node *p = first;
      while (p!=NULL) {
        p = p->next;
        delete first;
        first = p;
      }
    }
  }

  t getHead() {
    return first->data;
  }

  t getTail() {
    return last->data;
  }

  Node *getHeadNode() {
    return first;
  }

  Node *getTailNode() {
    return last;
  }

  void insertSorted(t val) {
    Node *n = new Node(val);
    if (size==0) {
      //insert first val
      first = n;
      last = n;
      size++;
      return;
    }

    Node *it = first;
    while (it != NULL) {
      // eg. QUEUE_SORT_DIRECTION *n<*it
      if (QUEUE_SORT_DIRECTION) { 
        // insert new value before 'it'
        n->prev = it->prev;
        n->next = it;
        it->prev = n;
        if (it==first) {
          first = n;
        }
        size++;
        break;
      }
      if (it==last) {
        //append 'n'
        n->next = NULL;
        n->prev = it;
        it->next = n;
        last = n;
        size++;
        break;
      }

      it = it->next;
    }
  }

  void addHead(t val) {
    Node *n = new Node(val);
    if (size>0) {
      first->prev = n;
    } else {
      last = n;
    }
    n->next = first;
    first = n;
    size++;
  }

  void addTail(t val) {
    Node *n = new Node(val);
    if (size>0) {
      last->next = n;
    } else {
      first = n;
    }
    n->prev = last;
    last = n;
    size++;
  }

  void popHead() {
    Node *p = first;

    if (size>0) {
      if (last==first) {
        last = NULL;
      } else if (first->next==last) {
        last->prev = NULL;
      }

      first = first->next;
      if (first!=NULL)
        first->prev = NULL;
    }

    delete p;
    size--;
  }

  void popTail() {
    Node *p = last;

    if (size>0) {
      if (last==first) {
        first = NULL;
      } else if (last->prev==first) {
        first->next = NULL;
      }

      last = last->prev;
      if (last!=NULL)
        last->next = NULL;
    }

    delete p;
    size--;
  }

  bool isEmpty() {
    return (size == 0);
  }

  int count() {
    return size;
  }

  DQueue* copy() const {
    Node *p = first;
    DQueue<t> *nq = new DQueue<t>();

    while (p!=NULL) {
      nq->addTail(p->data);
      p = p->next;
    }
    return nq;
  }

  friend std::ostream &operator<<( std::ostream &o, const DQueue<t> *q ) {
    DQueue::Node *p = q->first;
    o << "Content of queue:" << std::endl;
    while (p!=NULL) {
      o << p->data << " ";
      p = p->next;
    }
    o << std::endl;
    return o;
  }


  
  std::vector<t> getData() {
    std::vector<t> v;
    Node *p = first;

    while (p!=NULL) {
      v.push_back(p->data);
      p = p->next;
    }
    return v;
  }
  
};

#endif
