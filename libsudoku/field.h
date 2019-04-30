#ifndef FIELD_H
#define FIELD_H

#include "cell.h"
#include "house.h"
#include "bilocationlink.h"
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

class Resolver : public QThread
{
    Q_OBJECT
    Field& field;
    quint64 elaps;
public:
    Resolver(Field& field, QObject* parent = nullptr)
        :QThread(parent), field(field), elaps(0)
    {}
    quint64 resolveTime() const
    {
        return elaps;
    }
protected:
    void run()
    {
        QElapsedTimer timer;
        timer.start();
        field.process();
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
