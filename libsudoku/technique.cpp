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
            allCandidatesCombinationsMasks.insert(testMask);
        }while(gsl_combination_next(c) == GSL_SUCCESS);
        gsl_combination_free(c);
    }
}
#endif

Technique::Technique(Field& field, const QString& name, QObject *parent)
    :QObject(parent), enabled(true), techniqueName(name), field(field), N(field.getN())
{
    if (Technique::allCandidatesCombinationsMasks.isEmpty())
        fillCandidatesCombinationsMasks(N);
}

Technique::~Technique()
= default;

void Technique::setEnabled(bool enabled)
{
    this->enabled = enabled;
}

bool Technique::perform()
{
    if (!enabled)
        return false;
    emit started();
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 100ms;
//        std::this_thread::sleep_until (t0);
    }
    bool res = run();
    if (res)
        emit applied();
    else
        emit done();
    return res;
}

QVector<House::Ptr>& Technique::areas()
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

QVector<Cell::Ptr>& Technique::cells()
{
    return field.cells;
}

Cell::Ptr Technique::cell(const Coord& c)
{
    return field.cell(c);
}

NakedSingleTechnique::NakedSingleTechnique(Field& field, QObject *parent)
    :PerCellTechnique(field, "Naked Single", parent)
{}

bool NakedSingleTechnique::runPerCell(Cell::Ptr pCell)
{
    bool changed = false;
    if (!pCell->isResolved() && pCell->candidatesCount() == 1)
    {
        for (CellValue j=1;j<=pCell->candidatesCapacity(); j++)
            if (pCell->hasCandidate(j))
            {
                std::cout << "Naked single " << (int)j << " found in " << pCell->coord()
                          << std::endl;
                pCell->setValue(j);
                changed = true;
                break;
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
    for(CellValue bit=1; bit <= valuesCount.count() && !newValueSet; bit++)
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
                    break;
                }
            }
        }
    }
    return newValueSet;
}

HiddenSingleTechnique::HiddenSingleTechnique(Field& field, QObject* parent)
    :PerHouseTechnique(field, "Hidden Single", parent)
{

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
            if (ret)
                break;
        }
    }
    return ret;
}

NakedGroupTechnique::NakedGroupTechnique(Field& field, QObject* parent)
    :PerHouseTechnique(field, "Naked Group", parent)
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
            if (ret)
                return true;
        }
    }
    return ret;

}

HiddenGroupTechnique::HiddenGroupTechnique(Field& field, QObject* parent)
    :PerHouseTechnique(field, "Hidden Group", parent)
{

}

#define MT
#ifdef MT
#  include <QtConcurrent>
#endif
bool PerHouseTechnique::run()
{
#ifdef MT
    return QtConcurrent::blockingFilteredReduced<bool>(areas(), [this](House* area)
    {
        return runPerHouse(area);
    },
    [](bool& result, const bool& intermediate)
    {
        result |= intermediate;
    });
#else
    bool newValuesSet = false;
    for(House* area: areas())
    {
        newValuesSet = runPerHouse(area);
        if (newValuesSet)
            break;
    }
    return newValuesSet;
#endif
}

IntersectionsTechnique::IntersectionsTechnique(Field& field, QObject* parent)
    :Technique (field, "Intersections", parent)
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

BiLocationColoringTechnique::BiLocationColoringTechnique(Field& field, QObject* parent)
    :Technique (field, "Bi-Location Coloring", parent)
{

}

