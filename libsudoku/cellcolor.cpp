#include "cellcolor.h"
#include "bilocationlink.h"
#include "cell.h"

QVector<QPair<CellColor, CellColor>> ColorPair::pairs;

CellColor ColoredLinksVault::getColor(Cell* cell) const
{
    for(const CellColor& color: map.keys())
    {
        if (map[color].contains(cell))
            return color;
    }
    return ColorPair::UnknownColor;
}

void ColoredLinksVault::addLink(const BiLocationLink& link, ColorPair cp)
{
    addCell(link.first(), cp.first());
    addCell(link.second(), cp.second());
}

void ColoredLinksVault::addCell(Cell* cell, CellColor color)
{
    map[color].append(cell);
}

void ColoredLinksVault::recolor(CellColor from, CellColor to)
{
    map[to].append(map[from]);
    map.remove(from);
}

bool ColoredLinksVault::removeCandidateForColor(CellColor color)
{
    bool changed = false;
    CellColor acolor = ColorPair::antiColor(color);
    for(Cell* cell: map[color])
        changed |= cell->removeCandidate(candidateValue);
    for(Cell* cell: map[acolor])
        cell->setValue(candidateValue);
    return changed;
}

CellColor ColorPair::antiColor(const CellColor& color)
{
    for (const QPair<CellColor, CellColor>& pair: pairs)
        if (pair.first == color)
            return pair.second;
        else if (pair.second == color)
            return pair.first;
    return ColorPair::UnknownColor;
}

ColorPair ColorPair::newPair()
{
    static int nextColor = 0;
    ColorPair pair;
    pair.color1 = nextColor++;
    pair.color2 = nextColor++;
    QPair<CellColor, CellColor> p;
    p.first = pair.color1;
    p.second = pair.color2;
    pairs.append(p);
    return pair;
}
