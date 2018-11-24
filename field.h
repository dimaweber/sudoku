#ifndef FIELD_H
#define FIELD_H

#include "cell.h"
#include "house.h"
#include <QVector>

class Field
{
    quint8 N;
    QVector<ColumnHouse> columns;
    QVector<RowHouse> rows;
    QVector<SquareHouse> squares;
    QVector<House*> areas;
    QVector<Cell> cells;

public:
    Field();

    void setN(quint8 n);
    void prepareHouses(quint8 n);

    bool readFromFormattedTextFile(const QString& filename);
    bool readFromPlainTextFile(const QString& filename, int num);

    void process();

    Cell& cell(const Coord& coord);
    const Cell& cell(const Coord& coord) const;

    void print() const;

    bool isResolved() const;
    bool hasEmptyValues() const;
    bool isValid() const;

    quint8 columnCount() const;
    quint8 rowsCount() const;

private:
    bool reduceIntersections();
    bool reduceIntersection(SquareHouse& square, LineHouse& area);
    bool reduceXWing();
};

#endif // FIELD_H