bool BiLocationColoringTechnique::run()
{
    bool changed = false;
    for (CellValue i=1; i<=N; i++)
    {
        QVector<BiLocationLink> links;
        links = findBiLocationLinks(i);
        ColoredLinksVault vault(i);
        for (BiLocationLink& link: links)
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
        for (BiLocationLink& link: links)
        {
            std::cout << (int)i << "bi-location link: " << link.first()->coord() << vault.getColor(link.first())
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

XWingTechnique::XWingTechnique(Field& field, QObject* parent)
    :Technique(field, "X-Wing", parent)
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

                    Cell::Ptr cA1 = cell(A1);
                    Cell::Ptr cA2 = cell(A2);
                    Cell::Ptr cB1 = cell(B1);
                    Cell::Ptr cB2 = cell(B2);

                    if (cA1->isResolved() || cA2->isResolved() || cB1->isResolved() || cB2->isResolved())
                        continue;

                    mask  = cA1->commonCandidates(cA2);
                    mask &= cB1->commonCandidates(cB2);

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
                                changed |= cell(c1)->removeCandidate(value);
                                Coord c2(row2_idx, col);
                                changed |= cell(c2)->removeCandidate(value);
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
                                changed |= cell(c1)->removeCandidate(value);
                                Coord c2(row, colB_idx);
                                changed |= cell(c2)->removeCandidate(value);
                            }
                        }
                    }
                }
            }
        }
    }
    return changed;
}

YWingTechnique::YWingTechnique(Field &field, QObject* parent)
    :PerCellTechnique(field, "Y-Wing", parent)
{

}

bool YWingTechnique::runPerCell(Cell::Ptr cellAB)
{
    bool ret = false;

    if (cellAB->candidatesCount() != 2)
        return ret;

    QVector<CellValue> candidates = cellAB->candidates();
    CellValue A = candidates[0];
    CellValue B = candidates[1];

    CellSet visibleFromAB = field.allCellsVisibleFromCell(cellAB);
    CellSet biValueCellsVisibleFromAB;
    for(Cell* c: visibleFromAB)
        if (c->candidatesCount() == 2)
            biValueCellsVisibleFromAB.addCell(c);


    for(CellValue C=1; C<=N; C++)
    {
        // this can be paralleled for every C
        if(A == C || B == C)
            continue;

        CellSet cellsAC;
        CellSet cellsBC;

        for (Cell* c: biValueCellsVisibleFromAB)
        {
            if (c->hasCandidate(C) && c->hasCandidate(A))
                cellsAC.addCell(c);
            if (c->hasCandidate(C) && c->hasCandidate(B))
                cellsBC.addCell(c);
        }

        for (Cell* ac: cellsAC)
            for(Cell* bc: cellsBC)
            {
                std::cout << "Y-Wing found: " << cellAB->coord() << " " << ac->coord() << " " << bc->coord() << std::endl;
                CellSet visibleFromBoth = field.allCellsVisibleFromBothCell(ac, bc);
                ret |= visibleFromBoth.removeCandidate(C);
            }
    }

    return ret;

}

XYZWingTechnique::XYZWingTechnique(Field &field, QObject *parent)
    :PerCellTechnique (field, "XYZ-Wing", parent)
{

}

