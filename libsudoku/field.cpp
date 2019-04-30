#include "field.h"
#include "cellcolor.h"

#include <QFile>
#include <QTextStream>
#include <iostream>
#include <QtMath>

Field::Field()
    :N(0), cells(0),enabledTechniques(0xFFFF)
{
}

Field::~Field()
{
    for(Cell* cell: cells)
        cell->deleteLater();
}

void Field::enableTechnique(Field::SolvingTechnique tech, bool enabled)
{
    if (enabled)
        enabledTechniques |= tech;
    else
        enabledTechniques &= ~tech;
}

void Field::setN(quint8 n)
{
    N = n;
    cells.resize(n*n);
    Coord::init(n);
    House::init(n);

    for (quint16 idx=0; idx<n*n; idx++)
    {
        if (!cells[idx])
            cells[idx] = new Cell(n);
        Cell& cell = *cells[idx];
        cell.coord().setRawIndex(idx);
        cell.resetCandidates(n);
    }

    prepareHouses(n);
}

bool Field::readFromFormattedTextFile(const QString& filename)
{
    QFile inputFile(filename);
    if (!inputFile.open(QFile::ReadOnly))
    {
        std::cerr << "unable to open input file" << std::endl;
        return false;
    }
    QTextStream stream(&inputFile);
    QStringList lines;
    while(!stream.atEnd())
    {
        lines.append(stream.readLine());
    }
    quint8 n = static_cast<quint8>(lines.count());
    setN(n);
    for(quint8 row=1;row <= n; row++)
    {
        if (lines[row-1].length() != n)
        {
            std::cerr << "wrong input file line: " << qPrintable(lines[row-1]) << std::endl;
            return false;
        }
        for(quint8 col=1; col<= n; col++)
        {
            if (lines[row-1][col-1]!= '.')
            {
                CellValue v = static_cast<CellValue>(lines[row-1][col-1].digitValue());
                cell(Coord(row,col)).setValue(v, true);
            }
        }
    }
    return true;
}

bool Field::readFromPlainTextFile(const QString& filename, int num)
{
    QFile inputFile(filename);
    if (!inputFile.open(QFile::ReadOnly))
    {
        std::cerr << "unable to open input file" << std::endl;
        return false;
    }
    QTextStream stream(&inputFile);
    QStringList lines;
    do
    {
        QString line =  stream.readLine();
        line = line.simplified();
        if (!line.startsWith('#'))
            lines.append(line);
    }while(!stream.atEnd());
    QString line = lines.at(qMin(num, lines.count()));
    quint8 n = static_cast<quint8>(qSqrt(line.count()));
    setN(n);
    for(Coord coord = Coord::first(); coord.isValid(); coord++)
    {
        QChar symbol = line[coord.rawIndex()];
        if (symbol.isDigit() && symbol.toLatin1() != '0')
            cell(coord).setValue(static_cast<CellValue>(symbol.digitValue()), true);
    }
    return true;
}

void Field::prepareHouses(quint8 n)
{
    areas.clear();

    columns.clear();
    rows.clear();
    squares.clear();

    columns.resize(n);
    rows.resize(n);
    squares.resize(n);

    for (Coord coord = Coord::first(); coord.isValid(); coord++)
    {
            Cell& c = cell(coord);
            c.registerInHouse(columns[coord.col()-1]);
            c.registerInHouse(rows[coord.row()-1]);
            c.registerInHouse(squares[coord.squareIdx()]);
    }

    for (int i=1; i<=n; i++)
    {
        columns[i-1].setName(QString("C%1").arg(i));
        rows[i-1].setName(QString("R%1").arg(i));
        squares[i-1].setName(QString("S%1").arg(i));

        areas.append(&columns[i-1]);
        areas.append(&rows[i-1]);
        areas.append(&squares[i-1]);
    }
}

void Field::process()
{
    bool changed = false;

    do
    {
        changed = false;

        for(House* pArea: areas)
        {
            if (enabledTechniques & NakedSingle || true) // always enabled
                changed |= pArea->checkNakedSingle();
            if (changed)  continue;

            if (enabledTechniques & HiddenSingle)
                changed |= pArea->checkHiddenSingle();
            if (changed) continue;

            if (enabledTechniques & NakedGroup)
                changed |= pArea->checkNakedCombinations();
            if (changed) continue;

            if (enabledTechniques & HiddenGroup)
                changed |= pArea->checkHiddenCombinations();
            if (changed) continue;
        }

        if (enabledTechniques & Intersections)
            changed |= reduceIntersections();
        if (changed) continue;

        if (enabledTechniques & XWing)
            changed |= reduceXWing();
        if (changed) continue;

        if (enabledTechniques & BiLocationColoring)
            changed |= findLinks();
        if (changed) continue;

        if (enabledTechniques & YWing)
            changed |= reduceYWing();
        if (changed) continue;

        if (enabledTechniques & XYZWing)
            changed |= reduceXYZWing();
        if (changed) continue;

    }while(changed);
}

