/* 
 * File:   State.h
 * Author: zaltair
 *
 * Created on 7. říjen 2010, 9:18
 */

#ifndef _STATE_H
#define	_STATE_H

#include <vector>

class State {
public:
    State(int indexArraySizeArgument);
    virtual ~State();
    int setIndex(int value, int position);
    int getIndex(int position);
private:

    int indexArraySize;
    int *indexArray;
    std::vector<bool> bitMaskArray;
};

#endif	/* _STATE_H */