bool XYZWingTechnique::runPerCell(Cell::Ptr xyzcell)
{
    bool ret = false;

    if (xyzcell->candidatesCount() != 3)
        return ret;

    QVector<CellValue> xyzvalues = xyzcell->candidates();

    CellValue v1 = xyzvalues[0];
    CellValue v2 = xyzvalues[1];
    CellValue v3 = xyzvalues[2];

    QVector<Coord> squareCoords = xyzcell->coord().sameSquareCoordinates();
    for(const Coord& xz_co: squareCoords)
    {
        Cell::Ptr sq_cell = cell(xz_co);
        if (sq_cell->candidatesCount() == 2 &&
            xyzcell->commonCandidatesCount(sq_cell) == 2)
        {
            Cell::Ptr xzcell = sq_cell;
            CellValue y;
            if (!xzcell->hasCandidate(v1))
                y = v1;
            else if (!xzcell->hasCandidate(v2))
                y = v2;
            else
                y = v3;

            QVector<Coord> rowCoords = xyzcell->coord().sameRowCoordinates();
            for (const Coord& yz_co: rowCoords)
            {
                Cell::Ptr row_cell = cell(yz_co);
                if (   row_cell->candidatesCount()==2
                    && xyzcell->commonCandidatesCount(row_cell)==2
                    && row_cell->hasCandidate(y))
                {
                    Cell::Ptr yzcell = row_cell;
                    CellValue z;
                    if ( yzcell->hasCandidate(v1) && y != v1)
                        z = v1;
                    else if (yzcell->hasCandidate(v2) && y != v2)
                        z = v2;
                    else
                        z = v3;

                    if (xz_co.row() == yz_co.row())
                        continue;

                    std::cout << "XYZ-Wing found with apex " << xyzcell->coord()
                              << " and wings " << xzcell->coord()
                              << " / " << yzcell->coord()
                              << " Z is " << (int)z << std::endl;

                    for (const Coord& co: yz_co.sameRowCoordinates())
                    {
                        if (co.squareIdx() == xyzcell->coord().squareIdx()
                                && co != xyzcell->coord())
                            ret |= cell(co)->removeCandidate(z);
                    }
                }
            }

            QVector<Coord> colCoords = xyzcell->coord().sameColumnCoordinates();
            for (const Coord& yz_co: colCoords)
            {
                Cell::Ptr col_cell = cell(yz_co);
                if (   col_cell->candidatesCount()==2
                    && xyzcell->commonCandidatesCount(col_cell)==2
                    && col_cell->hasCandidate(y))
                {
                    Cell::Ptr yzcell = col_cell;
                    CellValue z;
                    if ( yzcell->hasCandidate(v1) && y != v1)
                        z = v1;
                    else if (yzcell->hasCandidate(v2) && y != v2)
                        z = v2;
                    else
                        z = v3;

                    if (xz_co.col() == yz_co.col())
                        continue;

                    std::cout << "XYZ-Wing found with apex " << xyzcell->coord()
                              << " and wings " << xzcell->coord()
                              << " / " << yzcell->coord()
                              << " Z is " << (int)z << std::endl;

                    for (const Coord& co: yz_co.sameColumnCoordinates())
                    {
                        if (co.squareIdx() == xyzcell->coord().squareIdx()
                                && co != xyzcell->coord())
                            ret |= cell(co)->removeCandidate(z);
                    }
                }
            }

        }
    }

    return ret;
}


bool PerCellTechnique::run()
{
    bool ret = false;
    for (Cell::Ptr pCell: cells())
    {
        emit cellAnalyzeStarted(pCell);
        ret |= runPerCell(pCell);
        emit cellAnalyzeFinished(pCell);
        if (ret)
            break;
    }
    return ret;
}

std::ostream& operator << (std::ostream& stream, UniqueRectangle::Rectangle& r)
{
    stream << r.cell->coord() << r.sameRowCell->coord() << r.sameColumnCell->coord() << r.diagonalCell->coord();
    return stream;
}

bool UniqueRectangle::Rectangle::applyType1Check()
{
    if (   sameRowCell->candidatesExactMatch(cell)
        && sameColumnCell->candidatesExactMatch(cell))
    {

        QBitArray commonCandidates = diagonalCell->commonCandidates(cell);
        if (commonCandidates.count(true) == 2)
        {
            std::cout << "Unique Rectangle Type 1" << *this << std::endl;
            return diagonalCell->removeCandidate(commonCandidates);
        }
    }
    return  false;
}

bool UniqueRectangle::Rectangle::applyType2aCheck()
{
    if ( cell->candidatesExactMatch(neigborCell) &&
         cell->commonCandidates(diagonalCell).count(true) == 2 &&
         diagNeigborCell->candidatesExactMatch(diagonalCell) &&
         diagonalCell->candidatesCount() == 3)
    {
        std::cout << "Unique Rectangle Type 2A" << *this << std::endl;
        QVector<CellValue> cellValues = cell->candidates();
        QVector<CellValue> diagValues = diagonalCell->candidates();
        for (CellValue v: cellValues)
            diagValues.removeAll(v);
        CellValue candidateToRemove = diagValues[0];
        return field.allCellsVisibleFromBothCell(diagonalCell, diagNeigborCell).removeCandidate(candidateToRemove);
    }
    return false;
}

