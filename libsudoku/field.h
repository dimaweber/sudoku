#ifndef FIELD_H
#define FIELD_H

#include "cell.h"
#include "house.h"
#include "technique.h"

#include <QVector>


class Field
{
    quint8 N;
    QVector<ColumnHouse> columns;
    QVector<RowHouse> rows;
    QVector<SquareHouse> squares;
    QVector<House*> areas;
    QVector<Cell*> cells;
public:
    Field();
    ~Field();

    quint8 getN() const {return N;}
    void setN(quint8 n);
    void prepareHouses(quint8 n);

    bool readFromFormattedTextFile(const QString& filename);
    bool readFromPlainTextFile(const QString& filename, int num);

    Cell& cell(const Coord& coord);
    const Cell& cell(const Coord& coord) const;

    CellSet allCellsVisibleFromCell(const Cell* c) ;
    CellSet allCellsVisibleFromBothCell(const Cell* c1, const Cell* c2);

    void print() const;

    bool isResolved() const;
    bool hasEmptyValues() const;
    bool isValid() const;

    quint8 columnCount() const;
    quint8 rowsCount() const;

private:
    friend class Technique;
};

#endif // FIELD_H