Cell& Field::cell(const Coord& coord)
{
    return *cells[coord.rawIndex()];
}

const Cell& Field::cell(const Coord& coord) const
{
    return *cells[coord.rawIndex()];
}

CellSet Field::allCellsVisibleFromCell(const Cell* c)
{
    CellSet visibleCells;
    QVector<Coord> coords = c->coord().sameColumnCoordinates();
    for (const Coord& co: coords)
    {
        Cell* cellToAdd = &cell(co);
        visibleCells.addCell(cellToAdd );
    }

    coords = c->coord().sameRowCoordinates();
    for (const Coord& co: coords)
    {
        Cell* cellToAdd = &cell(co);
        if (!visibleCells.hasCell(cellToAdd))
            visibleCells.addCell(cellToAdd);
    }

    coords = c->coord().sameSquareCoordinates();
    for (const Coord& co: coords)
    {
        Cell* cellToAdd = &cell(co);
        if (!visibleCells.hasCell(cellToAdd))
            visibleCells.addCell(cellToAdd);
    }

    return visibleCells;
}

CellSet Field::allCellsVisibleFromBothCell(const Cell* c1, const Cell* c2)
{
    CellSet vis1 = allCellsVisibleFromCell(c1);
    CellSet vis2 = allCellsVisibleFromCell(c2);

    return vis1 / vis2;
}

void Field::print() const
{
    std::cout << " C: ";
    for (int col=1; col<=N;col++)
        std::cout << col;
    std::cout <<std::endl;
    std::cout << "    ";
    for (int i=0; i<N; i++)
        std::cout <<".";
    std::cout << std::endl;
    for (quint8 row=1; row <= N; row ++)
    {
        rows[row-1].print();
    }
}

bool Field::hasEmptyValues() const
{
    for (const Cell* cell: cells)
        if (!cell->isResolved())
            return true;
    return false;
}

bool Field::isValid() const
{
    for(const House* area: areas)
    {
        if (!area->isValid())
            return false;
    }
    return true;
}

quint8 Field::columnCount() const
{
    return  static_cast<quint8>(columns.count());
}

quint8 Field::rowsCount() const
{
    return static_cast<quint8>(rows.count());
}

