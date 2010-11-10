/* 
 * File:   State.h
 * Author: zaltair
 *
 * Created on 7. říjen 2010, 9:18
 */

#ifndef _STATE_H
#define	_STATE_H

#include <vector>
#include <list>
#include <iostream>
#include <ostream>
#include "types.h"

/**
 this class represents state in DFS algorithm
 */
class State {
public:
    /**
     * standard constructor that creates empty State
     * @param indexArraySizeArgument amount of points/indexes
     * @param pointArray need to know for breaks calculation
     * @return
     */
    State(unsigned int indexArraySizeArgument, const Point *pointArray);

    /*!
     * constructor which is used for reconstruction after serialization - from indexes
     * @param indexArraySizeArgument - amount of points
     * @param indexes - received indexes
     * @param pointArray - array of points that was distributed
     */

    State(unsigned int indexArraySizeArgument,  const Point *pointArray, int *indexes);

    /**
     * something like a copy constructor, that creates copy
     * which has size=sizeOriginal+1 because new expanded State is bigger.
     * @param copyFromMe state instance to copy
     * @return
     */
    State(const State& original);

    virtual ~State();
    /**
     * setUp value to position
     * @param value index
     * @param position position
     * @return sucessfull = 0
     */
     int setLastIndex(unsigned int value);
    /**
     * this function set last index of array
     * this function need the right pointArray, indexArraySize and indexArray, 
     * @param position position
     * @return value or -2 if value was not set yet, -1 if position is out of bounds
     */
    int getIndex(unsigned int position) const;
    /**
     *
     * @return size of index set
     */
   unsigned int getSize() const;
    /**
     * index array is used step by step
     * @return return last used postition
     */
    unsigned int getLastUsedPosition();

    /**
     * @return vector of indexes, using getIndex()
     */
    std::vector<int> getIndexes();

    /**
     * @return array of indexes
     */
    unsigned int *getArrayPointerIndexes();

    /**
     * function that returns acctual count of breaks = price
     * @return
     */
   virtual unsigned int getPrice();

   /**
    * it is good to know if a price will be higher, if the point pointsArray[index] will be stored
    * this function need the right pointArray, indexArraySize and indexArray,
    * @param index index that will be added in the future
    * @return price of state if I add index
    */
   virtual unsigned int getExpandPrice(int index);

   /**
    * function that expand state to stack, if state is a leaf of DFS tree then is stored as a solution
    * @param stack
    * @param solution
    * @param mask reusable variable
    * @param cpu_id
    * @param pointsSize size of pointsArray
    */
   void expand(std::list<State *> &stack, State *&solution, std::vector<bool> &mask, int cpu_id, unsigned int pointsSize);

   /**
    *  outputstream operator
    */
   friend std::ostream& operator<< (std::ostream &out, const State &myState){

     for(unsigned int i=0;i<myState.getSize();i++){
         out << myState.getIndex(i) << " ";
     }

     return out;

   }


 /**
 * are three different points a,b,c on a line ?
 * @param a first point
 * @param b second point
 * @param c third point
 * @return true if they are on a line, else false
 */

static inline bool isElementLine(const Point *a, const Point *b, const Point *c) {

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
 * @return true if c is on the same line with a,b and c is between a,b
 */
static inline bool isOnSegment(const Point *a, const Point *b, const Point *c) {

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

private:

    /**
     * function that recount price or count of breaks on a line defined via indexes
     * this function need the right pointArray, indexArraySize and indexArray,
     * price is stored and also returned
     * @return recounted price
     */
    unsigned int reCountPrice();

    /**
     * this is a pointer to array of Points, points are accesed via indexes
     * and used in computations of breaks
     */
   const Point *pointArray;

   /**
    * size of array indexes
    */
    unsigned int indexArraySize;

    /**
     * array of indexes
     */
    unsigned int *indexArray;

    /**
     * also known as count of breaks
     */
    unsigned int price; // 
};

#endif	/* _STATE_H */

