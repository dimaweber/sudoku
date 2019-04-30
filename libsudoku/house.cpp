#include "house.h"
#include "cell.h"
#include <iostream>
#include <QVector>

quint8 House::N = 0;

void House::init(quint8 n)
{
    N = n;
}

void CellSet::addCell(Cell* pCell)
{
    cells.append(pCell);
}

void CellSet::removeCell(Cell* pCell)
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
    return std::count_if(cells.begin(), cells.end(), [](const Cell* p)
        {return !p->isResolved();}
    );
}

bool CellSet::hasValue(CellValue val) const
{
    for (const Cell* pCell: cells)
        if (pCell->value() == val)
            return true;
    return false;
}

int CellSet::candidatesCount(CellValue val) const
{
    return std::count_if(cells.begin(), cells.end(), [val](const Cell* p)
        {return p->hasCandidate(val);}
    );
}

bool CellSet::hasEmptyValues() const
{
    for (const Cell* pCell: cells)
        if (!pCell->isResolved())
            return true;
    return  false;
}

bool House::isValid() const
{
    QBitArray mask(cells.count());
    for (const Cell* pCell: cells)
    {
        if (!pCell->isValid())
            return false;
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
    return isValid() && !hasEmptyValues();
}

bool CellSet::hasCell(const Cell* p) const
{
    for(const Cell* pCell: cells)
        if (pCell->coord() == p->coord())
            return true;
    return false;
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
    for (Cell* cell: cells)
        ret.addCell(cell);
    for (Cell* cell: a.cells)
        if (!ret.cells.contains(cell))
            ret.addCell(cell);
    return ret;
}

CellSet CellSet::operator-(const CellSet& a) const
{
    CellSet ret;
    for(Cell* cell: cells)
        if (!a.cells.contains(cell))
            ret.addCell(cell);
    return ret;
}

CellSet CellSet::operator/(const CellSet& a) const
{
    CellSet ret;
    for(Cell* cell: cells)
        if (a.cells.contains(cell))
            ret.addCell(cell);
    return ret;
}
