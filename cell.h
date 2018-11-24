#ifndef CELL_H
#define CELL_H

#include "coord.h"
#include <iostream>
#include <QBitArray>
#include <QVector>

class House;

class Cell
{
    quint8    val;
    QBitArray candidateMask;
    bool initial_value;
    Coord coordinate;
    QVector<House*> houses;
public:
    Cell();
    Cell (quint8 val);
    quint8 value() const {return val;}
    bool isInitialValue() const {return initial_value;}
    void setValue(quint8 val, bool init_value = false);
    bool removeCandidate(quint8 val);
    bool removeCandidate(const QBitArray& candidate);
    int candidatesCapacity() const {return candidateMask.count();}
    int candidatesCount() const {return candidateMask.count(true);}
    bool candidatesExactMatch(const QBitArray& mask) const;
    bool isResolved() const {return value() != 0;}
    bool hasCandidate(quint8 val) const;
    int hasAnyOfCandidates(const QBitArray& mask) const;
    void print() const;
    void registerInHouse(House& house);
    Coord& coord() { return coordinate;}
    const Coord& coord() const { return coordinate;}
    void resetCandidates(quint8 n) { candidateMask.resize(n); candidateMask.fill(true);}

    QBitArray commonCandidates(const Cell& a) const;
};

std::ostream& operator << (std::ostream& stream, const QBitArray& arr);


#endif // CELL_H
