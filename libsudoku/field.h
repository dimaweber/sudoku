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
    QVector<Cell*> cells;

public:
    Field();
    ~Field();

    enum SolvingTechnique {NakedSinge=0x0001,
                           HiddenSingle=0x0002,
                           NakedGroup=0x0004,
                           HiddenGroup=0x0008,
                           Intersections=0x0010,
                           XWing=0x0020,
                           BiLocationColoring=0x0040,
                           YWing=0x0100,
                           XYZWing = 0x0200};

    void enableTechnique(SolvingTechnique tech, bool enabled=true);


    void setN(quint8 n);
    void prepareHouses(quint8 n);

    bool readFromFormattedTextFile(const QString& filename);
    bool readFromPlainTextFile(const QString& filename, int num);

    void process();

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
    quint32 enabledTechniques;

    bool findLinks();
    QVector<BiLocationLink> findBiLocationLinks(CellValue val) const;
    bool reduceIntersections();
    bool reduceIntersection(SquareHouse& square, LineHouse& area);
    bool reduceXWing();
    bool reduceYWing();
    bool reduceXYZWing();

};

#endif // FIELD_H
