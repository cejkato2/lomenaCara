#include <stdio.h>
#include <iostream>
#include <fstream>
#include "types.h"
#include "dqueue.h"
#include "search.h"
#include <vector>

#define MIN_AMOUNT_POINTS 3

std::vector<Point *> points; // all readed points

int read_points(FILE *f, DQueue<Point *> *q) {
    int amount;
    fscanf(f, "%i", &amount);
    if (amount < MIN_AMOUNT_POINTS) {
        fclose(f);
        std::cerr << "Amount of points has to be greater or equal " <<         \
      MIN_AMOUNT_POINTS << std::endl;
        return MIN_AMOUNT_ERR;
    }
    for (int i = 0; i < amount; ++i) {
        if (feof(f) != 0) {
            std::cerr << "Amount of inserted points is lesser than expected" <<        \
        std::endl;
            return LESS_AMOUNT_ERR;
        }
        Point *p = new Point();
        fscanf(f, "%i %i", &p->x, &p->y);
        q->addHead(p);
    }

    fclose(f);
    return SUCCESS;
}

void print_result(FILE *f, DQueue<Point *> *q) {

    DQueue<Point *>::Node *p = q->getHeadNode();
    while (p != NULL) {
        fprintf(f, "%i %i\n", p->data->x, p->data->y);
        p = p->next;
    }
}

/*
 * is the point c on the line which is defined by points a and b ?
 */

inline bool isElementLine(const Point *a, const Point *b, const Point *c) {

    if (*a == *c || *b == *c)
        return false;

    int det = (a->x * b->y + a->y * c->x + b->x * c->y) - (b->y * c->x + c->y * a->x + a->y * b->x);

    if (det == 0)
        return true;
    else return false;
}

/*
 * is the point c on the segment between points a and b ?
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

/*
 *
 */

inline int val(const Point *a, const Point *b, const std::vector<Point *> *in, std::vector<bool> *maskVector) {





    if (maskVector->size() != in->size()) // ouch something wrong
        return -1;

    if (*a == *b)
        return 0;

    int count = 0;
    Point *c = NULL;

    for (unsigned int i = 0; i < in->size(); i++) {

        if (!maskVector->at(i)) // am I able to work with it ?
            continue;

        c = in->at(i);
        if (isOnSegment(a, b, c)) { //X-----*-----X//
            (*maskVector)[i] = false; // ok this point is marked as used now
            count++;
        }
    }

    return count;

}

void calculate(std::vector<int> &returnVector, std::vector<bool> *globalMask) {

    bool finished = false;
    while(!finished){


    if (globalMask == NULL || points.size() != globalMask->size()) // ouch where I return my output?
        return;

    int indexPointA;
    int indexPointB;
    std::vector<bool>* bestMaskVector = NULL; // actual mask vector in this step of recursion
    std::vector<bool>* tempMask = NULL;
    int valu = 0;
    int valMax = -1;



    // here I combine all possibilities how to construct a segment
    for (unsigned int i = 0; i < points.size() - 1; i++) {

        if (!globalMask->at(i)) // can I work with this point ?
            continue;

        for (unsigned int j = i + 1; j < points.size(); j++) {

            if (!globalMask->at(i)) // can I work with this point ?
                continue;

            tempMask = new std::vector<bool>(*globalMask); // here I make a deep copy of globalMask
            valu = val(points[i], points[j], &points, tempMask); // now I count a val function
          //  std::cerr << points[i] << " and " << points[j] << " has val = " << valu << std::endl;

            if (valu > valMax) {

                indexPointA = i;
                indexPointB = j;
                valMax = valu;

                if (bestMaskVector != NULL)
                    delete bestMaskVector;

                bestMaskVector = tempMask;
                (*bestMaskVector)[i] = false; // this point determine segment and is used
                (*bestMaskVector)[j] = false; // this point determine segment and is used

            } else {
                delete tempMask;
            }
        }
    }
    std::cerr << "best val is for " << points[indexPointA] << " and " << points[indexPointB] << " ,valMax = "<< valMax << std::endl;


    if (bestMaskVector != NULL) {
        delete globalMask;
        globalMask = bestMaskVector;
    }

    returnVector.push_back(indexPointA);
    returnVector.push_back(indexPointB);


    if (valMax == 0) // there are no points on segments at all
    {
        unsigned int count = 0;
        for (unsigned int u = 0; u < points.size(); u++) {
            if (globalMask->at(u)) {
                returnVector.push_back(u);
                count++;
            }
        }
        std::cout << "there were " << count << " points out of any segment" << std::endl;
        delete globalMask;
       // return;
        finished = true;
        continue;
    }



    // here I count points which are not used
    int notUsed = 0;
    for (unsigned int u = 0; u < points.size(); u++) {
        if (globalMask->at(u)) {
            notUsed++;
            indexPointB = indexPointA;
            indexPointA = u;
        }

        if (notUsed > 2)
            break;

    }



    switch (notUsed) {
        case 0: {
            //return;
            finished = true;
            break;
             }
        case 1:
        {
            returnVector.push_back(indexPointA);
            delete globalMask;
            //return;
            finished = true;
            break;
            
        }
        case 2:
        {
            returnVector.push_back(indexPointA);
            returnVector.push_back(indexPointB);
            delete globalMask;
           // return;
            finished = true;
            break;
        }
        default:
        {
          //  startRecursion(returnVector, globalMask);
          //  return;

        }
    }


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

    DQueue<Point *> *input, *backup;

    /*init of input queue*/
    input = new DQueue<Point *>();

    /*get points from stdin*/
    return_val = read_points(input_file, input);
    if (return_val == SUCCESS) {
        std::cerr << input;
        /* backup points */
        backup = input->copy();
        //
        //
        //        /*finding algorithm*/
        //        result = find_line(input);

        //        print_result(stdout, result);
    } else {
        delete input;
        return return_val;
    }



    points = input->getData();
    std::vector<Point *>::iterator it;
    std::ofstream output_stream;
    output_stream.open("vystup1.dat");
    for (it = points.begin(); it != points.end(); ++it) {
        output_stream << (*it)->x << " " << (*it)->y << std::endl;
    }
    output_stream.close();



    /*--------*/
    /*cleaning*/
    /*--------*/


    delete input;

    if (return_val == SUCCESS) {
        //        delete result;
        if (backup != NULL) {
            while (!backup->isEmpty()) {
                delete backup->getHead();
                backup->popHead();
            }
            delete backup;
        }
    }

    return (return_val);
}

