#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "types.h"
#include "State.h"
#include "search.h"
#include <vector>
#include <list>

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

/*!
 * number of cpu, which follows current cpu
 */
#define CPU_NEXT_NEIGH ((cpu_id+1)%cpu_amount)

/*!
 * how many loop cycles should cpu count before getting new message
 */
#define MAX_TIME_FROM_IPROBE 2

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
FLAG_FINISHING,
/*!
 * flag for function to wait for any token
 */
FLAG_TOKEN,
/*!
 * flag for function to wait for any flag
 */
FLAG_ANY
};

void wait_for_message(std::list<State *> &stack, int flag, MPI_Status *status);


/*!
 * translate mpiflags to text
 * @param flag - number, value of flag
 */
std::string getFlagName( int flag )
{
  switch (flag) {
    case FLAG_DIST_DATA:
      return "DISTRIB_DATA";
    case FLAG_ASK_TOKEN:
      return "ASK_FOR_TOKEN";
    case FLAG_ASK_WORK:
      return "ASK_FOR_WORK";
    case FLAG_SEND_WORK:
      return "SENDING_WORK";
    case FLAG_IDLE:
      return "HAVE_NO_WORK";
    case FLAG_WHITE_TOKEN:
      return "WHITE_TOKEN";
    case FLAG_BLACK_TOKEN:
      return "BLACK_TOKEN";
    case FLAG_FINISHING:
      return "EVERYBODY_FINISH!";
  }
  return "";
}


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
State *solution; // the solution will be stored here



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
 * state of cpu, at first it is idle, at the end STATUS_FINISHING
 */
int cpu_state = STATUS_IDLE;

/*!
 * counter - for deciding where to send MPI message
 */
int cpu_counter;

/*!
 * number of cpu, whom I sent work
 */
int cpu_sent_work = -1;

#define MESSAGE_BUF_SIZE 1025
char message_buf[MESSAGE_BUF_SIZE];

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

void print(State *state){

    for(unsigned int i =0;i<state->getSize(); i++){
        std::cout << " " << points[state->getIndex(i)] ;
    }

    std::cout << std::endl;

}

/*!
 * This function test stack, if dividable then returns true else false
 * @param sv own stack
 * @return s chosen state
 */
bool is_dividable(std::list<State *> &stack)
{
  //TODO FIXME

    if((stack.size() >= (unsigned int) 2))
        return true;
  
    else return false;
  
}
/*!
 * This function should find the best State from stack, that
 * should be sent to other cpu and delete it from own stack. We use D-ADZ algorithm
 * @param stack own stack
 * @return s chosen state
 */
std::vector<State *> choose_state_for_share(std::list<State *> &stack)
{

    if(!is_dividable(stack)) // stack has only one state object, cannot be divided
    {
    std::cerr << "Error - own stack is too small, it cannot be shared" << std::endl;
    return std::vector<State *>();
    }


    int countSameLevel = 0;
    unsigned int level= stack.front()->getSize();

    std::list<State *>::iterator it;

    // here i count states on the tom level
    for ( it=stack.begin() ; it != stack.end(); it++ )
    {
        if(level == (*it)->getSize())
            countSameLevel++;
        else
            break;
    }

    // divide half
    int howManyStates = ( (countSameLevel - (countSameLevel % 2)) / 2 );

    if(howManyStates == 0)
        howManyStates = 1;


    std::vector<State *> ret = std::vector<State *>();
    ret.reserve( (unsigned) howManyStates );

    // get states pointers
    int i =0;
     for ( it=stack.begin(); i<howManyStates; it++, i++ ){
         ret.push_back(*it);
     }

    // delete those states pointers from stack

    stack.erase(stack.begin(), it);


    return ret;
}


/*!
 * divide stack s and send part to idle cpu target
 */
