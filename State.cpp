/* 
 * File:   State.cpp
 * Author: zaltair
 * 
 * Created on 7. říjen 2010, 9:18
 */

#include <stddef.h>
#include <vector>

#include "State.h"

State::State(int indexArraySizeArgument) {

    indexArraySize=indexArraySizeArgument;
    indexArray = new int[indexArraySize];
    bitMaskArray = std::vector<bool>(indexArraySize, true);
}


State::~State() {

    bitMaskArray.clear();
    delete [] indexArray;
    indexArray = NULL;
}

int State::setIndex(int value, int position){

    if(position < 0 || position >= indexArraySize || value < 0 || value >= indexArraySize)
        return -1;

    indexArray[position] = value;
    bitMaskArray[position] = false;

    return 0;


}

int State::getIndex(int position){
    if(position < 0 || position >= indexArraySize)
        return -1;

    if(bitMaskArray[position]) // not set yet
        return -2;

    return indexArray[position];

}

