#include "field.h"

#include <QFile>
#include <QTextStream>
#include <iostream>
#include <QtMath>

Field::Field()
    :N(0), cells(0)
{
}

Field::~Field()
{
    for(Cell* cell: cells)
        cell->deleteLater();
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

void Resolver::process()
{
    bool changed = false;

    do
    {
        changed = false;

        for(Technique* tech: techniques)
        {
            changed = tech->perform();
            if (changed)
                break;
        }

        /*
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
        */

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

