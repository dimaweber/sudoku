#pragma once
#include "coord.h"
#include <iostream>
#include <QBitArray>
#include <QVector>
#include <QObject>
#include <QReadWriteLock>
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

using CellValue = quint8;
Q_DECLARE_METATYPE(CellValue)

class Cell: public QObject
{
    Q_OBJECT

    CellValue    val{0};
    QBitArray candidateMask;
    bool initial_value{false};
    Coord coordinate;
    QVector<House*> houses;
    //Cell& operator = (const Cell& );
    bool useDelay{false};

    mutable QReadWriteLock accessLock;

public:
    using Ptr =  Cell*;
    using CPtr = const Cell*;
    Cell(quint8 n = 0, QObject* parent = nullptr);

    CellValue value() const;
    bool isInitialValue() const {return initial_value;}
    void setValue(CellValue val, bool init_value = false);
    void removeValue();
    bool removeCandidate(CellValue val);
    int candidatesCapacity() const {return candidateMask.count();}
    int candidatesCount() const {return candidateMask.count(true);}
    bool isResolved() const {return value() != 0;}
    bool hasCandidate(CellValue val) const;
    void print(std::ostream& stream) const;
    void registerInHouse(House& house);
    Coord& coord() { return coordinate;}
    const Coord& coord() const { return coordinate;}
    void resetCandidates(quint8 n);
    bool isValid() const;
    QVector<CellValue> candidates() const;
    QBitArray commonCandidates(Cell::Ptr o) const { return candidateMask & o->candidateMask;}
    void setDelay(bool use = true);
    bool removeCandidate(const QBitArray& candidate);
    bool candidatesExactMatch(const QBitArray& mask) const;
    bool candidatesExactMatch(Cell::CPtr o) const;
    int hasAnyOfCandidates(const QBitArray& mask) const;
    QBitArray commonCandidates(Cell::CPtr a) const;
    int commonCandidatesCount(Cell::CPtr a) const;

    bool operator == (const Cell& other) const;

    void reset(quint8 n, quint8 idx);
signals:
    void valueSet(CellValue);
    void candidatesReset();
    void valueRemoved();
    void reseted();
    void valueAboutToBeSet(CellValue);
    void candidateRemoved(CellValue);
    void candidateAboutToBeRemoved(CellValue);
    void candidatesRemoved(QBitArray);
    void candidatesAboutToBeRemoved(QBitArray);
};

std::ostream& operator << (std::ostream& stream, const QBitArray& arr);
std::ostream& operator << (std::ostream& stream, const Cell& cell);
