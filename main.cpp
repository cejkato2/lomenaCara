#include <stdio.h>
#include <iostream>
#include <fstream>
#include "types.h"
#include "dqueue.h"
#include "search.h"
#include <vector>

#define MIN_AMOUNT_POINTS 3

std::vector<Point *> points; // all readed points
int valMax = 0;

int read_points(FILE *f, DQueue<Point *> *q) {
    int amount;
    fscanf(f, "%i", &amount);
    if (amount < MIN_AMOUNT_POINTS) {
        fclose(f);
        std::cerr << "Amount of points has to be greater or equal " <<      \
      MIN_AMOUNT_POINTS << std::endl;
        return MIN_AMOUNT_ERR;
    }
    for (int i = 0; i < amount; ++i) {
        if (feof(f) != 0) {
            std::cerr << "Amount of inserted points is lesser than expected" <<      \
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

bool isElementLine(const Point *a, const Point *b, const Point *c) {

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
bool isOnSegment(const Point *a, const Point *b, const Point *c) {

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

int val(const Point *a, const Point *b, const std::vector<Point *> *in, std::vector<bool> *maskVector) {


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

void startRecursion(std::vector<int> *returnVector, std::vector<bool> *maskVector) {

    if (returnVector == NULL || maskVector == NULL) // ouch where I return my output?
        return;

    int indexPointA;
    int indexPointB;
    std::vector<bool> bestMaskVector;
    int valu = 0;



    // here I combine all possibilities how to construct a segment
    for (unsigned int i = 0; i < points.size() - 1; i++) {

        if (!maskVector->at(i))
            continue;

        for (unsigned int j = i + 1; j < points.size(); j++) {

            if (!maskVector->at(i))
                continue;

            std::vector<bool> myMaskVector = *maskVector;
            valu = val(points[i], points[j], &points, &myMaskVector);
            std::cerr << points[i] << " and " << points[j] << " has val = " << valu << std::endl;

            if (valu >= valMax) {
                indexPointA = i;
                indexPointB = j;
                valMax = valu;
                bestMaskVector = myMaskVector;
                bestMaskVector[i] = false; // this point determine segment and is used
                bestMaskVector[j] = false; // this point determine segment and is used
            }
        }
    }
    std::cerr << "best val is for " << points[indexPointA] << " and " << points[indexPointB] << std::endl;

    *maskVector = bestMaskVector;

    returnVector->push_back(indexPointA);
    returnVector->push_back(indexPointB);


    // here I count points which are not used
    int notUsed = 0;
    for (unsigned int u = 0; u < points.size(); u++) {
        if (maskVector->at(u)) {
            notUsed++;
            indexPointB = indexPointA;
            indexPointA = u;
        }

        if(notUsed>2)
            break;

    }

    switch (notUsed) {
        case 0: return;
        case 1:
        {
            returnVector->push_back(indexPointA);
            return;
        }
        case 2:
        {
            returnVector->push_back(indexPointA);
            returnVector->push_back(indexPointB);
            return;
        }
        default: startRecursion(returnVector, maskVector);
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


    // testing val function
    Point * a = new Point(3, 1);

    Point * b = new Point(1, 1);

    points = input->getData();
    std::vector<Point *>::iterator it;
    std::ofstream output_stream;
    output_stream.open("vystup1.dat");
    for (it=points.begin(); it!=points.end(); ++it) {
      output_stream << (*it)->x << " " << (*it)->y << std::endl;
    }
    output_stream.close();
      

    std::vector<bool> maskVector(points.size(), true);

    std::cerr << "Count of points on the line " << a << " " << b << " is " << val(a, b, &points, &maskVector) << std::endl;

    std::vector<int> brokenLine;
    std::vector<bool> mask(points.size(), true);
    startRecursion(&brokenLine, &mask);

    output_stream.open("vystup2.dat");
    for (unsigned int i = 0; i < brokenLine.size(); i++) {

        std::cerr << i + 1 << ". point of broken line is " << points[brokenLine[i]] << std::endl;
        output_stream << points[brokenLine[i]]->x << " " << points[brokenLine[i]]->y << std::endl;
    }
    output_stream.close();


    /*--------*/
    /*cleaning*/
    /*--------*/

    delete a;
    delete b;
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

