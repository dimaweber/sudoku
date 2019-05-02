#include "technique.h"
#include "coord.h"
#include "cell.h"
#include "field.h"
#include "house.h"
#include "cellcolor.h"

#include <QVector>
#include <QMap>
#include <QSet>

QSet<QBitArray> Technique::allCandidatesCombinationsMasks;

#ifdef Q_OS_WIN
void Technique::fillCandidatesCombinationsMasks(quint8 n)
{
    allCandidatesCombinationsMasks.clear();
    QSet<QBitArray> array;

    for (quint8 bit=0; bit<n-1; bit++)
        for (quint8 bit2=bit+1; bit2<n; bit2++)
        {
            QBitArray a(n);
            a.setBit(bit);
            a.setBit(bit2);
            array.insert(a);
        }

    for (size_t k=3; k<=n/2; k++)
    {
        QSet<QBitArray> b_array;
        for(const QBitArray& a: array)
        {
            for (quint8 bit=0; bit<n; bit++)
            {
                QBitArray b = a;
                b.setBit(bit);
                b_array.insert(b);
            }
        }
        array = b_array;
    }
    allCandidatesCombinationsMasks = array;
}
#else
#include <gsl/gsl_combination.h>


void Technique::fillCandidatesCombinationsMasks(quint8 n)
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
#endif

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
                    std::cout << "Hidden single " << (int)bit << " found in " << pCell->coord()
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

QVector<House*>& Technique::areas()
{
    return field.areas;
}

QVector<SquareHouse>& Technique::squares()
{
    return field.squares;
}

QVector<RowHouse>& Technique::rows()
{
    return field.rows;
}

QVector<ColumnHouse>& Technique::columns()
{
    return field.columns;
}

QVector<Cell*>& Technique::cells()
{
    return field.cells;
}

