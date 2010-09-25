#include <stdio.h>
#include <iostream>
#include "types.h"
#include "dqueue.h"
#include "search.h"

void read_points(FILE *f, DQueue<Point *> *q) {

}

DQueue<Point *> *find_line(DQueue<Point *> *in) {
  DQueue<Point *> *q = new DQueue<Point *>();
  return q;
}

void print_result(FILE *f, DQueue<Point *> *q) {

}

int main(int argc, char **argv) {
  DQueue<Point *> *result, *input;
  
  /*init of input queue*/
  input = new DQueue<Point *>();

  /*get points from stdin*/
  read_points(stdin, input);
  
  /*finding algorithm*/
  result = find_line(input);

  print_result(stdout, result);

  /*cleaning*/
  delete input;
  /*we still have to release every point in queue*/
  delete result;

  return(0);
}

