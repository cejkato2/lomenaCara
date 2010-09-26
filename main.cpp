#include <stdio.h>
#include <iostream>
#include "types.h"
#include "dqueue.h"
#include "search.h"

#define MAX_AMOUNT_POINTS 3

int read_points(FILE *f, DQueue<Point *> *q) {
  int amount;
  fscanf(f, "%i", &amount);
  if (amount < MAX_AMOUNT_POINTS) {
    fclose(f);
    std::cerr << "Amount of points has to be greater or equal " << \
      MAX_AMOUNT_POINTS << std::endl;
    return MAX_AMOUNT_ERR;
  }
  for (int i=0; i<amount; ++i) {
    if (feof(f)!=0)
    {
      std::cerr << "Amount of inserted points is lesser than expected" << \
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

DQueue<Point *> *find_line(DQueue<Point *> *in) {
  DQueue<Point *> *q = new DQueue<Point *>();
  /* 
   * TODO
   * algorithm for finding line
   */
  return q;
}


void print_result(FILE *f, DQueue<Point *> *q) {

  DQueue<Point *>::Node *p = q->getHeadNode();
  while (p!=NULL) {
    fprintf(f, "%i %i\n", p->data->x, p->data->y);
    p = p->next;
  }
}



bool isElementLine(Point *a, Point *b, Point *c){

    if(*a==*c || *b==*c)
        return false;

   int det = (a->x * b->y + a->y *c->x + b->x * c->y) - (b->y * c->x + c->y*a->x + a->y * b->x);

            if(det == 0)
                return true;
            else return false;
}

int val(Point *a, Point *b, DQueue<Point *> *in){

    int count = 0;
    DQueue<Point *>::Node *myNode = in->getHeadNode();

   while(myNode!=NULL)
    {

        if(isElementLine(a,b,myNode->data))
            count++;

        if(myNode != in->getTailNode())
            myNode = myNode->next;
        else break;
    }

    return count;

}

int main(int argc, char **argv) {
  int return_val = SUCCESS;

  FILE *input_file = stdin;
  if (argc > 1) {
    input_file = fopen(argv[1], "r");
    if (input_file==NULL) {
      input_file = stdin;
    }
  }
  
  DQueue<Point *> *result, *input, *backup;
  
  /*init of input queue*/
  input = new DQueue<Point *>();

  /*get points from stdin*/
  return_val = read_points(input_file, input);
  if (return_val == SUCCESS) {
    std::cout << input;
    /* backup points */
    backup = input->copy();

    /*finding algorithm*/
    result = find_line(input);

    print_result(stdout, result);
  }


  // testing val function
  Point * a = new Point();
  a->x=3;
  a->y=1;

  Point * b = new Point();
  b->x=1;
  b->y=1;

  std::cout << "Count of points on the line " << a << " " << b << " is " << val(a,b,input) << std::endl;

   /*--------*/
  /*cleaning*/
 /*--------*/

  delete a;
  delete b;
  delete input;

  if (return_val==SUCCESS) {
    delete result;
    if (backup!=NULL) {
      while (!backup->isEmpty()) {
        delete backup->getHead();
        backup->popHead();
      }
      delete backup;
    }
  }

  return(return_val);
}