bool UniqueRectangle::Rectangle::applyType2bCheck()
{
    if (cell->candidatesExactMatch(diagNeigborCell) &&
        cell->commonCandidates(neigborCell).count(true) == 2 &&
        neigborCell->candidatesExactMatch(diagonalCell) &&
        neigborCell->candidatesCount() == 3)
    {
        std::cout << "Unique Rectangle Type 2B" << *this << std::endl;
        QVector<CellValue> cellValues = cell->candidates();
        QVector<CellValue> neigValues = neigborCell->candidates();
        for (CellValue v: cellValues)
            neigValues.removeAll(v);
        CellValue candidateToRemove = neigValues[0];
        return field.allCellsVisibleFromBothCell(neigborCell, diagonalCell).removeCandidate(candidateToRemove);
    }
    return false;
}

bool UniqueRectangle::Rectangle::applyType2cCheck()
{
    if (cell->candidatesExactMatch(diagonalCell) &&
        cell->commonCandidates(neigborCell).count(true) == 2 &&
        neigborCell->candidatesExactMatch(diagNeigborCell) &&
        neigborCell->candidatesCount() == 3
        )
    {
        std::cout << "Unique Rectangle Type 2C" << *this << std::endl;
        QVector<CellValue> cellValues = cell->candidates();
        QVector<CellValue> neigValues = neigborCell->candidates();
        for (CellValue v: cellValues)
            neigValues.removeAll(v);
        CellValue candidateToRemove = neigValues[0];
        return field.allCellsVisibleFromBothCell(neigborCell, diagNeigborCell).removeCandidate(candidateToRemove);
    }
    return false;
}

bool UniqueRectangle::Rectangle::applyType3aCheck()
{
    bool ret = false;
    if (cell->candidatesExactMatch(diagNeigborCell) &&
        cell->commonCandidates(neigborCell).count(true) == 2 &&
        cell->commonCandidates(diagonalCell).count(true) == 2 &&
        neigborCell->candidatesCount() == 3 &&
        diagonalCell->candidatesCount() == 3 &&
        !diagonalCell->candidatesExactMatch(neigborCell)
       )
    {
        std::cout << "Unique rectangle type 3A" << *this << std::endl;
        QVector<CellValue> vals1 = diagonalCell->candidates();
        QVector<CellValue> vals2 = neigborCell->candidates();
        QVector<CellValue> baseValues = cell->candidates();
        for (CellValue v: baseValues)
        {
            vals1.removeAll(v);
            vals2.removeAll(v);
        }
        std::cout << "virtual cell values from roof are " << (int)vals1[0] << " " << (int)vals2[0] <<std::endl;
        QBitArray virtualCellCandidates(cell->candidatesCapacity());
        virtualCellCandidates.setBit(vals1[0]-1);
        virtualCellCandidates.setBit(vals2[0]-1);
        Cell* pair = nullptr;
        for (auto c: field.allCellsVisibleFromBothCell(diagonalCell, neigborCell))
        {
            if (c->candidatesExactMatch(virtualCellCandidates))
            {
                std::cout << "pair found" << c->coord() << std::endl;
                pair = c;
                break;
            }
        }
        if (pair)
        {
            for (auto c: field.allCellsVisibleFromBothCell(diagonalCell, neigborCell))
            {
                if (c != pair)
                {
                    ret |= c->removeCandidate(virtualCellCandidates);
                }
            }
        }
    }
    return ret;
}