void send_work(std::list<State *> &stack, int target)
{
  // get some state for sending
  std::vector<State *> list = choose_state_for_share(stack);
  
  //send amount of States
  unsigned int *amount = (unsigned int *) message_buf;
  unsigned int buf = list.size();
  std::cerr << "cpu#" << cpu_id << "buf(amount of states): " << buf << std::endl;
  *amount = buf;
  MPI_Send(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, target, FLAG_SEND_WORK,
      MPI_COMM_WORLD);


  //send States
  std::vector<State *>::iterator it;
  for (it=list.begin(); it!=list.end(); ++it) {
    amount = (unsigned int *) message_buf;
    buf = (*it)->getSize();
    std::cerr << "cpu#" << cpu_id << "buf(amount of indexes): " << buf << std::endl;
    *amount = buf;
    //send size of array
    MPI_Send(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, target, FLAG_SEND_WORK,
        MPI_COMM_WORLD);

    unsigned int *indexes = (*it)->getArrayPointerIndexes();
    std::cerr << "indexes: ";
    for (unsigned int i = 0; i < buf; ++i) {
      std::cerr << indexes[i] << " ";
    }
    std::cerr << std::endl;

    //send array of indexes
    MPI_Send((void *) indexes, *amount, MPI_INT, target, FLAG_SEND_WORK,
        MPI_COMM_WORLD);

    delete *it;
  }

  list.clear();
  std::cerr << "sent indexes" << std::endl;

  std::cerr << "cpu#" << cpu_id << " MPI_Send send work target: " << target << std::endl;
}

/*!
 * Decide whether to resend white or black token
 */
void handle_white_token()
{
  if (cpu_state == STATUS_WORKING) {
    int *id = (int *) message_buf;
    *id = cpu_id;
    MPI_Send(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, CPU_NEXT_NEIGH,
        FLAG_BLACK_TOKEN, MPI_COMM_WORLD);
    std::cerr << "cpu#" << cpu_id << " received WT resend BT" << std::endl;
  } else {
    //no work, resend white token
    MPI_Send(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, CPU_NEXT_NEIGH,
        FLAG_WHITE_TOKEN, MPI_COMM_WORLD);
    std::cerr << "cpu#" << cpu_id << " received WT resend WT" << std::endl;
  }
}



/*!
 * receive MPI message if any and handle it
 * it should include sending another job
 * @param stack - pointer to stack for dividing
 * @param status - variable for storing mpi status
 */
void handle_messages(std::list<State *> &stack, MPI_Status *status)
{
  /*!
   * variable for storing status of message
   */
  //receive message
  MPI_Recv(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, status->MPI_SOURCE, status->MPI_TAG,
    MPI_COMM_WORLD, status);
  std::cerr << "cpu#" << cpu_id << " MPI_Recv tag: " << getFlagName(status->MPI_TAG);
  std::cerr << " source: " << status->MPI_SOURCE << std::endl;

  switch (status->MPI_TAG) {
    case FLAG_ASK_WORK: {
      if (is_dividable(stack)) {
        send_work(stack, status->MPI_SOURCE);
      } else {
        //no work, I'm idle
        MPI_Send(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, status->MPI_SOURCE, FLAG_IDLE, 
            MPI_COMM_WORLD);
        std::cerr << "cpu#" << cpu_id << " MPI_Send tag: " << getFlagName(status->MPI_TAG);
        std::cerr << " source: " << status->MPI_SOURCE << std::endl;
      }
      }
      break;
    case FLAG_WHITE_TOKEN: {
        //only for non CPU_MASTER
        if (cpu_id != CPU_MASTER) {
          handle_white_token();
        } else {
          std::cerr << "CPU_MASTER in FLAG_WHITE_TOKEN ERROR!!!" << std::endl;
        }
      }
      break;
    case FLAG_BLACK_TOKEN: {
        //only for non CPU_MASTER
        if (cpu_id != CPU_MASTER) {
          MPI_Send(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, CPU_NEXT_NEIGH,
              FLAG_WHITE_TOKEN, MPI_COMM_WORLD);
          std::cerr << "cpu#" << cpu_id << " received BT resend BT MPI_Send tag: " << getFlagName(status->MPI_TAG);
          std::cerr << " source: " << status->MPI_SOURCE << std::endl;
        } else {
          std::cerr << "CPU_MASTER in FLAG_BLACK_TOKEN ERROR!!!" << std::endl;
        }
      }
      break;
    case FLAG_ASK_TOKEN: {
      //only for CPU_MASTER (target id in send)

        handle_white_token();

        MPI_Status status;

        wait_for_message(stack, FLAG_TOKEN, &status);

        MPI_Recv(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, status.MPI_SOURCE, MPI_ANY_TAG,
            MPI_COMM_WORLD, &status);
        std::cerr << "cpu#" << cpu_id << " in ask_token, received message " << getFlagName(status.MPI_TAG) << std::endl;

        if (status.MPI_TAG == FLAG_WHITE_TOKEN) {
          //everybody is idle
          MPI_Send(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, CPU_NEXT_NEIGH,
              FLAG_FINISHING, MPI_COMM_WORLD);
          cpu_state = STATUS_FINISHING;
        }
      }
      break;            
      
    case FLAG_FINISHING: {
      std::cerr << "cpu#" << cpu_id << " received " << getFlagName(FLAG_FINISHING) << std::endl;
      if (cpu_id != CPU_MASTER) {
          MPI_Send(message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, CPU_NEXT_NEIGH,
              FLAG_FINISHING, MPI_COMM_WORLD);
      }
      cpu_state = STATUS_FINISHING;
      }
      break;
      //other message types
  }
}

