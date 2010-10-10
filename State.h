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

class State {
public:
    /**
     * this class represents state in DFS algorithm
     * @param indexArraySizeArgument amount of points/indexes
     * @return
     */
    State(unsigned int indexArraySizeArgument);
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
     int setIndex(unsigned int value, unsigned int position);
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

    std::vector<bool> getIndexUsedVector();
   virtual int getPrice();
   friend std::ostream& operator<< (std::ostream &out, const State &myState){

     for(unsigned int i=0;i<myState.getSize();i++){
         out << myState.getIndex(i) << " ";
     }

     return out;

   }
private:

    unsigned int indexArraySize;
    unsigned int *indexArray;
    int price; // also count of breaks
};

#endif	/* _STATE_H */

