#include <iostream>
#include "dqueue.h"
#include <vector>
#include "types.h"

void print(std::vector<Point *> *v) {
  std::vector<Point *>::iterator it;
  for (it=v->begin(); it!=v->end(); ++it) {
    std::cout << " " << *it;
  }
  std::cout << std::endl;
}

void remove_item(std::vector<Point *> *s, const Point *val) {
  std::vector<Point *>::iterator it;
  for (it=s->begin(); it!=s->end(); ++it) {
    if (*it==val) {
      s->erase(it);
      break;
    }
  }
}

void permutate(std::vector<Point *> *s, std::vector<Point *> *f) {
  std::vector<Point *>::iterator it;
  if (s->empty()==false) {
    for (it=s->begin(); it!=s->end(); ++it) {
      std::vector<Point *> temp = *s;
      f->push_back(*it);
      remove_item(&temp, *it);
      if (temp.empty()==true) {
        print(f);
      } else {
        permutate(&temp, f);
      }
      remove_item(f, *it);
    }
  }
}


int main(int argc, char **argv) {
  std::vector<Point *> set_of_num, permut;
  set_of_num.push_back(new Point(1,1));
  set_of_num.push_back(new Point(2,1));
  set_of_num.push_back(new Point(3,1));
  set_of_num.push_back(new Point(1,4));

  
  std::vector<Point *> temp = set_of_num;
  permutate(&temp, &permut);

  std::cout << "Original set: " << std::endl;
  print(&set_of_num);

  std::vector<Point *>::iterator it;
  for (it=set_of_num.begin(); it!=set_of_num.end(); ++it) {
    delete *it;
  }

  return 0;

}

