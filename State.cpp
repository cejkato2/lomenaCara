/* 
 * File:   State.cpp
 * Author: zaltair
 * 
 * Created on 7. říjen 2010, 9:18
 */

#include <stddef.h>

#include "State.h"

State::State(unsigned int indexArraySizeArgument, const Point *pointArrayArgument) {

    pointArray=pointArrayArgument; // swallow copy
    indexArraySize=indexArraySizeArgument;
    indexArray = new unsigned int[indexArraySize];
    price=0; // it is new empty state, price should be zero
}

State::State(unsigned int indexArraySizeArgument, const Point *pointArrayArgument, int *indexes) {
    pointArray = pointArrayArgument;
    indexArraySize=indexArraySizeArgument;
    indexArray = new unsigned int[indexArraySize];
    for (unsigned int i=0; i<indexArraySizeArgument; ++i) {
      indexArray[i] = indexes[i];
    } //copy points according received indexes

    reCountPrice();

}

State::State(const State& original){
    
    pointArray=original.pointArray; // swallow copy
    indexArraySize=original.indexArraySize+1;
    indexArray = new unsigned int[indexArraySize];

    for(unsigned int i=0; i<indexArraySize-1;i++){ // -1 because new array is bigger
        indexArray[i]=original.indexArray[i];
    }

    price=original.price;
}


State::~State() {

    delete [] indexArray;
    indexArray = NULL;
}

unsigned int *State::getArrayPointerIndexes()
{ 
  return indexArray;
}

int State::setLastIndex(unsigned int value){

    indexArray[indexArraySize-1] = value; // setting up last value

    if(indexArraySize < 3) // only 2, then doing nothing
        return 0;

    // counting breaks
                     // pre-pre last point             last point                       pre-last point
    if(!isOnSegment( &pointArray[indexArray[indexArraySize-3]]  , &pointArray[indexArray[indexArraySize-1]], &pointArray[indexArray[indexArraySize-2]] ))
        price++;

    return 0;


}

int State::getIndex(unsigned int position) const{
    if(position >= indexArraySize)
        return -1;

    return indexArray[position];

}

std::vector<int> State::getIndexes() {
  std::vector<int> set(indexArraySize);
  for (unsigned int i=0; i<indexArraySize; ++i) {
    set[i] = indexArray[i];
  }
  return set;
}

unsigned int State::getSize() const{
    return indexArraySize;
}


unsigned int State::getPrice(){
    return price;
}

unsigned int State::getExpandPrice(int index){

 if(indexArraySize < 2) // i know only 1, if any point is added then there cannot be a break
        return 0;

    // counting breaks
                     // pre-pre last point                          last point                                   pre-last point
    if(!isOnSegment( &pointArray[indexArray[indexArraySize-2]]  , &pointArray[index], &pointArray[indexArray[indexArraySize-1]] ))
        return price+1; // if I add this point then price will be higher

    return price;

}

unsigned int State::reCountPrice(){

 if(indexArraySize < 3) // i know only 2, then there is no break
        return 0;

    price = 0;
    Point a,b,c;

    for(unsigned int i=2; i<indexArraySize; i++){

        a = pointArray[indexArray[i-2]]; // first point
        b = pointArray[indexArray[i]]; // second point
        c = pointArray[indexArray[i-1]]; // the middle one

        // std::cout << "counting isOnSegment(" << a << ", "<< b << ", " << c << ")" << std::endl;
        if (!isOnSegment(&a, &b, &c)) // is c on line between a and b
            price++;

    }

    return price;

}

void State::expand(std::list<State*> &stack, State *&solution, std::vector<bool> &mask, int cpu_id, unsigned int pointsSize){


          if(this->getSize()==pointsSize) // if I am on a floor, I cannot expand
          {

              if ((solution == NULL) || (this->getPrice() < solution->getPrice())) { // is parentState better then acctual solution ?

                std::cout << "cpu#" << cpu_id << " prev min_breaks " << ((solution==NULL)? 0 : solution->getPrice());
                
                std::cout << " my price=" << this->getPrice() << std::endl;

                if(solution != NULL)
                    delete solution; // because I have got new better solution

                solution = this; // here I store the new solution

                std::cout << " and new solution price=" << solution->getPrice() << std::endl;
              }else{
                delete this; // if parentState is not a best solution
              }

              return ;

          }

          int lastPosition=0;

          // set all mask variables to true
          for(unsigned int i=0;i<pointsSize;i++){
              mask[i]=true;
          }


          // set mask variables depend on used indexes
          for(unsigned int i=0;i<this->getSize();i++){
              mask[this->getIndex(i)]=false;
          }

          // here I expand my state to stack
          for(unsigned int i=0;i<pointsSize-this->getSize();i++){

              while(!mask[lastPosition]) // iterate to position where mask variable=true, thats the index which can be added to new state
                  lastPosition++;

              if(solution != NULL && ( this->getExpandPrice(lastPosition) >= solution->getPrice() )) // if I cannot get better solution = bounding
              {
                  lastPosition++;
                  continue;
              }

              State *childState = new State(*this); // here I make a deep copy
              childState->setLastIndex(lastPosition); // add a new part of state
              lastPosition++;
              stack.push_back(childState);
          }
          delete this; // now I can delete parent state

          return ;

}