/*!
 * Accept messages in loop and return if message has expected flag,
 * otherwise handle_message
 * @param stack - stack for handle_message
 * @param flag - what message to wait for, FLAG_TOKEN for any token (W/B)
 */
void wait_for_message(std::list<State *> &stack, int flag, MPI_Status *status)
{
  int isMessage = 0;
  int tag = -1;

  while (true) {
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
    tag = status->MPI_TAG;

    if (flag == FLAG_TOKEN) {
      if ((tag == FLAG_WHITE_TOKEN) || (tag == FLAG_BLACK_TOKEN)) {
        return;
      }
    } else if (flag == FLAG_ANY) {
      return;
    } else {
      if (tag == flag) {
        return;
      }
    }

    std::cerr << "cpu#" << cpu_id << " MPI_Iprobe " << isMessage << std::endl;
    handle_messages(stack, status);
  }
}

/*!
 * send request for work to target cpu and receive its answer
 * @param stack - private stack
 * @param target - number of cpu, whom to ask
 */
void handle_request_work(std::list<State *> &stack, int target)
{
  MPI_Status status;

  MPI_Send((void *) message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, cpu_counter, 
      FLAG_ASK_WORK, MPI_COMM_WORLD);


  MPI_Recv((void *) message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, cpu_counter, 
      MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  std::cerr << "cpu#" << cpu_id << " asked for work, received tag: " << getFlagName(status.MPI_TAG) << std::endl;
  if (status.MPI_TAG == FLAG_SEND_WORK) {

    //get amount of states
    int *buf = (int *) message_buf;
    int amount_states = *buf;

    for (int i=0; i<amount_states; ++i) {
      //receive amount of indexes
      MPI_Recv((void *) message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, cpu_counter, 
          MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      int *buf = (int *) message_buf;
      int amount = *buf;
      int indexes[amount];

      //receive array of indexes
      MPI_Recv((void *) indexes, amount, MPI_INT, cpu_counter, 
          MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      State *ns = new State(amount, points, indexes);
      stack.push_back(ns);
      std::cerr << "cpu#" << cpu_id << " got work: " << *ns << std::endl;
    }
    cpu_state = STATUS_WORKING;
  }
}

void permut(const Point *pointArray){
    /*!
     * attempts of requests for job
     */
    int job_requests = 0;

    /*!
     * number of loop cycles after last Iprobe
     */
    int time_from_iprobe = 0;

    std::list<State *> stack; // implicit stack
    std::vector<bool> mask(pointsSize,true); 

    if (cpu_id == CPU_MASTER) {
      // initialize stack, put first level of values
      for(unsigned int i=0;i<pointsSize;i++){
        State *state = new State(1, pointArray);
        state->setLastIndex(i);
        stack.push_back(state);
      }
    }

    

    do {
      if (cpu_state == STATUS_FINISHING) {
        break;
      }

      if (stack.empty()) {
        std::cerr << "cpu#" << cpu_id << " IDLE" << std::endl;
        cpu_state = STATUS_IDLE;
        do {
          cpu_counter++;
          cpu_counter %= cpu_amount;
          if (cpu_counter == cpu_id) {
            cpu_counter++;
            cpu_counter %= cpu_amount;
          }
          
          std::cerr << "cpu#" << cpu_id << " " << job_requests << ". ask for work from cpu#";
          std::cerr << cpu_counter << std::endl;

          handle_request_work(stack, cpu_counter);
          if (cpu_state == STATUS_WORKING) {
            break;
          }

        } while (job_requests++ < TRY_GET_WORK);
        job_requests = 0;

        if (cpu_state == STATUS_IDLE) {
          //token
          MPI_Send((void *) message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, CPU_MASTER, 
              FLAG_ASK_TOKEN, MPI_COMM_WORLD);

          std::cerr << "cpu#" << cpu_id << " sent request for token" << std::endl;


          MPI_Status status;

          wait_for_message(stack, FLAG_TOKEN, &status);
          handle_messages(stack, &status);

          MPI_Send((void *) message_buf, MESSAGE_BUF_SIZE, MPI_CHAR, CPU_NEXT_NEIGH, 
              status.MPI_TAG, MPI_COMM_WORLD);

          if (status.MPI_TAG == FLAG_BLACK_TOKEN) {
            //TODO get work from cpu in black token
          }

        }

      //  cpu_state = STATUS_FINISHING;
       // std::cerr << "cpu#" << cpu_id << " setting state to finishing" << std::endl;
      }

      if (cpu_state == STATUS_FINISHING) {
        std::cerr << "cpu#" << cpu_id << " breaking cyckle because of FINISHING" << std::endl;
        break;
      }
      
      while(!stack.empty()){

          State *parentState=stack.back();
          stack.pop_back();

          parentState->expand(stack, solution, mask, cpu_id, pointsSize);
          
          if (time_from_iprobe >= MAX_TIME_FROM_IPROBE) {
            //check for new message
            MPI_Status status;
            int isMessage = 0;

            do {
              MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &isMessage, &status);
              if (isMessage != 0) {
                std::cerr << "cpu#" << cpu_id << " received message (MPI_Iprobe)" << std::endl;
                handle_messages(stack, &status);
              }
            } while (isMessage != 0);
            time_from_iprobe = 0;
          }
          time_from_iprobe++;
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
      for (unsigned int i =0; i< ((solution == NULL)? 0 : solution->getSize()) ;i++) {
        output_stream << (points[solution->getIndex(i)]).x << " " << points[solution->getIndex(i)].y << std::endl;
      }
      output_stream.close();
      print(solution); 
      
    }


    /*--------*/
    /*cleaning*/
    /*--------*/

    
    /*!
     * wait for all cpus to end their job
     */
    std::cerr << "cpu#" << cpu_id << " standing behind barrier" << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    unsigned int myPrice = solution->getPrice();
    unsigned int bestPrice;
    MPI_Reduce((void *) &myPrice, (void *) &bestPrice, 1,
        MPI_INT, MPI_MIN, CPU_MASTER, MPI_COMM_WORLD);

    if (cpu_id == CPU_MASTER) {
      std::cerr << "Best solution has " << bestPrice << " breaks..." << std::endl;
    }

    MPI_Finalize();

    delete [] points;
    
    if(solution != NULL)
        delete solution;



    return (return_val);
}