Cell& Technique::cell(const Coord& c)
{
    return field.cell(c);
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

BiLocationColoringTechnique::BiLocationColoringTechnique(Field& field)
    :Technique (field, "Bi-Location Coloring")
{

}

bool BiLocationColoringTechnique::run()
{
    bool changed = false;
    QMap<int, QVector<BiLocationLink>> links;
    for (CellValue i=1; i<=N; i++)
    {
        links[i] = findBiLocationLinks(i);
        ColoredLinksVault vault(i);
        for (BiLocationLink& link: links[i])
        {
            CellColor c1 = vault.getColor(link.first());
            CellColor c2 = vault.getColor(link.second());

            if (c1 == ColorPair::UnknownColor && c2 == ColorPair::UnknownColor)
            {
                ColorPair cp = ColorPair::newPair();
                vault.addLink(link, cp);
            }
            else
            {
                if (c1 == ColorPair::UnknownColor)
                    vault.addCell(link.first(), ColorPair::antiColor(c2));
                else if (c2 == ColorPair::UnknownColor)
                    vault.addCell(link.second(), ColorPair::antiColor(c1));
                else if (c1 != ColorPair::antiColor(c2))
                {
                    vault.recolor(ColorPair::antiColor(c2), c1);
                    vault.recolor(c2, ColorPair::antiColor(c1));
                }
                else
                {
                    // loop
                }
            }
        }
        for (BiLocationLink& link: links[i])
        {
            std::cout << i << "bi-location link: " << link.first()->coord() << vault.getColor(link.first())
                      << link.second()->coord() << vault.getColor(link.second()) <<std::endl;
        }
        for(House* house: areas())
        {
            // check for houses with 2 cells of same color
            CellSet cellsWithCandidate = house->cellsWithCandidate(i);
            QMap<CellColor, int> presentColor;
            for (Cell* cell: cellsWithCandidate)
            {
                CellColor color = vault.getColor(cell);
                if (color != ColorPair::UnknownColor)
                {
                    presentColor[color]++;
                    if (presentColor[color]>1)
                    {
                        // we've found house with 2 cells from same chain and same color
                        // this mean -- all cells with this color in this chain are OFF
                        std::cout << "two cells with same color in one house: this color is OFF" << std::endl;
                        changed |= vault.removeCandidateForColor(color);
                    }
                }
            }
        }
        for(Cell* c: cells())
        {
            if (!c->hasCandidate(i))
                continue;

            CellColor clr = vault.getColor(c);
            if (clr != ColorPair::UnknownColor)
                continue;

            QVector<CellColor> visibleColors;
            CellSet visibleCells = field.allCellsVisibleFromCell(c);
            for (Cell* pCell: visibleCells)
            {
                CellColor color = vault.getColor(pCell);
                if (color == ColorPair::UnknownColor)
                    continue;
                CellColor acolor = ColorPair::antiColor(color);
                if (visibleColors.contains(acolor))
                {
                    std::cout << "Non-colored cell " << c->coord() << " can see color " << color << " and its antiColor " << acolor << ": this cell is OFF" << std::endl;
                    changed |= c->removeCandidate(i);
                    break;
                }
                else
                    visibleColors.append(color);
            }
        }
    }

    return changed;
}

QVector<BiLocationLink> BiLocationColoringTechnique::findBiLocationLinks(CellValue val)
{
    QVector<BiLocationLink> ret;
    for(House* house: areas())
    {
        CellSet lst = house->cellsWithCandidate(val);
        if (lst.count() == 2)
        {
            BiLocationLink link(val, lst[0], lst[1]);
            if (!ret.contains(link))
                ret.append(link);
        }
    }
    return ret;
}

XWingTechnique::XWingTechnique(Field& field)
    :Technique(field, "X-Wing")
{

}

bool XWingTechnique::run()
{
    bool changed = false;

    for (quint8 colA_idx = 1; colA_idx <= columns().count()-1; colA_idx++)
    {
        ColumnHouse& columnA = columns()[colA_idx-1];
        for(quint8 colB_idx=colA_idx+1; colB_idx <= columns().count(); colB_idx++ )
        {
            ColumnHouse& columnB = columns()[colB_idx-1];
            for (quint8 row1_idx=1; row1_idx <= rows().count()-1; row1_idx++)
            {
                RowHouse& row1 = rows()[row1_idx-1];
                for (quint8 row2_idx=row1_idx+1; row2_idx <= rows().count(); row2_idx++)
                {
                    RowHouse& row2 = rows()[row2_idx-1];

                    QBitArray mask(N);
                    Coord A1(row1_idx, colA_idx);
                    Coord A2(row2_idx, colA_idx);
                    Coord B1(row1_idx, colB_idx);
                    Coord B2(row2_idx, colB_idx);

                    Cell& cA1 = cell(A1);
                    Cell& cA2 = cell(A2);
                    Cell& cB1 = cell(B1);
                    Cell& cB2 = cell(B2);

                    if (cA1.isResolved() || cA2.isResolved() || cB1.isResolved() || cB2.isResolved())
                        continue;

                    mask  = cA1.commonCandidates(cA2);
                    mask &= cB1.commonCandidates(cB2);

                    for (quint8 bit=0; bit < mask.count(); bit++)
                    {
                        CellValue value = bit+1;
                        if (!mask.testBit(bit))
                            continue;
                        if (columnA.candidatesCount(value) == 2 && columnB.candidatesCount(value) == 2
                                && (row1.candidatesCount(value) > 2 || row2.candidatesCount(value) > 2))
                        {
                            std::cout << "columns x-wing found for " << value << " in " << A1 << A2 << B1 << B2 << std::endl;
                            for (quint8 col=1; col <= columns().count(); col++)
                            {
                                if (col == colA_idx || col==colB_idx)
                                    continue;
                                Coord c1(row1_idx, col);
                                changed |= cell(c1).removeCandidate(value);
                                Coord c2(row2_idx, col);
                                changed |= cell(c2).removeCandidate(value);
                            }
                        }
                        if (row1.candidatesCount(value) == 2 && row2.candidatesCount(value) == 2
                                && (columnA.candidatesCount(value)>2 || columnB.candidatesCount(value)>2))
                        {
                            std::cout << "rows x-wing found for " << value << " in " << A1 << A2 << B1 << B2 << std::endl;
                            for (quint8 row=1; row <= rows().count(); row++)
                            {
                                if (row == row1_idx || row==row2_idx)
                                    continue;
                                Coord c1(row, colA_idx);
                                changed |= cell(c1).removeCandidate(value);
                                Coord c2(row, colB_idx);
                                changed |= cell(c2).removeCandidate(value);
                            }
                        }
                    }
                }
            }
        }
    }
    return changed;
}