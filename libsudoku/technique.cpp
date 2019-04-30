#include "technique.h"
#include "coord.h"
#include "cell.h"
#include "field.h"
#include "house.h"

#include <QVector>
#include <gsl/gsl_combination.h>

QVector<QBitArray> Technique::allCandidatesCombinationsMasks;

void Technique::fillCandidatesCombinationsMasks(size_t n)
{
    allCandidatesCombinationsMasks.clear();
    gsl_combination* c = nullptr;
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


NakedSingleTechnique::NakedSingleTechnique(Field& field)
    :Technique(field, "Naked Single")
{}

bool NakedSingleTechnique::run()
{
    bool changed = false;
    for(Coord coord = Coord::first(); coord.isValid(); coord++)
    {
        Cell& cell = field.cell(coord);
        if (!cell.isResolved() && cell.candidatesCount() == 1)
        {
            for (quint8 j=1;j<=cell.candidatesCapacity(); j++)
                if (cell.hasCandidate(j))
                {
                    changed = true;
                    std::cout << "Naked single " << (int)j << " found in " << cell.coord()
                              << std::endl;
                    cell.setValue(j);
                }
        }
    }
    return changed;
}

void NakedSingleTechnique::setEnabled(bool enabled)
{
    (void)enabled;
    Technique::setEnabled(true);
}

bool HiddenSingleTechnique::runPerHouse(House* house)
{
    bool newValueSet = false;
    QVector<quint8> valuesCount(N, 0);
    for (Cell* cell: *house)
    {
        if (!cell->isResolved())
        {
            for(quint8 bit=1; bit <= cell->candidatesCapacity(); bit++)
                if (cell->hasCandidate(bit))
                    valuesCount[bit-1]++;
        }
    }
    for(CellValue bit=1; bit <= valuesCount.count(); bit++)
    {
        if (valuesCount[bit-1] == 1)
        {
            for(Cell* pCell: *house)
            {
                if (pCell->hasCandidate(bit))
                {
                    std::cout << "Hidden single " << bit << " found in " << pCell->coord()
                              << std::endl;
                    pCell->setValue(bit);
                    newValueSet = true;
                }
            }
        }
    }
    return newValueSet;
}

HiddenSingleTechnique::HiddenSingleTechnique(Field& field)
    :PerHouseTechnique(field, "Hidden Single")
{

}

Technique::Technique(Field& field, const QString name)
    :enabled(true), techniqueName(name), field(field), N(field.getN())
{
    if (Technique::allCandidatesCombinationsMasks.isEmpty())
        fillCandidatesCombinationsMasks(N);
}

void Technique::setEnabled(bool enabled)
{
    this->enabled = enabled;
}

QVector<House*> Technique::areas()
{
    return field.areas;
}

QVector<SquareHouse> Technique::squares()
{
    return field.squares;
}

QVector<RowHouse> Technique::rows()
{
    return field.rows;
}

QVector<ColumnHouse> Technique::columns()
{
    return field.columns;
}

bool NakedGroupTechnique::runPerHouse(House *house)
{
    bool ret = false;
    for(const QBitArray& testMask: allCandidatesCombinationsMasks)
    {
            QVector<Cell*> indices;
            for(Cell* pCell: *house)
                if (pCell->candidatesExactMatch(testMask) && !pCell->isResolved())
                    indices.append(pCell);
            if (indices.count() == testMask.count(true) && house->unresolvedCellsCount() > indices.count())
            {
                std::cout << "Naked combination " << testMask << " found in ";
                for (Cell* pCell: indices)
                    std::cout << pCell->coord();
                std::cout << std::endl;
                for(Cell* pCell: *house)
                    if (!indices.contains(pCell) && !pCell->isResolved())
                        ret |= pCell->removeCandidate(testMask);
            }
    }
    return ret;
}

NakedGroupTechnique::NakedGroupTechnique(Field& field)
    :PerHouseTechnique(field, "Naked Group")
{

}


bool HiddenGroupTechnique::runPerHouse(House* house)
{
    bool ret = false;
    for(const QBitArray& testMask: allCandidatesCombinationsMasks)
    {
        if (testMask.count(true) == house->unresolvedCellsCount())
            continue;
        QVector<Cell*> indices;
        for(Cell* pCell: *house)
        {
            int candidatesInCell = pCell->hasAnyOfCandidates(testMask);
            if (candidatesInCell == 1)
            {
                indices.clear();
                break;
            }
            if (candidatesInCell > 1 && !pCell->isResolved())
                indices.append(pCell);
        }
        if (indices.count() == testMask.count(true))
        {
            std::cout << "Hidden combination " << testMask << " found in ";
            for (Cell* pCell: indices)
            {
                std::cout << pCell->coord();
                ret |= pCell->removeCandidate(~testMask);
            }
            std::cout << std::endl;
        }
    }
    return ret;

}

HiddenGroupTechnique::HiddenGroupTechnique(Field& field)
    :PerHouseTechnique(field, "Hidden Group")
{

}

bool PerHouseTechnique::run()
{
    bool newValuesSet = false;
    for(House* area: areas())
    {
        newValuesSet = runPerHouse(area);
    }
    return newValuesSet;
}

IntersectionsTechnique::IntersectionsTechnique(Field& field)
    :Technique (field, "Intersections")
{
}

bool IntersectionsTechnique::run()
{
    bool changed = false;
    for (SquareHouse& sq: squares())
    {
        for (LineHouse& r: rows())
            changed |= reduceIntersection(sq, r);
        for (LineHouse& c: columns())
            changed |= reduceIntersection(sq, c);
    }
    return changed;
}

bool IntersectionsTechnique::reduceIntersection(SquareHouse& square, LineHouse& area)
{
    bool changed = false;
    CellSet inter = square / area;
    CellSet squareNoLine = square - area;
    CellSet lineNoSquare = area - square;

    if (inter.isEmpty())
        return false;

    for (CellValue v = 1; v <= N; v++)
    {
        if (inter.candidatesCount(v) > 1)
        {
            if (squareNoLine.candidatesCount(v) == 0 && lineNoSquare.candidatesCount(v) != 0)
            {
                std::cout << (int)v << " found in " << qPrintable(square.name()) << " and " << qPrintable(area.name())
                          << " intersection but no in any other cell of " << qPrintable(square.name()) << std::endl;
                changed |= lineNoSquare.removeCandidate(v);
            }
            if (lineNoSquare.candidatesCount(v) == 0 && squareNoLine.candidatesCount(v) != 0)
            {
                std::cout << (int)v << " found in " << qPrintable(square.name()) << " and " << qPrintable(area.name())
                          << " intersection but no in any other cell of " << qPrintable(area.name()) << std::endl;
                changed |= squareNoLine.removeCandidate(v);
            }
        }
    }
    return changed;
}

