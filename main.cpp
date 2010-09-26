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

   /*--------*/
  /*cleaning*/
 /*--------*/
  delete input;
  delete result;

  if (return_val==SUCCESS) {
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

