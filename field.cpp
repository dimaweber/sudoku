#include "field.h"
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <QtMath>

Field::Field()
    :N(0), cells(0)
{
}
void Field::setN(quint8 n)
{
    N = n;
    cells.resize(n*n);
    Coord::init(n);
    House::init(n);

    for (quint16 idx=0; idx<n*n; idx++)
    {
        Cell& cell = cells[idx];
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
    quint8 n = lines.count();
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
                cell(Coord(row,col)).setValue(lines[row-1][col-1].toLatin1()-'0', true);
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
    quint8 n = qSqrt(line.count());
    setN(n);
    for(Coord coord = Coord::first(); coord.isValid(); coord++)
    {
        if (line[coord.rawIndex()].toLatin1() != '.' && line[coord.rawIndex()].toLatin1() != '0')
            cell(coord).setValue(line[coord.rawIndex()].toLatin1()-'0', true);
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
            changed |= pArea->checkNakedSingle();
            changed |= pArea->checkHiddenSingle();
            changed |= pArea->checkNakedCombinations();
            changed |= pArea->checkHiddenCombinations();
        }

        for (SquareHouse& sq: squares)
        {
            for (LineHouse& r: rows)
                changed |= reduceIntersection(sq, r);
            for (LineHouse& c: columns)
                changed |= reduceIntersection(sq, c);
        }

        reduceIntersections();
        reduceXWing();

    }while(changed);

    findLinks();
}

Cell& Field::cell(const Coord& coord)
{
    return cells[coord.rawIndex()];
}

const Cell& Field::cell(const Coord& coord) const
{
    return cells[coord.rawIndex()];
}

void Field::print() const
{
    for (quint8 row=1; row <= N; row ++)
    {
        rows[row-1].print();
    }
}

bool Field::hasEmptyValues() const
{
    for (const Cell& cell: cells)
        if (!cell.isResolved())
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
    return  columns.count();
}

quint8 Field::rowsCount() const
{
    return rows.count();
}

void Field::findLinks()
{
    QMap<int, QVector<BiLocationLink>> links;
    for (int i=1;i<=N;i++)
    {
        links[i] = findBiLocationLinks(i);
        for (BiLocationLink& link: links[i])
        {
            std::cout << i << "bi-location link: " << link.first()->coord() << link.second()->coord() << std::endl;
/*
            CellColor c1 = checkColoring(link.first());
            CellColor c2 = checkColoring(link.second());
            if (c1.isUnknown() && c2.isUnknown())
            {
                ColorPair cp = CellColor::newColorPair();
                addColoredLink(link, cp);
            }
            else
            {
                if (c1.isUnknown())
                    addColoredCell(link.first(), c2.antiColor());
                else if (c2.isUnknown())
                    addColoredCell(link.second(), c1.antiColor());
                else if (c1 != c2.antiColor())
                {
                    recolor(c2.antiColor(), c1);
                    recolor(c2, c1.antiColor());
                }
            }
*/
        }
    }
}

QVector<BiLocationLink> Field::findBiLocationLinks(quint8 val) const
{
    QVector<BiLocationLink> ret;
    for(House* house: areas)
    {
        QVector<Cell*> lst = house->cellsWithCandidate(val);
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
    Set inter = square / area;
    Set squareNoLine = square - area;
    Set lineNoSquare = area - square;

    if (inter.isEmpty())
        return false;

    for (quint8 v = 1; v < N; v++)
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

                    Cell& cA1 = cells[A1.rawIndex()];
                    Cell& cA2 = cells[A2.rawIndex()];
                    Cell& cB1 = cells[B1.rawIndex()];
                    Cell& cB2 = cells[B2.rawIndex()];

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
                                changed |= cells[c1.rawIndex()].removeCandidate(value);
                                Coord c2(row2_idx, col);
                                changed |= cells[c2.rawIndex()].removeCandidate(value);
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
                                changed |= cells[c1.rawIndex()].removeCandidate(value);
                                Coord c2(row, colB_idx);
                                changed |= cells[c2.rawIndex()].removeCandidate(value);
                            }
                        }
                    }
                }
    return changed;
}


bool Field::isResolved() const
{
    return isValid() && !hasEmptyValues();
}
