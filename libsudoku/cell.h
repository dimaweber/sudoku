#ifndef CELL_H
#define CELL_H

#include "coord.h"
#include <iostream>
#include <QBitArray>
#include <QVector>
#include <QObject>

class House;

//class Value
//{
//    quint8 val;
//public:
//    explicit Value(quint8 v) :val(v){}
//    bool isSet() const {return val > 0;}
//    int toInt() const {return val;}
//    char toChar() const {return static_cast<char>('0' + val);}
//    QBitArray toBitMask() const { QBitArray bin(9); bin.setBit(val-1); return bin;}

//    void set(int v) { val = v;}
//    void reset() {val = 0;}

//    bool presentInMask (const QBitArray& a) const { return a.testBit(val-1); }
//};

typedef quint8 CellValue;
Q_DECLARE_METATYPE(CellValue)

class Cell: public QObject
{
    Q_OBJECT

    CellValue    val;
    QBitArray candidateMask;
    bool initial_value;
    Coord coordinate;
    QVector<House*> houses;
    Cell& operator = (const Cell& );
    bool useDelay;
public:
    Cell(quint8 n = 0, QObject* parent = nullptr);

    CellValue value() const {return val;}
    bool isInitialValue() const {return initial_value;}
    void setValue(CellValue val, bool init_value = false);
    bool removeCandidate(quint8 val);
    int candidatesCapacity() const {return candidateMask.count();}
    int candidatesCount() const {return candidateMask.count(true);}
    bool isResolved() const {return value() != 0;}
    bool hasCandidate(quint8 val) const;
    void print() const;
    void registerInHouse(House& house);
    Coord& coord() { return coordinate;}
    const Coord& coord() const { return coordinate;}
    void resetCandidates(quint8 n) { candidateMask.resize(n); candidateMask.fill(true);}
    bool isValid() const;
    QVector<CellValue> candidates() const;
    void setDelay(bool use = true);
    bool removeCandidate(const QBitArray& candidate);
    bool candidatesExactMatch(const QBitArray& mask) const;
    int hasAnyOfCandidates(const QBitArray& mask) const;
    QBitArray commonCandidates(const Cell& a) const;

    bool operator == (const Cell& other) const;
signals:
    void valueSet(CellValue v);
    void candidateRemoved(CellValue v);
    void candidatesRemoved(QBitArray v);
};

std::ostream& operator << (std::ostream& stream, const QBitArray& arr);


#endif // CELL_H