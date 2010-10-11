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
unsigned int min_breaks = (unsigned int) -1;

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

    std::vector<State *> stack; // implicit stack
    std::vector<bool> mask(pointsSize,true); 

    // initialize stack, put first level of values

    for(unsigned int i=0;i<pointsSize;i++){
        State *state = new State(1, pointArray);
        state->setLastIndex(i);
        stack.push_back(state);
    }

    
    int lastPosition; //

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


    std::cout << "All permutations:" << std::endl;
    permut(points);

    output_stream.open("vystup2.dat");
    for (std::vector<Point>::iterator it = solution.begin(); it != solution.end(); ++it) {
        output_stream << it->x << " " << it->y << std::endl;
    }
    output_stream.close();
    print(solution); // not yet solution



    /*--------*/
    /*cleaning*/
    /*--------*/

    delete [] points;


    return (return_val);
}

