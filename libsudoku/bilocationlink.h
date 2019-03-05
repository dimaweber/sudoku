#ifndef BILOCATIONLINK_H
#define BILOCATIONLINK_H

#include <QtCore/qglobal.h>
class Cell;

class BiLocationLink
{
    int candidate;
    Cell* cell1;
    Cell* cell2;
public:
    BiLocationLink():candidate(0),cell1(nullptr),cell2(nullptr){}
    BiLocationLink(int val, Cell* cell1, Cell* cell2);

    Cell* first() const { return cell1;}
    Cell* second() const {return cell2;}

    bool operator == (const BiLocationLink& link) const;
};

#endif // BILOCATIONLINK_H
