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
#define MAX_TIME_FROM_IPROBE 10

#define HEADER_SIZE 1 // defines size of header

/*!
 * defines position of lenght of message
 */
#define LENGHT_POSITION 0 

/*!
 * it defines total lenght of message
 */
#define MESSAGE_SIZE (buffer[LENGHT_POSITION]+HEADER_SIZE) 

/*!
 * defines where start body of message
 */
#define BODY_POSITION (HEADER_SIZE) 

/*!
 * color of cpu is used to decide whether to send BLACK token after sending work to another cpu;
 * it is important for situation when sending work to predecessor
 */
enum color {
    WHITE,
    BLACK,
    NONE
};

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

enum mpiTags {
    /*!
     * distribution of data - broadcast
     */
    MSG_DIST_DATA,
    /*!
     * try to get work from cpu 'cpu_counter'
     */
    MSG_WORK_REQUEST,
    /*!
     * send part of job - answer to work request
     */
    MSG_WORK_TRANSFER,
    /*!
     * cpu has no work - answer to work request
     */
    MSG_NO_WORK,
    /*!
     * CPU_MASTER sends white token
     */
    MSG_WHITE_TOKEN,
    /*!
     * cpu that works and accepts token sends black token to next cpu
     */
    MSG_BLACK_TOKEN,
    /*!
     * tell cpus to end themselves
     */
    MSG_FINISHING,

    MSG_BEST_GLOBAL_SOLUTION

};

/*!
 * translate mpiflags to text
 * @param flag - number, value of flag
 */
