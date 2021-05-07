#ifndef BILOCATIONLINK_H
#define BILOCATIONLINK_H

#include <QtCore/qglobal.h>
#include "cell.h"

class BiLocationLink
{
    CellValue candidate {0};
    Cell* cell1   {nullptr};
    Cell* cell2   {nullptr};
public:
    BiLocationLink()= default;
    BiLocationLink(int val, Cell::Ptr cell1, Cell::Ptr cell2);

    Cell* first() const { return cell1;}
    Cell* second() const {return cell2;}

    bool operator == (const BiLocationLink& link) const;
};

#endif // BILOCATIONLINK_H
