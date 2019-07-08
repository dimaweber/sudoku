#include "house.h"
#include "cell.h"
#include <iostream>
#include <QVector>

quint8 House::N = 0;

void House::init(quint8 n)
{
    N = n;
}

void CellSet::addCell(Cell::Ptr pCell)
{
    cells.append(pCell);
}

void CellSet::removeCell(Cell::Ptr pCell)
{
    if (cells.contains(pCell))
        cells.removeOne(pCell);
}


House::House()
{
    cells.reserve(House::N);
}

void CellSet::print() const
{
    std::cout << qPrintable(name()) << ": ";
    for (Cell* pCell: cells)
        pCell->print();
    std::cout << std::endl;
}


bool CellSet::removeCandidate(CellValue val)
{
    bool ret = false;
    for (Cell* pCell: cells)
    {
        ret |= pCell->removeCandidate(val);
    }
    return ret;
}

int CellSet::unresolvedCellsCount() const
{
    return std::count_if(begin(), end(), [](Cell::CPtr p)
    {
        return !p->isResolved();
    });
}

bool CellSet::hasValue(CellValue val) const
{
    return std::any_of(cells.begin(), cells.end(), [val](Cell::Ptr pCell)
    {
        return pCell->value() == val;
    });
}

int CellSet::candidatesCount(CellValue val) const
{
    return std::count_if(begin(), end(), [val](Cell::CPtr p)
    {
        return p->hasCandidate(val);
    });
}

bool CellSet::hasUnresolvedCells() const
{
    return std::any_of(begin(), end(), [](Cell::Ptr pCell)
    {
       return !pCell->isResolved();
    });
}

bool House::isValid() const
{
    QBitArray mask(cells.count());
    for ( Cell::CPtr pCell: cells)
    {
//        if (!pCell->isValid())
//            return false; /// TODO: this check fails when reading from file since value might no be set yet and only 1 variant left
        if (!pCell->isResolved())
            continue;
        quint8 bit = pCell->value()-1;
        if (mask.testBit(bit))
            return false;
        mask.setBit(bit, true);
    }
    return true;
}

bool House::isResolved() const
{
    return isValid() && !hasUnresolvedCells();
}

bool CellSet::hasCell(Cell::CPtr p) const
{
    return std::any_of(begin(), end(), [p](Cell::Ptr pCell)
    {
       return *pCell == *p;
    });
}

CellSet CellSet::cellsWithCandidate(CellValue val) const
{
    CellSet ret;
    for(Cell* cell: cells)
        if (cell->hasCandidate(val))
                ret.addCell(cell);
    return ret;
}

CellSet CellSet::operator+(const CellSet& a) const
{
    CellSet ret;
    for (Cell::Ptr cell: cells)
        ret.addCell(cell);
    for (Cell::Ptr cell: a.cells)
        if (!ret.cells.contains(cell))
            ret.addCell(cell);
    return ret;
}

CellSet CellSet::operator-(const CellSet& a) const
{
    CellSet ret;
    for(Cell::Ptr cell: cells)
        if (!a.cells.contains(cell))
            ret.addCell(cell);
    return ret;
}

CellSet CellSet::operator/(const CellSet& a) const
{
    CellSet ret;
    for(Cell::Ptr cell: cells)
        if (a.cells.contains(cell))
            ret.addCell(cell);
    return ret;
}