std::string getFlagName(int flag) {
    switch (flag) {
        case MSG_DIST_DATA:
            return "DISTRIB_DATA";
        case MSG_WORK_REQUEST:
            return "WORK_REQUEST";
        case MSG_WORK_TRANSFER:
            return "SENDING_WORK";
        case MSG_NO_WORK:
            return "HAVE_NO_WORK";
        case MSG_WHITE_TOKEN:
            return "WHITE_TOKEN";
        case MSG_BLACK_TOKEN:
            return "BLACK_TOKEN";
        case MSG_FINISHING:
            return "EVERYBODY_FINISHING!";
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
 * number of loop cycles after last Iprobe
 */
int timeFromIprobe = 0;

/**
 try to get work
 */
int tryToGetWork = 0;

color tokenColor = NONE;

/*!
 * max size of memory for storing incoming and outgoing data
 */
int bufferSize = 2000;
/*!
 * buffer for incoming and outgoing data
 */
int buffer[2000];

int lastTimeOutPut = 0;
unsigned int bestGlobalSolution = (unsigned) - 1;

/*!
 * Reading input data from FILE
 * @param f - file descriptor
 */
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

/*!
 * Print points in order saved in state.
 * @param state - state in DFS algorithm 
 */
void print(State *state) {

    for (unsigned int i = 0; i < state->getSize(); i++) {
        std::cout << " " << points[state->getIndex(i)];
    }

    std::cout << std::endl;

}

/*!
 * This function test stack, if dividable then returns true else false
 * @param sv own stack
 * @return s chosen state
 */
bool is_dividable(std::list<State *> &stack) {
    if ((stack.size() >= (unsigned int) 2))
        return true;

    else return false;
}

/*!
 * This function should find the best State from stack, that
 * should be sent to other cpu and delete it from own stack. We use D-ADZ algorithm
 * @param stack own stack
 * @return s chosen state
 */
std::vector<State *> getSharedStates(std::list<State *> &stack) {

    if (!is_dividable(stack)) // stack has only one state object, cannot be divided
    {
        std::cerr << "cpu#" << cpu_id << ": Error - own stack is too small, it cannot be shared" << std::endl;
        return std::vector<State *>();
    }


    int countSameLevel = 0;
    unsigned int level = stack.front()->getSize();

    std::list<State *>::iterator it;

    // here i count states on the tom level
    for (it = stack.begin(); it != stack.end(); it++) {
        if (level == (*it)->getSize())
            countSameLevel++;
        else
            break;
    }

    // divide 
    int howManyStates = countSameLevel / 3;

    if (howManyStates == 0)
        howManyStates = 1;


    std::vector<State *> ret = std::vector<State *>();
    ret.reserve((unsigned) howManyStates);

    // get states pointers

    for (int i = 0; i < howManyStates; i++) {
        ret.push_back(stack.front()); // take state from front of stack
        stack.pop_front(); // delete state from front of stack
    }

    return ret;
}

/*!
 * Send MPI message to cpu target with MPI_TAG mpitag.
 * Size of message is MESSAGE_SIZE*MPI_INT and it is stored in buffer.
 * @param target - number of cpu, that will get message
 * @param mpitag - type of message, that is sent
 */
void sendMessage(int target, mpiTags mpitag) {

    MPI_Send(&buffer, MESSAGE_SIZE, MPI_INT, target, mpitag, MPI_COMM_WORLD);
}

/*!
 * Serialize state and send it to target.
 * @param sendState - state of DFS algorithm, that will be sent
 * @param target - number of cpu, that will get state
 */
void sendState(State * sendState, int target) {

    buffer[LENGHT_POSITION] = sendState->getSize();

    unsigned int *arrayToCopy = sendState->getArrayPointerIndexes();
    // I must copy it to the buffer
    for (unsigned int i = 0; i < sendState->getSize(); i++) {
        buffer[BODY_POSITION + i] = arrayToCopy[i];
    }

    sendMessage(target, MSG_WORK_TRANSFER);
}

/*!
 * Divide stack and send the part of it to idle cpu target, that asked for work.
 * Function sets color of cpu to BLACK, if cpu sends work to any predecessor.
 * @param stack - stack of DFS algorithm
 * @param target - number of cpu, that will receive work
 */
void sendWork(std::list<State *> &stack, int target) {

    if (!is_dividable(stack)) { // stack is not dividable -> cannot send work
        buffer[LENGHT_POSITION] = 0; // set size of data to zero, no data will be send
        sendMessage(target, MSG_NO_WORK);
        return;
    }

    // get some state for sending
    std::vector<State *> list = getSharedStates(stack);
    std::vector<State *>::iterator it;

    // send all available states
    for (it = list.begin(); it < list.end(); it++) {

        sendState(*it, target);

        delete *it;
    }

    // in order to avoid sending bad token set BLACK color of cpu
    if (cpu_id > target) {
        tokenColor = BLACK;
    }
    std::cout << "cpu#" << cpu_id << ": send " << list.size() << " states to cpu#" << target << std::endl;
}

/**
 * Send request for work to cpu with number target
 */
void requestWork() {

    // this cpu sent work to any predecessor, so it will send black token for sure,
    // because of avoiding early finishing
    if (tokenColor == BLACK) {
        tokenColor = NONE;
        sendMessage(CPU_NEXT_NEIGH, MSG_BLACK_TOKEN);
    } else if (tokenColor == WHITE) {
        tokenColor = NONE;
        sendMessage(CPU_NEXT_NEIGH, MSG_WHITE_TOKEN);
    }


    int target = tryToGetWork;
    target = target % cpu_amount;

    if (target == cpu_id) {
        target++;
        target = target % cpu_amount;
    }

    buffer[LENGHT_POSITION] = 0; // there are no data
    sendMessage(target, MSG_WORK_REQUEST);

    std::cout << "cpu#" << cpu_id << ": send MSG_WORK_REQUEST to " << target << std::endl;

    if (tryToGetWork > (cpu_amount + 10))
        tryToGetWork = 0;

    tryToGetWork++;
}

/*!
 * receive MPI message if any and handle it
 * it should include sending another job
 * @param stack - pointer to stack of DFS algorithm
 * @param blockingRecv - sets the mode of function to blocking or non-blocking, false for non-blocking
 */
void handleMessages(std::list<State *> &stack, bool blockingRecv) {


    MPI_Status status;
    int flag = 0;

    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

    if (flag == 0 && !blockingRecv) // there is no message and i cant use blockingRecieve
        return;


    bool repeatRecieve = true;

    while (repeatRecieve) {
        repeatRecieve = false;

        // recieve what I can, and store it to the buffer
        MPI_Recv(&buffer, bufferSize, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);


        switch (status.MPI_TAG) {
            case MSG_WORK_REQUEST: // somebody ask me for work
            {
                std::cout << "cpu#" << cpu_id << ": cpu#" << status.MPI_SOURCE << " asked me for work" << std::endl;
                sendWork(stack, status.MPI_SOURCE);

                if (stack.size() == 0) // I probably send NO_WORK so, I will wait to next message
                    repeatRecieve = true;

                break;
            }

            case MSG_WORK_TRANSFER: // somebody send me a work
            {
                std::cout << "cpu#" << cpu_id << ": cpu#" << status.MPI_SOURCE << " sends me work, my stack.size() was =" << stack.size();
                tryToGetWork = 0; // I get work, set tryToGetWork=0 for next time

                // store state/work to local implicit stack
                State *recievedState = new State(buffer[LENGHT_POSITION], points, &buffer[BODY_POSITION]);
                stack.push_back(recievedState);

                std::cout << " and now is " << stack.size() << std::endl;

                cpu_state = STATUS_WORKING;
                break;
            }

            case MSG_NO_WORK: // somebody tells me that he cannot give me work
            {
                std::cout << "cpu#" << cpu_id << ": cpu#" << status.MPI_SOURCE << " cannot give me work" << std::endl;

                if (buffer[LENGHT_POSITION] != 0)
                    std::cerr << "===========Something wrong, no data expected =======";


                if ((tryToGetWork > cpu_amount) && (cpu_id == CPU_MASTER) && (cpu_state == STATUS_IDLE)) // I am CPU_MASTER and I tried to get work, but nobody give it to me -> send white token
                {
                    buffer[LENGHT_POSITION] = 0;
                    sendMessage(CPU_NEXT_NEIGH, MSG_WHITE_TOKEN);
                    tryToGetWork = 0;

                }

                requestWork(); // request for work to next CPU
                repeatRecieve = true; // here i use blocking recv
                break;
            }

            case MSG_WHITE_TOKEN: // neighbour cpu state is IDLE and sends me a white token
            {
                std::cout << "cpu#" << cpu_id << ": cpu#" << status.MPI_SOURCE << " send me a WHITE TOKEN" << std::endl;

                if (cpu_id == CPU_MASTER) // I am master and I have got white token -> set and send finishing
                {
                    cpu_state = STATUS_FINISHING;
                    buffer[LENGHT_POSITION] = 0;
                    sendMessage(CPU_NEXT_NEIGH, MSG_FINISHING);

                } else { // if I am not CPU_MASTER -> decide what next
                    buffer[LENGHT_POSITION] = 0;

                    if (tokenColor == NONE) // token který poslu az nebudu mit praci bude white, pokud byl dosud NONE
                        tokenColor = WHITE;
                }

                break;
            }

            case MSG_BLACK_TOKEN: // neighbour send me black token -> I will send it to neighbour, nothing else
            {
                std::cout << "cpu#" << cpu_id << ": cpu#" << status.MPI_SOURCE << " send me a BLACK TOKEN" << std::endl;

                buffer[LENGHT_POSITION] = 0;

                if (cpu_id == CPU_MASTER)
                    sendMessage(CPU_NEXT_NEIGH, MSG_WHITE_TOKEN);
                else
                    tokenColor = BLACK;

                break;
            }

            case MSG_FINISHING: // neighbour sends me MSG_FINISHING, I finish and send finish to other neighbour
            {
                std::cout << "cpu#" << cpu_id << ": cpu#" << status.MPI_SOURCE << " send me a MSG_FINISHING" << std::endl;

                cpu_state = STATUS_FINISHING;
                buffer[LENGHT_POSITION] = 0;

                if (cpu_id != (cpu_amount - 1)) // if I am not last one
                    sendMessage(CPU_NEXT_NEIGH, MSG_FINISHING);

                break;
            }

            case MSG_BEST_GLOBAL_SOLUTION:
            {

                 int recievedSolution = buffer[0];

                if (bestGlobalSolution > (unsigned) recievedSolution) {

                    bestGlobalSolution = (unsigned) recievedSolution;
                    std::cerr << "cpu#" << cpu_id << ": cpu#" << status.MPI_SOURCE << " send me bestGlobalSolution: " << recievedSolution << "my bestGlobal solution is:" << bestGlobalSolution << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<------------------"<< std::endl;

                }
                break;
            }

            default:
            {
                std::cerr << std::endl << std::endl << "!!!!!!!!=========Fatal error, unknown type of message========!!!!!!!!" << std::endl << std::endl << std::endl;
                return;
            }


        }


    }
}

/*!
 * Main counting function. It generates permutations using DFS - expanding states 
 * on the stack (calling expand of state).
 * @param pointArray - array of Points, that CPU_MASTER got from input
 */
void permut(const Point *pointArray) {


    std::list<State *> stack; // implicit stack
    std::vector<bool> mask(pointsSize, true);

    if (cpu_id == CPU_MASTER) {
        // initialize stack, put first level of values
        for (unsigned int i = 0; i < pointsSize; i++) {
            State *state = new State(1, pointArray);
            state->setLastIndex(i);
            stack.push_back(state);
        }
    }

    while (cpu_state != STATUS_FINISHING) { // outer loop

        if (stack.empty()) {
            std::cerr << "cpu#" << cpu_id << " local stack empty -> status = IDLE" << std::endl;
            cpu_state = STATUS_IDLE;

            requestWork();
            handleMessages(stack, true); // use blocking recv, because there is no work at all
        }

        if (cpu_state == STATUS_FINISHING) {
            std::cerr << "cpu#" << cpu_id << " there is no work -> FINISHING" << std::endl;
            break;
        }

        while (!stack.empty()) { // inner loop

            State *parentState = stack.back();
            stack.pop_back();

            parentState->expand(stack, solution, mask, cpu_id, pointsSize, bestGlobalSolution);

            if ((solution != NULL) && (bestGlobalSolution > solution->getPrice())) {

                bestGlobalSolution = solution->getPrice();
                int tempBuf = (int) bestGlobalSolution;
                // send it to all by MPI_ISend
                MPI_Request request;

                std::cerr << "cpu#" << cpu_id << " sending bestGlobalSolution=" << tempBuf << " to ALL" << std::endl  << std::endl;
                for(int i=0;i<cpu_amount;i++){
                    if(i!=cpu_id)
                        MPI_Isend(&tempBuf , 1, MPI_INT, i, MSG_BEST_GLOBAL_SOLUTION, MPI_COMM_WORLD, &request );
                }
            }

            if (lastTimeOutPut > 5000) {
                std::cerr << "cpu#" << cpu_id << " working, my stack.size()=" << stack.size() << std::endl;
                lastTimeOutPut = 0;
            } else {
                lastTimeOutPut++;
            }


            if (timeFromIprobe >= MAX_TIME_FROM_IPROBE) {
                handleMessages(stack, false);
                timeFromIprobe = 0;
            } else
                timeFromIprobe++;
        }
    }

    std::cout << "cpu#" << cpu_id << "============= End of calculation ===============" << std::endl;
}

int main(int argc, char **argv) {

    int return_val = SUCCESS;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &cpu_id);
    MPI_Comm_size(MPI_COMM_WORLD, &cpu_amount);


    if (cpu_id == CPU_MASTER) {
        std::cout << "================" << std::endl;
        std::cout << "AMOUNT OF CPUs: " << cpu_amount << std::endl;
        std::cout << "================" << std::endl;
    }

    std::cout << "cpu#: " << cpu_id << " of " << cpu_amount << "starting" << std::endl;

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


        MPI_Bcast(&pointsSize, sizeof (pointsSize), MPI_UNSIGNED, CPU_MASTER, MPI_COMM_WORLD);
        std::cout << "MASTER cpu#" << cpu_id << " sends pointsSize=" << pointsSize << " by bcast" << std::endl;


        MPI_Bcast(points, pointsSize * sizeof (Point), MPI_BYTE, CPU_MASTER, MPI_COMM_WORLD);
        std::cout << "MASTER cpu#" << cpu_id << " sends points by bcast: " << std::endl;

        for (unsigned int i = 0; i < pointsSize; ++i)
            std::cout << points[i] << " ";

        if (return_val != SUCCESS)
            return return_val;

    } else {

        MPI_Bcast(&pointsSize, sizeof (pointsSize), MPI_UNSIGNED, CPU_MASTER, MPI_COMM_WORLD);
        MPI_Comm_rank(MPI_COMM_WORLD, &cpu_id); // if I comment this then cpu_id is zero
        std::cout << "NOT MASTER cpu#" << cpu_id << " recieved pointsSize=" << pointsSize << " by bcast" << std::endl;

        points = new Point[pointsSize];



        MPI_Bcast(points, pointsSize * sizeof (Point), MPI_BYTE, CPU_MASTER, MPI_COMM_WORLD);
        MPI_Comm_rank(MPI_COMM_WORLD, &cpu_id); // if I comment this then cpu_id is zero
        std::cout << "NOT MASTER cpu#" << cpu_id << " recieved points by bcast: " << std::endl;

        for (unsigned int i = 0; i < pointsSize; ++i)
            std::cout << points[i] << " ";

    }


    std::cout << std::endl;

    std::ofstream output_stream;
    if (cpu_id == CPU_MASTER) {
        output_stream.open("vystup1.dat");
        for (unsigned int i = 0; i < pointsSize; i++) {
            output_stream << points[i].x << " " << points[i].y << std::endl;
        }
        output_stream.close();
    }

    permut(points);


    /*!
     * wait for all cpus to end their job after FINISHING message was sent
     */
    std::cerr << "===========cpu#" << cpu_id << " standing behind BARRIER==========" << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);

    unsigned int bestPrice;
    unsigned int myPrice = (unsigned) - 1;

    if (solution != NULL)
        myPrice = solution->getPrice();

    MPI_Reduce(&myPrice, &bestPrice, 1, MPI_UNSIGNED, MPI_MIN, CPU_MASTER, MPI_COMM_WORLD);


    MPI_Barrier(MPI_COMM_WORLD);

    if (cpu_id == CPU_MASTER) {
        std::cerr << "========= I am CPU_MASTER cpu#" << cpu_id << " best solution has " << bestPrice << " breaks ===========" << std::endl;
    }

    // send it back whole solution


    MPI_Bcast(&bestPrice, 1, MPI_UNSIGNED, CPU_MASTER, MPI_COMM_WORLD);
    MPI_Comm_rank(MPI_COMM_WORLD, &cpu_id); // if I comment this then cpu_id is zero

    if ((myPrice == bestPrice) && (cpu_id != CPU_MASTER)) // I found the best solution, but I am not master -> send solution to master
        sendState(solution, CPU_MASTER);

    if ((cpu_id == CPU_MASTER) && (bestPrice != myPrice)) { // if cpu master doesnt have best solution, then recieve at least one
        MPI_Status status;
        MPI_Recv(&buffer, bufferSize, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (solution != NULL)
            delete solution;

        solution = new State(buffer[LENGHT_POSITION], points, &buffer[BODY_POSITION]);
    }



    MPI_Finalize();


    if (cpu_id == CPU_MASTER) {
        output_stream.open("vystup2.dat");
        for (unsigned int i = 0; i < ((solution == NULL) ? 0 : solution->getSize()); i++) {
            output_stream << (points[solution->getIndex(i)]).x << " " << points[solution->getIndex(i)].y << std::endl;
        }
        output_stream.close();
        print(solution);

    }



    /*--------*/
    /*cleaning*/
    /*--------*/

    delete [] points;

    if (solution != NULL)
        delete solution;



    return (return_val);
}

