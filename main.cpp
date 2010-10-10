#include <stdio.h>
#include <iostream>
#include <fstream>
#include "types.h"
#include "State.h"
#include "search.h"
#include <vector>

#define MIN_AMOUNT_POINTS 3

Point* points = NULL; // all readed points
unsigned int pointsSize = 0; // size of points array
std::vector<Point> solution; // the solution will be stored here
int permu=0;

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

void print(std::vector<Point> &v) {
    std::vector<Point>::const_iterator it;
    for (it = v.begin(); it != v.end(); ++it) {
        std::cout << " " << *it << std::endl;
    }
    std::cout << std::endl;
}

/**
 * counts a count of breaks on the line defined by sequence of points defined by vector l
 * @param l sequence of points defining [broken] line
 * @return count of breaks
 */
int countBreaks(const std::vector<Point> &l) {

    if (l.size() < 3)
        return 0;

    int amount = 0;
    Point a,b,c;

    for (std::vector<Point>::const_iterator it = l.begin() + 2; it != l.end(); ++it) {

        a = *(it - 2); // first point
        b = *it; // second point
        c = *(it - 1); // the middle one

        // std::cout << "counting isOnSegment(" << a << ", "<< b << ", " << c << ")" << std::endl;
        if (!isOnSegment(&a, &b, &c))
            amount++;

    }
    return amount;
}

/**
 * counts a count of breaks on the line defined by sequence of points defined by vector l
 * @param l sequence of points defining [broken] line
 * @return count of breaks
 */
int countBreaks(const State *state) {

    if (state->getSize() < 3)
        return 0;

    int amount = 0;
    Point a,b,c;

    for (unsigned int i=2;i<state->getSize();i++) {

        a = points[state->getIndex(i-2)]; // first point
        b = points[state->getIndex(i)]; // second point
        c = points[state->getIndex(i-1)]; // the middle one

        // std::cout << "counting isOnSegment(" << a << ", "<< b << ", " << c << ")" << std::endl;
        if (!isOnSegment(&a, &b, &c))
            amount++;

    }
    return amount;
}


void permut(){

    std::vector<State *> stack;
    std::vector<bool> mask(pointsSize,true);

    // initialize stack

    for(unsigned int i=0;i<pointsSize;i++){
        State *state = new State(1);
        state->setIndex(i,0);
        stack.push_back(state);
    }

    
    int lastPosition;

    while(!stack.empty()){

        State *onStackState=stack.back();
        stack.pop_back();

        

        if(onStackState->getSize()==pointsSize) // if I am on a floor, I cannot expand
        {
          // write out the state for now
           // std::cout << "One of lists of DFS tree:  " << std::endl;
            std::cout << *onStackState << " breaks: " << countBreaks(onStackState) << std::endl;
            delete onStackState;
            permu++; // just a counter for verification if I wrote all permutations
            continue;

        }

        lastPosition=0;
        
        // set all mask variables to true
        for(unsigned int i=0;i<pointsSize;i++){
            mask[i]=true;
        }

        
        // set mask variables depend on used indexes
        for(unsigned int i=0;i<onStackState->getSize();i++){
            mask[onStackState->getIndex(i)]=false;
        }

        // here I expand my state to stack
        for(unsigned int i=0;i<pointsSize-onStackState->getSize();i++){
            State *expandedState = new State(*onStackState); // here I make a deep copy

            while(!mask[lastPosition]) // iterate to position where mask variable=true, thats the index which can be added to new state
                lastPosition++;

            expandedState->setIndex(lastPosition,expandedState->getSize()-1); // add a new part of state to last position
            lastPosition++;
            stack.push_back(expandedState);
        }
        delete onStackState;
        

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


    /*get points from stdin*/
    return_val = read_points(input_file);
    if (return_val != SUCCESS) {
        return return_val;
    }

    std::ofstream output_stream;
    output_stream.open("vystup1.dat");
    for (unsigned int i = 0; i < pointsSize; i++) {
        output_stream << points[i].x << " " << points[i].y << std::endl;
    }
    output_stream.close();

    for(unsigned int i=0;i<pointsSize;i++){
        solution.push_back(points[i]);
    }

    //std::cout << "Count of breaks on a line : "  << countBreaks(solution) << std::endl;
    print(solution); // not yet solution

    std::cout << "All permutations:" << std::endl;
    permut();

    output_stream.open("vystup2.dat");
    for (std::vector<Point>::iterator it = solution.begin(); it != solution.end(); ++it) {
        output_stream << it->x << " " << it->y << std::endl;
    }
    output_stream.close();


    std::cout << "count of all permutation :" << permu << std::endl;

    /*--------*/
    /*cleaning*/
    /*--------*/

    delete [] points;


    return (return_val);
}

