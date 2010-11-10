#ifndef TYPES_H
#define TYPES_H
#include <ostream>
#define SUCCESS         0
#define MIN_AMOUNT_ERR  1
#define LESS_AMOUNT_ERR 2

  struct Point {
    int x;
    int y;
    Point() {}

    Point(int a, int b) {
      x = a;
      y = b;
    }

    Point(const Point &o) {
      x = o.x;
      y = o.y;
    }



    friend std::ostream &operator<<(std::ostream &o, const Point *p) {
      o << "[ " << p->x << ", " << p->y << " ]";
      return o;
    }

    friend std::ostream &operator<<(std::ostream &o, const Point &p) {
      o << "[ " << p.x << ", " << p.y << " ]";
      return o;
    }

   friend bool operator== (const Point &a, const Point &b)
{
       if(a.x == b.x && a.y == b.y)
           return true;
       else return false;
}


  };


#endif

