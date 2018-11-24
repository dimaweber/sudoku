#include "house.h"
#include "cell.h"
#include <gsl/gsl_combination.h>
#include <iostream>
#include <QVector>

QVector<QBitArray> House::allCandidatesCombinationsMasks;
quint8 House::N = 0;

void House::init(quint8 n)
{
    N = n;
}

void Set::addCell(Cell* pCell)
{
    cells.append(pCell);
}

void House::fillCandidatesCombinationsMasks()
{
    allCandidatesCombinationsMasks.clear();
    gsl_combination* c = nullptr;
    size_t n = static_cast<size_t>(House::N);
    QBitArray testMask(n);
    for(size_t k=2; k<=n/2; k++)
    {
        c = gsl_combination_calloc(n, k);
        do
        {
            testMask.fill(false);
            size_t* comb_data = gsl_combination_data(c);
            for(size_t m = 0; m<gsl_combination_k(c); m++)
            {
                size_t bit = comb_data[m];
                testMask.setBit(bit);
            }
            allCandidatesCombinationsMasks.append(testMask);
        }while(gsl_combination_next(c) == GSL_SUCCESS);
        gsl_combination_free(c);
    }
}

House::House()
{
    if (House::allCandidatesCombinationsMasks.isEmpty())
        fillCandidatesCombinationsMasks();
    cells.reserve(House::N);
}

void House::reducePossibilities()
{
    for(int i=0; i<cells.size(); i++)
    {
        quint8 val = cells[i]->value();
        if (val > 0)
        {
            for (int j=0; j < cells.size(); j++)
                if (i!=j && !cells[j]->isResolved())
                    cells[j]->removeCandidate(val);
        }
    }
}

bool House::checkNakedCombinations()
{
    bool ret = false;
    for(const QBitArray& testMask: allCandidatesCombinationsMasks)
    {
            QVector<int> indices;
            for(int i=0;i<cells.count();i++)
                if (cells[i]->candidatesExactMatch(testMask) && !cells[i]->isResolved())
                    indices.append(i);
            if (indices.count() == testMask.count(true))
            {
                std::cout << "Naked combination " << testMask << " found in ";
                for (int i=0; i<indices.count();i++)
                    std::cout << cells[indices[i]]->coord();
                std::cout << std::endl;
                for(int i=0;i<cells.count();i++)
                    if (!indices.contains(i) && !cells[i]->isResolved())
                        ret |= cells[i]->removeCandidate(testMask);
            }
    }
    return ret;
}

bool House::checkHiddenCombinations()
{
    bool ret = false;
    for(const QBitArray& testMask: allCandidatesCombinationsMasks)
    {
        if (testMask.count(true) == unresolvedCellsCount())
            continue;
        QVector<int> indices;
        for(int i=0;i<cells.count();i++)
        {
            int candidatesInCell = cells[i]->hasAnyOfCandidates(testMask);
            if (candidatesInCell == 1)
            {
                indices.clear();
                break;
            }
            if (candidatesInCell > 1 && !cells[i]->isResolved())
                indices.append(i);
        }
        if (indices.count() == testMask.count(true))
        {
            std::cout << "Hidden combination " << testMask << " found in ";
            for (int i=0; i<indices.count();i++)
                std::cout << cells[indices[i]]->coord();
            std::cout << std::endl;
            for(int i: indices)
                ret |= cells[i]->removeCandidate(~testMask);
        }
    }
    return ret;
}

void Set::print()
{
    for (Cell* pCell: cells)
        pCell->print();
    std::cout << std::endl;
}

bool House::checkNakedSingle()
{
    bool newValueSet = false;
    for (int i=0; i<cells.size(); i++)
    {
        Cell& cell = *cells[i];
        if (!cell.isResolved() && cell.candidatesCount() == 1)
        {
            for (quint8 j=1;j<=cell.candidatesCapacity(); j++)
                if (cell.hasCandidate(j))
                {
                    newValueSet = true;
                    std::cout << "Naked single " << (int)j << " found in " << cell.coord()
                              << std::endl;
                    cell.setValue(j);
                }
        }
    }
    return newValueSet;
}

bool House::checkHiddenSingle()
{
    bool newValuesSet = false;
    QVector<quint8> valuesCount(House::N, 0);
    for (int i=0;i<cells.count(); i++)
    {
        if (!cells[i]->isResolved())
        {
            for(quint8 bit=1; bit <= cells[i]->candidatesCapacity(); bit++)
                if (cells[i]->hasCandidate(bit))
                    valuesCount[bit-1]++;
        }
    }
    for(int bit=1; bit <= valuesCount.count(); bit++)
    {
        if (valuesCount[bit-1] == 1)
        {
            for(Cell* pCell: cells)
            {
                if (pCell->hasCandidate(bit))
                {
                    std::cout << "Hidden single " << bit << " found in " << pCell->coord()
                              << std::endl;
                    pCell->setValue(bit);
                    newValuesSet = true;
                }
            }
        }
    }
    return newValuesSet;
}

bool Set::removeCandidate(quint8 val)
{
    bool ret = false;
    for (Cell* pCell: cells)
    {
        ret |= pCell->removeCandidate(val);
    }
    return ret;
}

int Set::unresolvedCellsCount() const
{
    int n;
    for(Cell* cell: cells)
        if (!cell->isResolved())
            n++;
    return n;
}

bool Set::hasValue(quint8 val) const
{
    for (const Cell* pCell: cells)
        if (pCell->value() == val)
            return true;
    return false;
}

int Set::candidatesCount(quint8 val) const
{
    int cnt=0;
    for (const Cell* pCell: cells)
        if (pCell->hasCandidate(val))
            cnt++;
    return cnt;
}

bool Set::hasEmptyValues() const
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

bool Set::hasCell(const Cell* p) const
{
    for(const Cell* pCell: cells)
        if (pCell->coord() == p->coord())
            return true;
    return false;
}

Set Set::operator+(const Set& a) const
{
    Set ret;
    for (Cell* cell: cells)
        ret.addCell(cell);
    for (Cell* cell: a.cells)
        if (!ret.cells.contains(cell))
            ret.addCell(cell);
    return ret;
}

Set Set::operator-(const Set& a) const
{
    Set ret;
    for(Cell* cell: cells)
        if (!a.cells.contains(cell))
            ret.addCell(cell);
    return ret;
}

Set Set::operator/(const Set& a) const
{
    Set ret;
    for(Cell* cell: cells)
        if (a.cells.contains(cell))
            ret.addCell(cell);
    return ret;
}