bool Field::findLinks()
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
        for(House* house: areas)
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
        for(Cell* c: cells)
        {
            if (!c->hasCandidate(i))
                continue;

            CellColor clr = vault.getColor(c);
            if (clr != ColorPair::UnknownColor)
                continue;

            QVector<CellColor> visibleColors;
            CellSet visibleCells = allCellsVisibleFromCell(c);
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

QVector<BiLocationLink> Field::findBiLocationLinks(CellValue val) const
{
    QVector<BiLocationLink> ret;
    for(House* house: areas)
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

bool Field::reduceIntersections()
{
    bool changed = false;
    for (SquareHouse& sq: squares)
    {
        for (LineHouse& r: rows)
            changed |= reduceIntersection(sq, r);
        for (LineHouse& c: columns)
            changed |= reduceIntersection(sq, c);
    }
    return changed;
}

bool Field::reduceIntersection(SquareHouse& square, LineHouse& area)
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

bool Field::reduceXWing()
{
    bool changed = false;

    for (int colA_idx = 1; colA_idx <= columns.count()-1; colA_idx++)
        for(int colB_idx=colA_idx+1; colB_idx <= columns.count(); colB_idx++ )
            for (int row1_idx=1; row1_idx <= rows.count()-1; row1_idx++)
                for (int row2_idx=row1_idx+1; row2_idx <= rows.count(); row2_idx++)
                {
                    QBitArray mask(Field::N);
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
                        int value = bit+1;
                        if (!mask.testBit(bit))
                            continue;
                        if (columns[colA_idx-1].candidatesCount(value) == 2 && columns[colB_idx-1].candidatesCount(value) == 2
                                && (rows[row1_idx-1].candidatesCount(value) > 2 || rows[row2_idx-1].candidatesCount(value) > 2))
                        {
                            std::cout << "columns x-wing found for " << value << " in " << A1 << A2 << B1 << B2 << std::endl;
                            for (int col=1; col <= columns.count(); col++)
                            {
                                if (col == colA_idx || col==colB_idx)
                                    continue;
                                Coord c1(row1_idx, col);
                                changed |= cell(c1).removeCandidate(value);
                                Coord c2(row2_idx, col);
                                changed |= cell(c2).removeCandidate(value);
                            }
                        }
                        if (rows[row1_idx-1].candidatesCount(value) == 2 && rows[row2_idx-1].candidatesCount(value) == 2
                                && (columns[colA_idx-1].candidatesCount(value)>2 || columns[colB_idx-1].candidatesCount(value)>2))
                        {
                            std::cout << "rows x-wing found for " << value << " in " << A1 << A2 << B1 << B2 << std::endl;
                            for (int row=1; row <= rows.count(); row++)
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
    return changed;
}

bool Field::reduceYWing()
{
    bool ret = false;

    for (Cell* cellAB: cells)
    {
        if (cellAB->candidatesCount() != 2)
            continue;

        QVector<CellValue> candidates = cellAB->candidates();
        CellValue A = candidates[0];
        CellValue B = candidates[1];

        CellSet visibleFromAB = allCellsVisibleFromCell(cellAB);
        CellSet biValueCellsVisibleFromAB;
        for(Cell* c: visibleFromAB)
            if (c->candidatesCount() == 2)
                biValueCellsVisibleFromAB.addCell(c);


        for(CellValue C=1; C<=N; C++)
        {
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
                    CellSet visibleFromBoth = allCellsVisibleFromBothCell(ac, bc);
                    ret |= visibleFromBoth.removeCandidate(C);
                }
        }
    }

    return ret;
}

bool Field::reduceXYZWing()
{
    bool ret = false;

    for (Cell* xyzcell: cells)
    {
        if (xyzcell->candidatesCount() != 3)
            continue;

        QVector<CellValue> xyzvalues = xyzcell->candidates();

        CellValue v1 = xyzvalues[0];
        CellValue v2 = xyzvalues[1];
        CellValue v3 = xyzvalues[2];

        QVector<Coord> squareCoords = xyzcell->coord().sameSquareCoordinates();
        for(const Coord& xz_co: squareCoords)
        {
            Cell& sq_cell = cell(xz_co);
            if (sq_cell.candidatesCount() == 2 &&
                xyzcell->commonCandidates(sq_cell).count(true) == 2)
            {
                Cell* xzcell = &sq_cell;
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
                    Cell& row_cell = cell(yz_co);
                    if (   row_cell.candidatesCount()==2
                        && xyzcell->commonCandidates(row_cell).count(true)==2
                        && row_cell.hasCandidate(y))
                    {
                        Cell* yzcell = &row_cell;
                        CellValue z;
                        if ( yzcell->hasCandidate(v1) && y != v1)
                            z = v1;
                        else if (yzcell->hasCandidate(v2) && y != v2)
                            z = v2;
                        else
                            z = v3;

                        std::cout << "XYZ-Wing found with apex " << xyzcell->coord()
                                  << " and wings " << xzcell->coord()
                                  << " / " << yzcell->coord()
                                  << " Z is " << (int)z << std::endl;

                        for (const Coord& co: yz_co.sameRowCoordinates())
                        {
                            if (co.squareIdx() == xyzcell->coord().squareIdx()
                                    && co != xyzcell->coord())
                                ret |= cell(co).removeCandidate(z);
                        }
                    }
                }

                QVector<Coord> colCoords = xyzcell->coord().sameColumnCoordinates();
                for (const Coord& yz_co: colCoords)
                {
                    Cell& col_cell = cell(yz_co);
                    if (   col_cell.candidatesCount()==2
                        && xyzcell->commonCandidates(col_cell).count(true)==2
                        && col_cell.hasCandidate(y))
                    {
                        Cell* yzcell = &col_cell;
                        CellValue z;
                        if ( yzcell->hasCandidate(v1) && y != v1)
                            z = v1;
                        else if (yzcell->hasCandidate(v2) && y != v2)
                            z = v2;
                        else
                            z = v3;

                        std::cout << "XYZ-Wing found with apex " << xyzcell->coord()
                                  << " and wings " << xzcell->coord()
                                  << " / " << yzcell->coord()
                                  << " Z is " << (int)z << std::endl;

                        for (const Coord& co: yz_co.sameColumnCoordinates())
                        {
                            if (co.squareIdx() == xyzcell->coord().squareIdx()
                                    && co != xyzcell->coord())
                                ret |= cell(co).removeCandidate(z);
                        }
                    }
                }

            }
        }
    }

    return ret;
}


bool Field::isResolved() const
{
    return isValid() && !hasEmptyValues();
}