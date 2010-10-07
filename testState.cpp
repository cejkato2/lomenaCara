/* 
 * File:   testState.cpp
 * Author: zaltair
 *
 * Created on 7. říjen 2010, 9:43
 */

#include <stdlib.h>
#include "State.h"
#include <iostream>

/*
 * 
 */
int main(int argc, char** argv) {

    State* stav = new State(5);
    stav->setIndex(1,0);
    stav->setIndex(2,1);
    stav->setIndex(3,2);
    stav->setIndex(4,3);


    for(int i = 0; i<5;i++){

        if(stav->getIndex(i)==i+1)
            std::cout << i << ".ok" << std::endl;
        else if(i==4 && stav->getIndex(i) == -2)
            std::cout << i << ".ok" << std::endl;
        else
            std::cout << "Something wrong on position " << i << " has index value" <<  stav->getIndex(i) << std::endl;
    }

    delete stav;

    return (EXIT_SUCCESS);
}

