#include <iostream>
#include "types.h"
#include "dqueue.h"
#include "search.h"


int main(int argc, char **argv) {
  DQueue *result, *input;
  
  /*init of input queue*/
  input = new DQueue<Point *>();

  /*get points from stdin*/
  read_points(stdin, input);
  
  /*init of queue for storing results*/
  result = new DQueue<Point *>()

  /*finding algorithm*/
  find_line(input, result);

  print_result(stdout, result);

  /*cleaning*/
  delete input;
  delete result;

  return(0);
}

