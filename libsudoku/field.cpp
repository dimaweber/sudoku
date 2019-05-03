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
    for(Cell::Ptr cell: cells)
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
        Cell::Ptr pCell = cells[idx];
        pCell->coord().setRawIndex(idx);
        pCell->resetCandidates(n);
        pCell->setDelay(false);
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
                cell(Coord(row,col))->setValue(v, true);
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
            cell(coord)->setValue(static_cast<CellValue>(symbol.digitValue()), true);
        if (symbol.isLetter())
        {
            QString s(symbol);
            quint8 v = s.toUInt(nullptr, 17);
            cell(coord)->setValue(static_cast<CellValue>(v), true);
        }
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
            Cell::Ptr c = cell(coord);
            c->registerInHouse(columns[coord.col()-1]);
            c->registerInHouse(rows[coord.row()-1]);
            c->registerInHouse(squares[coord.squareIdx()]);
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

Cell::Ptr Field::cell(const Coord& coord)
{
    return cells[coord.rawIndex()];
}

Cell::CPtr Field::cell(const Coord& coord) const
{
    return cells[coord.rawIndex()];
}

CellSet Field::allCellsVisibleFromCell(Cell::CPtr c)
{
    CellSet visibleCells;
    QVector<Coord> coords = c->coord().sameColumnCoordinates();
    for (const Coord& co: coords)
    {
        Cell::Ptr cellToAdd = cell(co);
        visibleCells.addCell(cellToAdd );
    }

    coords = c->coord().sameRowCoordinates();
    for (const Coord& co: coords)
    {
        Cell::Ptr cellToAdd = cell(co);
        if (!visibleCells.hasCell(cellToAdd))
            visibleCells.addCell(cellToAdd);
    }

    coords = c->coord().sameSquareCoordinates();
    for (const Coord& co: coords)
    {
        Cell::Ptr cellToAdd = cell(co);
        if (!visibleCells.hasCell(cellToAdd))
            visibleCells.addCell(cellToAdd);
    }

    return visibleCells;
}

CellSet Field::allCellsVisibleFromBothCell(Cell::CPtr c1, Cell::CPtr c2)
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
    for (Cell::CPtr cell: cells)
        if (!cell->isResolved())
            return true;
    return false;
}

bool Field::isValid() const
{
    for(House::CPtr area: areas)
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

bool Field::isResolved() const
{
    return !hasEmptyValues() && isValid();
}