bool UniqueRectangle::Rectangle::applyType3bCheck()
{
    bool ret = false;
    if (cell->candidatesExactMatch(neigborCell) &&
        cell->commonCandidates(diagonalCell).count(true) == 2 &&
        cell->commonCandidates(diagNeigborCell).count(true) == 2 &&
        diagNeigborCell->candidatesCount() == 3 &&
        diagonalCell->candidatesCount() == 3 &&
        !diagonalCell->candidatesExactMatch(diagNeigborCell)
       )
    {
        std::cout << "Unique rectangle type 3B" << *this << std::endl;
        QVector<CellValue> vals1 = diagonalCell->candidates();
        QVector<CellValue> vals2 = diagNeigborCell->candidates();
        QVector<CellValue> baseValues = cell->candidates();
        for (CellValue v: baseValues)
        {
            vals1.removeAll(v);
            vals2.removeAll(v);
        }
        std::cout << "virtual cell values from roof are " << (int)vals1[0] << " " << (int)vals2[0] <<std::endl;
        QBitArray virtualCellCandidates(cell->candidatesCapacity());
        virtualCellCandidates.setBit(vals1[0]-1);
        virtualCellCandidates.setBit(vals2[0]-1);
        for (House::CPtr hs: field.commonHouses(diagonalCell, diagNeigborCell))
        {
            Cell* pair = nullptr;
            for (auto c: *hs)
            {
                if (c == diagonalCell || c == diagNeigborCell)
                    continue;
                if (c->candidatesExactMatch(virtualCellCandidates))
                {
                    std::cout << "pair found" << c->coord() << std::endl;
                    pair = c;
                    break;
                }
            }
            if (pair)
            {
                for (auto c: *hs)
                {
                    if (c != pair && c != diagonalCell && c != diagNeigborCell)
                    {
                        ret |= c->removeCandidate(virtualCellCandidates);
                    }
                }
            }
        }
    }
    return ret;
}

UniqueRectangle::UniqueRectangle(Field& field, QObject* parent)
    :PerCellTechnique (field, "Unique Rectangle", parent)
{

}

bool UniqueRectangle::runPerCell(Cell::Ptr pCell)
{
    bool ret = false;
    if (pCell->candidatesCount() != 2)
        return false;
    RowHouse&    row = rows()[pCell->coord().row()-1];
    ColumnHouse& col = columns()[pCell->coord().col()-1];
    Rectangle rect(field);
    rect.cell = pCell;

    for(Cell::Ptr cellInSameRow: row)
    {
        if (cellInSameRow == pCell)
            continue;
        if (cellInSameRow->isResolved())
            continue;
        rect.sameRowCell = cellInSameRow;

        for (Cell::Ptr cellInSameColumn: col)
        {
            if (cellInSameColumn == pCell)
                continue;
            if (cellInSameColumn->isResolved())
                continue;
            rect.sameColumnCell = cellInSameColumn;

            rect.diagonalCell = cell(Coord(cellInSameColumn->coord().row(), cellInSameRow->coord().col()));
            if (rect.diagonalCell->isResolved())
                continue;
            if (rect.diagonalCell->coord().squareIdx() == pCell->coord().squareIdx())
                continue;

            if (cellInSameRow->coord().squareIdx() == pCell->coord().squareIdx())
            {
                rect.neigborCell = cellInSameRow;
                rect.diagNeigborCell = cellInSameColumn;
            }
            else if (cellInSameColumn->coord().squareIdx() == pCell->coord().squareIdx())
            {
                rect.neigborCell = cellInSameColumn;
                rect.diagNeigborCell = cellInSameRow;
            }
            else
                continue;

            ret |= rect.applyType1Check();
            ret |= rect.applyType2aCheck();
            ret |= rect.applyType2bCheck();
            ret |= rect.applyType2cCheck();
            ret |= rect.applyType3aCheck();
            ret |= rect.applyType3bCheck();
        }
    }
    return ret;
}
