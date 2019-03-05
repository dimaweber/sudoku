#ifndef CELLCOLOR_H
#define CELLCOLOR_H

#include <QPair>
#include <QMap>
#include <QVector>

class Cell;
class BiLocationLink;

typedef int CellColor;

class ColorPair
{
    static QVector<QPair<CellColor, CellColor>> pairs;
    CellColor color1;
    CellColor color2;
public:
    static const CellColor UnknownColor = -1;
    CellColor first() const {return color1;}
    CellColor second() const {return color2;}
    static CellColor antiColor(const CellColor& color);
    static ColorPair newPair();
};

class ColoredLinksVault
{
    quint8 candidateValue;
    QMap<CellColor, QVector<Cell*>> map;
public:
    ColoredLinksVault(quint8 candidate):candidateValue(candidate){}
    CellColor getColor(Cell* cell) const;
    void addLink(const BiLocationLink& link, ColorPair cp);
    void addCell(Cell* cell, CellColor color);
    void recolor(CellColor from, CellColor to);

    bool removeCandidateForColor(CellColor color);
};

#endif // CELLCOLOR_H
