#ifndef FIELD_H
#define FIELD_H

#include "cell.h"
#include "house.h"
#include "technique.h"

#include <QVector>


class Field
{
    quint8 N{0};
    QVector<ColumnHouse> columns;
    QVector<RowHouse> rows;
    QVector<SquareHouse> squares;
    QVector<House::Ptr> areas;
    QVector<Cell::Ptr> cells{nullptr};
public:
    Field() = default;
    ~Field();

    quint8 getN() const {return N;}
    void setN(quint8 n);
    void prepareHouses(quint8 n);

    bool readFromFormattedTextFile(const QString& filename);
    bool readFromPlainTextFile(const QString& filename, int num);

    Cell::Ptr  cell(const Coord& coord);
    Cell::CPtr cell(const Coord& coord) const;

    CellSet allCellsVisibleFromCell(Cell::CPtr c) ;
    CellSet allCellsVisibleFromBothCell(Cell::CPtr c1, Cell::CPtr c2);
    QVector<House::Ptr> commonHouses(Cell::CPtr c1, Cell::CPtr c2);

    void print(std::ostream& stream) const;

    bool isResolved() const;
    bool hasEmptyValues() const;
    bool isValid() const;

    quint8 columnCount() const;
    quint8 rowsCount() const;

private:
    friend class Technique;
};

std::ostream& operator << (std::ostream& stream, const Field& field);

#endif // FIELD_H
