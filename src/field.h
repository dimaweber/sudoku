#ifndef FIELD_H
#define FIELD_H

#include "cell.h"
#include "house.h"
#include "bilocationlink.h"
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

    enum SolvingTechnique {NakedSinge=0x0001,
                           HiddenSingle=0x0002,
                           NakedGroup=0x0004,
                           HiddenGroup=0x0008,
                           Intersections=0x000F,
                           XWing=0x0010,
                           BiLocationColoring=0x0020,
                           YWing=0x0040};

    void enableTechnique(SolvingTechnique tech, bool enabled=true);


    void setN(quint8 n);
    void prepareHouses(quint8 n);

    bool readFromFormattedTextFile(const QString& filename);
    bool readFromPlainTextFile(const QString& filename, int num);

    void process();

    Cell& cell(const Coord& coord);
    const Cell& cell(const Coord& coord) const;

    QVector<Cell*> allCellsVisibleFromCell(const Cell& cell);

    void print() const;

    bool isResolved() const;
    bool hasEmptyValues() const;
    bool isValid() const;

    quint8 columnCount() const;
    quint8 rowsCount() const;

private:
    quint32 enabledTechniques;

    bool findLinks();
    QVector<BiLocationLink> findBiLocationLinks(quint8 val) const;
    bool reduceIntersections();
    bool reduceIntersection(SquareHouse& square, LineHouse& area);
    bool reduceXWing();

};

#endif // FIELD_H
