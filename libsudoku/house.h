#ifndef AREA_H
#define AREA_H

#include <QSet>
#include "cell.h"

class CellSet
{
    QString houseName;
protected:
    QVector<Cell::Ptr> cells;
public:
    void addCell(Cell::Ptr pCell);
    void removeCell(Cell::Ptr pCell);

    void setName(const QString& name) {houseName = name;}
    const QString& name() const { return houseName;}

    void print() const;

    bool hasValue(CellValue val) const;
    int candidatesCount(CellValue val) const;
    bool hasUnresolvedCells() const;
    bool removeCandidate(CellValue val);
    int unresolvedCellsCount() const;
    bool hasCell(Cell::CPtr p) const;

    CellSet cellsWithCandidate(CellValue val) const;

    int count() const { return cells.count();}
    bool isEmpty() const { return cells.isEmpty();}
    Cell::Ptr const & operator[](int index) const {return cells[index];}
    Cell::Ptr &       operator[](int index)       {return cells[index];}

    CellSet operator+ (const CellSet& a) const;
    CellSet operator- (const CellSet& a) const;
    CellSet operator/ (const CellSet& a) const;

    typedef typename QVector<Cell::Ptr>::iterator iterator;
    typedef typename QVector<Cell::Ptr>::const_iterator const_iterator;
    inline iterator begin() { return cells.begin(); }
    inline const_iterator begin() const { return cells.constBegin(); }
    inline const_iterator cbegin() const { return cells.constBegin(); }
    inline const_iterator constBegin() const { return cells.constBegin(); }
    inline iterator end() { return cells.end(); }
    inline const_iterator end() const { return cells.constEnd(); }
    inline const_iterator cend() const { return cells.constEnd(); }
    inline const_iterator constEnd() const { return cells.constEnd(); }
};

class House : public CellSet
{
    static quint8 N;
public:
    using Ptr = House*;
    using CPtr = const House*;
    House();
    static void init(quint8 n);

    bool isValid() const;
    bool isResolved() const;
};

class LineHouse: public House
{};

class RowHouse : public LineHouse
{};

class ColumnHouse: public LineHouse
{};

class SquareHouse: public House
{};

#endif // AREA_H
