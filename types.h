#ifndef TYPES_H
#define TYPES_H
#include <iostream>
#define SUCCESS         0
#define MAX_AMOUNT_ERR  1
#define LESS_AMOUNT_ERR 2

  struct Point {
    int x;
    int y;
    Point() {}
    Point(Point &o) {
      x = o.x;
      y = o.y;
    }

    friend std::ostream &operator<<(std::ostream &o, const Point *p) {
      o << "[ " << p->x << ", " << p->y << " ]";
      return o;
    }
  };


#endif

