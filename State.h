/* 
 * File:   State.h
 * Author: zaltair
 *
 * Created on 7. říjen 2010, 9:18
 */

#ifndef _STATE_H
#define	_STATE_H

#include <vector>
#include <ostream>
#include "types.h"

class State {
public:
    /**
     * this class represents state in DFS algorithm
     * @param indexArraySizeArgument amount of points/indexes
     * @param pointArray need to know for breaks calculation
     * @return
     */
    State(unsigned int indexArraySizeArgument, const Point *pointArray);
    /**
     * copy constructor, that creates copy which has size=sizeOriginal+1
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
     * return index value
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
     *
     * @return return total price(count of breaks)
     */

    /**
     * @return vector of indexes, using getIndex()
     */
    std::vector<int> getIndexes();

    /**
     * function that returns acctual count of breaks = price
     * @return
     */
   virtual unsigned int getPrice();

   /**
    * it is good to know if a price will be higher,
    * @param index index that will be added in the future
    * @return price of state if I add index
    */
   virtual unsigned int getExpandPrice(int index);
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
   const Point *pointArray;
    unsigned int indexArraySize;
    unsigned int *indexArray;
    unsigned int price; // also count of breaks
};

#endif	/* _STATE_H */

