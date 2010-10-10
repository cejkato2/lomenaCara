/* 
 * File:   State.cpp
 * Author: zaltair
 * 
 * Created on 7. říjen 2010, 9:18
 */

#include <stddef.h>

#include "State.h"

State::State(unsigned int indexArraySizeArgument) {

    indexArraySize=indexArraySizeArgument;
    indexArray = new unsigned int[indexArraySize];
}

State::State(const State& original){

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

int State::setIndex(unsigned int value, unsigned int position){

    if(position >= indexArraySize)
        return -1;

    indexArray[position] = value;
    return 0;


}

int State::getIndex(unsigned int position) const{
    if(position >= indexArraySize)
        return -1;

    return indexArray[position];

}

unsigned int State::getSize() const{
    return indexArraySize;
}


int State::getPrice(){
    return price;
}
