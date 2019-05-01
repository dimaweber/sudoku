#ifndef FIELD_H
#define FIELD_H

#include "cell.h"
#include "house.h"
#include "technique.h"

#include <QVector>
#include <QThread>
#include <QElapsedTimer>


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

    quint8 getN() const {return N;}
    void setN(quint8 n);
    void prepareHouses(quint8 n);

    bool readFromFormattedTextFile(const QString& filename);
    bool readFromPlainTextFile(const QString& filename, int num);

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
    bool reduceXWing();
    bool reduceYWing();
    bool reduceXYZWing();

    friend class Technique;
};

class Resolver : public QThread
{
    Q_OBJECT
    Field& field;
    quint64 elaps;
    quint32 enabledTechniques;

public:
    QVector<Technique*> techniques; /// TODO: make in private
    Resolver(Field& field, QObject* parent = nullptr)
        :QThread(parent), field(field), elaps(0), enabledTechniques(0xffff)
    {}
    quint64 resolveTime() const
    {
        return elaps;
    }
    void registerTechnique(Technique* tech)
    {
        techniques.append(tech);
    }
    void process();
protected:
    void run()
    {
        QElapsedTimer timer;
        timer.start();
        process();
        elaps = timer.elapsed();

        if (field.isResolved())
        {
            emit done(elaps);
            emit resolved(elaps);
            std::cout << "resolved" << std::endl;
        }
        else if (!field.isValid())
        {
            emit done(elaps);
            emit failed(elaps);
            std::cout << "is INVALID" << std::endl;
        }
        else if (field.hasEmptyValues())
        {
            emit done(elaps);
            emit unresolved(elaps);
            std::cout << "NOT resolved" << std::endl;
        }
    }
signals:
    void done(quint64);
    void resolved(quint64);
    void unresolved(quint64);
    void failed(quint64);
};
#endif // FIELD_H
