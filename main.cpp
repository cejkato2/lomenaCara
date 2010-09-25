#include <stdio.h>
#include <iostream>
#include "types.h"
#include "dqueue.h"
#include "search.h"

void read_points(FILE *f, DQueue<Point *> *q) {
  int amount;
  fscanf(f, "%i", &amount);
  for (int i=0; i<amount; ++i) {
    Point *p = new Point();
    fscanf(f, "%i %i", &p->x, &p->y);
    q->addHead(p);
  }

  fclose(f);
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

}

int main(int argc, char **argv) {
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
  read_points(input_file, input);
  std::cout << input;
  /* backup points */
  backup = input->copy();
  
  /*finding algorithm*/
  result = find_line(input);

  print_result(stdout, result);


   /*--------*/
  /*cleaning*/
 /*--------*/
  delete input;
  delete result;

  while (!backup->isEmpty()) {
    delete backup->getHead();
    backup->popHead();
  }
  delete backup;

  return(0);
}

