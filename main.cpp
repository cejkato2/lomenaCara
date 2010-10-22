#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "types.h"
#include "State.h"
#include "search.h"
#include <vector>

/*!
 * minimal number of Points, which we can count with
 */
#define MIN_AMOUNT_POINTS 3

/*!
 * amount of requests for work before sending a request for token to CPU_MASTER
 */
#define TRY_GET_WORK 2

/*!
 * number of cpu, that distributes data and sends tokens/receives requests for tokens
 */
#define CPU_MASTER 0

enum cpustatus {
/*!
 * cpu has no work
 */
STATUS_IDLE,
/*!
 * cpu is working
 */
STATUS_WORKING,
/*!
 * the end of work
 */
STATUS_FINISHING
};

enum mpiflags {
/*!
 * distribution of data - broadcast
 */
FLAG_DIST_DATA, 
/*!
 * send request for token to CPU_MASTER
 */
FLAG_ASK_TOKEN,
/*!
 * try to get work from cpu 'cpu_counter'
 */
FLAG_ASK_WORK,
/*!
 * send part of job - answer to work request
 */
FLAG_SEND_WORK,
/*!
 * cpu has no work - answer to work request
 */
FLAG_IDLE,
/*!
 * CPU_MASTER sends white token
 */
FLAG_WHITE_TOKEN,
/*!
 * cpu that works and accepts token sends black token to next cpu
 */
FLAG_BLACK_TOKEN,
/*!
 * tell cpus to end themselves
 */
FLAG_FINISHING
};

/*!
 * Array of read Points.
 * This array is resent from CPU_MASTER to every other cpu
 */
Point* points = NULL; // all readed points

/*!
 * Amount of Points in array points
 */
unsigned int pointsSize = 0; // size of points array

/*!
 * final solution
 */
std::vector<Point> solution; // the solution will be stored here

/*!
 * Amount of breaks of the line
 * at first it is set to the biggest available value (all '1's in binary code)
 */
unsigned int min_breaks = (unsigned int) -1;

int permu=0;

/*!
 * total amount of cpus in cluster
 */
int cpu_amount;

/*!
 * id of this cpu
 */
int cpu_id;

/*!
 * counter - for deciding where to send MPI message
 */
int cpu_counter;


int read_points(FILE *f) {
    int amount;
    fscanf(f, "%i", &amount);
    if (amount < MIN_AMOUNT_POINTS) {
        fclose(f);
        std::cerr << "Amount of points has to be greater or equal " << MIN_AMOUNT_POINTS << std::endl;
        return MIN_AMOUNT_ERR;
    }
    pointsSize = amount;
    points = new Point[pointsSize];

    for (int i = 0; i < amount; ++i) {
        if (feof(f) != 0) { // I am fucked up
            std::cerr << "Amount of inserted points is lesser than expected" << std::endl;
            delete [] points;
            return LESS_AMOUNT_ERR;
        }
        Point p;
        fscanf(f, "%i %i", &p.x, &p.y);
        points[i] = p;
    }

    fclose(f);
    return SUCCESS;
}

void print(std::vector<Point> &v) {
    std::vector<Point>::const_iterator it;
    for (it = v.begin(); it != v.end(); ++it) {
        std::cout << " " << *it;
    }
    std::cout << std::endl;
}




