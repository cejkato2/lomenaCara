#ifndef TYPES_H
#define TYPES_H
#include <iostream>

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

