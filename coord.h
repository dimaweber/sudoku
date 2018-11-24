#ifndef COORD_H
#define COORD_H

#include <QtCore/qglobal.h>
#include <iostream>

class Coord
{
    static quint8 N;
    quint8  rowIdx;
    quint8  colIdx;
    quint16 rawIdx;
public:
    Coord();

    Coord(quint8 row, quint8 col);

    Coord& operator++(int);


    quint8 row() const;
    quint8 col() const;
    quint8 squareIdx() const;
    quint16 rawIndex() const;
    void setRawIndex(quint16 idx);
    void setRowCol(quint8 row, quint8 col);
    static quint16 maxRawIndex();
    bool isValid() const;
    static void init(quint16 n);
    static Coord first();
    static Coord last();

    bool operator < (const Coord& o) const;
    bool operator > (const Coord& o) const;
    bool operator == (const Coord& o)const;
    bool operator != (const Coord& o) const;
};

std::ostream& operator << (std::ostream& stream, const Coord& coord);

#endif // COORD_H