void permut(const Point *pointArray){
    /*!
     * attempts of requests for job
     */
    int job_requests = 0;
    int cpu_state = STATUS_IDLE;

    std::vector<State *> stack; // implicit stack
    std::vector<bool> mask(pointsSize,true); 

    if (cpu_id == CPU_MASTER) {
      // initialize stack, put first level of values
      for(unsigned int i=0;i<pointsSize;i++){
        State *state = new State(1, pointArray);
        state->setLastIndex(i);
        stack.push_back(state);
      }
    }

    
    int lastPosition;



    do {
      if (stack.empty()) {
        std::cerr << "cpu#" << cpu_id << " IDLE" << std::endl;
        cpu_state = STATUS_IDLE;


        //TODO request for work
        //TODO receive answer
        //
        ////TODO if request amount reaches TRY_GET_WORK, request for token
        ////receive token - if it is white, wait for FLAG_FINISHING otherwise
        ////request for job from black cpu
        //

        //developing end
        cpu_state = STATUS_FINISHING;
      }
      
      while(!stack.empty()){

          State *parentState=stack.back();
          stack.pop_back();

          

          if(parentState->getSize()==pointsSize) // if I am on a floor, I cannot expand
          { 
              unsigned int count = parentState->getPrice();

              if (count < min_breaks) {
                std::cout << "prev min_breaks " << min_breaks;
                min_breaks = count;
                std::cout << " new: " << min_breaks << std::endl;
                solution.clear();

                std::vector<int> indexes = parentState->getIndexes();
                
                std::vector<int>::iterator it;
                for (it=indexes.begin(); it!=indexes.end(); ++it) {
                  solution.push_back( points[*it] );
                }
                print(solution);
              }
              
              delete parentState;
              continue;

          }

          lastPosition=0;
          
          // set all mask variables to true
          for(unsigned int i=0;i<pointsSize;i++){
              mask[i]=true;
          }

          
          // set mask variables depend on used indexes
          for(unsigned int i=0;i<parentState->getSize();i++){
              mask[parentState->getIndex(i)]=false;
          }

          // here I expand my state to stack
          for(unsigned int i=0;i<pointsSize-parentState->getSize();i++){
              
              while(!mask[lastPosition]) // iterate to position where mask variable=true, thats the index which can be added to new state
                  lastPosition++;

              if(parentState->getExpandPrice(lastPosition) >= min_breaks) // if I cannot get better solution = bounding
              {
                  lastPosition++;
                  continue;
              }

              State *childState = new State(*parentState); // here I make a deep copy
              childState->setLastIndex(lastPosition); // add a new part of state
              lastPosition++;
              stack.push_back(childState);
          }
          delete parentState; // now I can delete parent state
          

      }
    } while(cpu_state != STATUS_FINISHING);

    //TODO: bcast solution

}


int main(int argc, char **argv) {

    int return_val = SUCCESS;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &cpu_id);
    MPI_Comm_size(MPI_COMM_WORLD, &cpu_amount);

    cpu_counter = cpu_id; //at first set counter to myself (than circular pointing)

    std::cout << "cpu#: " << cpu_id << " of " << cpu_amount << std::endl;

    if (cpu_id == CPU_MASTER) {
      FILE *input_file = stdin;
      if (argc > 1) {
        input_file = fopen(argv[1], "r");
        if (input_file == NULL) {
          input_file = stdin;
        }
      }

      /*get points from stdin*/
      return_val = read_points(input_file);
      void *pointer = (void *) &pointsSize;
      std::cout << "cpu#" << cpu_id << " bcast " << pointsSize << std::endl;
      MPI_Bcast(pointer, sizeof(pointsSize), MPI_UNSIGNED, FLAG_DIST_DATA, MPI_COMM_WORLD);
      pointer = (void *) points;
      MPI_Bcast(pointer, pointsSize*sizeof(Point), MPI_BYTE, FLAG_DIST_DATA, MPI_COMM_WORLD);

      std::cout << "cpu#" << cpu_id << " ";
      for (unsigned int i=0; i<pointsSize; ++i) {
        std::cout << points[i] << " ";
      }
      std::cout << std::endl;

      if (return_val != SUCCESS) {
        return return_val;
      }
    } else {
      
      void *pointer = (void *) &pointsSize;
      MPI_Bcast(pointer, sizeof(pointsSize), MPI_UNSIGNED, FLAG_DIST_DATA, MPI_COMM_WORLD);
      std::cout << "cpu#" << cpu_id << " pointSize: " << pointsSize << std::endl;
      points = new Point[pointsSize];
      pointer = points;
      MPI_Bcast(pointer, pointsSize*sizeof(Point), MPI_BYTE, FLAG_DIST_DATA, MPI_COMM_WORLD);
      std::cout << "cpu#" << cpu_id << " ";
      for (unsigned int i=0; i<pointsSize; ++i) {
        std::cout << points[i] << " ";
      }
      std::cout << std::endl;

    }

    std::ofstream output_stream;
    if (cpu_id == CPU_MASTER) {
      output_stream.open("vystup1.dat");
      for (unsigned int i = 0; i < pointsSize; i++) {
        output_stream << points[i].x << " " << points[i].y << std::endl;
      }
      output_stream.close();
    }

    permut(points);

    //std::cout << "All permutations:" << std::endl;
    if (cpu_id == CPU_MASTER) {
      output_stream.open("vystup2.dat");
      for (std::vector<Point>::iterator it = solution.begin(); it != solution.end(); ++it) {
        output_stream << it->x << " " << it->y << std::endl;
      }
      output_stream.close();
      print(solution); // not yet solution
    }


    /*--------*/
    /*cleaning*/
    /*--------*/

    delete [] points;

    
    /*!
     * wait for all cpus to end their job
     */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();


    return (return_val);
}

