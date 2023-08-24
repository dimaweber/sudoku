#include "bilocationlink.h"
#include "cell.h"

BiLocationLink::BiLocationLink(int /*val*/, Cell::Ptr cell1, Cell::Ptr cell2)
    :/*candidate(val), */cell1(cell1), cell2(cell2)
{
    if(cell2->coord() < cell1->coord())
        std::swap(cell1, cell2);
}

bool BiLocationLink::operator ==(const BiLocationLink& link) const
{
    return  &link == this ||
            ((*link.first() == *first() && *link.second() == *second())
             || (*link.second() == *first() && *link.first() == *second()));
}
