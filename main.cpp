#include <stdio.h>
#include <iostream>
#include <fstream>
#include "types.h"
#include "dqueue.h"
#include "search.h"
#include <vector>

#define MIN_AMOUNT_POINTS 3

std::vector<Point *> points; // all readed points

int read_points(FILE *f, DQueue<Point *> *q) {
    int amount;
    fscanf(f, "%i", &amount);
    if (amount < MIN_AMOUNT_POINTS) {
        fclose(f);
        std::cerr << "Amount of points has to be greater or equal " <<         \
      MIN_AMOUNT_POINTS << std::endl;
        return MIN_AMOUNT_ERR;
    }
    for (int i = 0; i < amount; ++i) {
        if (feof(f) != 0) {
            std::cerr << "Amount of inserted points is lesser than expected" <<        \
        std::endl;
            return LESS_AMOUNT_ERR;
        }
        Point *p = new Point();
        fscanf(f, "%i %i", &p->x, &p->y);
        q->addHead(p);
    }

    fclose(f);
    return SUCCESS;
}

void print_result(FILE *f, DQueue<Point *> *q) {

    DQueue<Point *>::Node *p = q->getHeadNode();
    while (p != NULL) {
        fprintf(f, "%i %i\n", p->data->x, p->data->y);
        p = p->next;
    }
}

/**
 * are three different points a,b,c on a line ?
 * @param a first point
 * @param b second point
 * @param c third point
 * @return true if they are on a line, else false
 */

inline bool isElementLine(const Point *a, const Point *b, const Point *c) {

    if (*a == *c || *b == *c)
        return false;

    int det = (a->x * b->y + a->y * c->x + b->x * c->y) - (b->y * c->x + c->y * a->x + a->y * b->x);

    if (det == 0)
        return true;
    else return false;
}

/**
 * is the point c on the segment between points a and b ?
 * @param a first point
 * @param b second point
 * @param c middle point
 * @return
 */
inline bool isOnSegment(const Point *a, const Point *b, const Point *c) {

    if (!isElementLine(a, b, c)) // if it is not on the line, then it cannot be on the segment
        return false;

    if ((a->x < c->x) && (c->x < b->x))
        return true;

    if ((b->x < c->x) && (c->x < a->x))
        return true;

    if ((a->y < c->y) && (c->y < b->y))
        return true;

    if ((b->y < c->y) && (c->y < a->y))
        return true;


    return false;
}

void print(std::vector<Point *> *v) {
  std::vector<Point *>::iterator it;
  for (it=v->begin(); it!=v->end(); ++it) {
    std::cout << " " << *it;
  }
  std::cout << std::endl;
}

/**
 * counts a count of breaks on the line defined by sequence of points defined by vector l
 * @param l sequence of points defining [broken] line
 * @return count of breaks
 */
int countBreaks(std::vector<Point *> *l) {

    if(l->size() < 3)
        return 0;

  int amount = 0;
  Point *a,*b,*c;

  for (std::vector<Point *>::iterator it=l->begin()+2; it!=l->end(); ++it) {

      a=*(it-2); // first point
      b=*it; // second point
      c=*(it-1); // the middle one

   // std::cout << "counting isOnSegment(" << a << ", "<< b << ", " << c << ")" << std::endl;
    if (!isOnSegment( a, b, c ))
        amount++;

  }
  return amount;    
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

void permutate(std::vector<Point *> *s, std::vector<Point *> *f, std::vector<Point *> *br_line) {
  std::vector<Point *>::iterator it;
  if (s->empty()==false) {
    for (it=s->begin(); it!=s->end(); ++it) {
      std::vector<Point *> temp = *s;
      f->push_back(*it);
      remove_item(&temp, *it);
      if (temp.empty()==true) {
        if (br_line->empty() == true) {
          //init state
          *br_line = *f;
        } else {
          int old_size = countBreaks(br_line);
          int new_size = countBreaks(f);
          std::cout << old_size << " / " << new_size << " ";
          if (new_size<old_size) {
            *br_line = *f;
          }
        }
        //DEBUG
        print(f);
      } else {
        permutate(&temp, f, br_line);
      }
      remove_item(f, *it);
    }
  }
}

int main(int argc, char **argv) {

    int return_val = SUCCESS;

    FILE *input_file = stdin;
    if (argc > 1) {
        input_file = fopen(argv[1], "r");
        if (input_file == NULL) {
            input_file = stdin;
        }
    }

    DQueue<Point *> *input, *backup;

    /*init of input queue*/
    input = new DQueue<Point *>();

    /*get points from stdin*/
    return_val = read_points(input_file, input);
    if (return_val == SUCCESS) {
        std::cerr << input;
        /* backup points */
        backup = input->copy();
        //
    } else {
        delete input;
        return return_val;
    }



    points = input->getData();
    std::vector<Point *>::iterator it;
    std::ofstream output_stream;
    output_stream.open("vystup1.dat");
    for (it = points.begin(); it != points.end(); ++it) {
        output_stream << (*it)->x << " " << (*it)->y << std::endl;
    }
    output_stream.close();


    std::vector<Point *> temp_results;
    std::vector<Point *> min_line;
    
    permutate(&points, &temp_results, &min_line);
    std::cout << "min_line - size: " << countBreaks(&min_line) << std::endl;
    print(&min_line);


    output_stream.open("vystup2.dat");
    for (it=min_line.begin(); it!=min_line.end(); ++it) {
        output_stream << (*it)->x << " " << (*it)->y << std::endl;
    }
    output_stream.close();




    /*--------*/
    /*cleaning*/
    /*--------*/


    delete input;

    if (return_val == SUCCESS) {
        //        delete result;
        if (backup != NULL) {
            while (!backup->isEmpty()) {
                delete backup->getHead();
                backup->popHead();
            }
            delete backup;
        }
    }

    return (return_val);
}

