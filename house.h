#ifndef AREA_H
#define AREA_H

#include <QVector>

class Cell;

class Set
{
    QString houseName;
protected:
    QVector<Cell*> cells;
public:
    void addCell(Cell* pCell);

    void setName(const QString& name) {houseName = name;}
    const QString& name() const { return houseName;}

    void print() const;

    bool hasValue(quint8 val) const;
    int candidatesCount(quint8 val) const;
    bool hasEmptyValues() const;
    bool removeCandidate(quint8 val);
    int unresolvedCellsCount() const;
    bool hasCell(const Cell* p) const;

    QVector<Cell*> cellsWithCandidate(quint8 val) const;

    bool isEmpty() const { return cells.isEmpty();}
    Set operator+ (const Set& a) const;
    Set operator- (const Set& a) const;
    Set operator/ (const Set& a) const;
};

class House : public Set
{
    static quint8 N;
    static QVector<QBitArray> allCandidatesCombinationsMasks;
    static void fillCandidatesCombinationsMasks();
public:
    House();
    static void init(quint8 n);

    bool checkNakedCombinations(); // Naked Pair / Triple / Quad / ...
    bool checkHiddenCombinations(); // Hidden Pair / Triple / Quad / ...
    bool checkNakedSingle();
    bool checkHiddenSingle();

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
