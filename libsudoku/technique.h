#ifndef TECHNIQUE_H
#define TECHNIQUE_H

#include "house.h"
#include "bilocationlink.h"

#include <QString>
#include <QBitArray>

#include <chrono>
#include <thread>

class Field;

class Technique : public QObject
{
    Q_OBJECT
    bool enabled;
    const QString techniqueName;
    static void fillCandidatesCombinationsMasks(quint8 n);
public:
    Technique (Field& field, const QString name, QObject* parent = nullptr);
    virtual ~Technique();
    const QString& name() const {return techniqueName;}
    virtual void setEnabled(bool enabled = true);
    virtual bool canBeDisabled() const { return true;}
    bool isEnabled() const {return enabled;}
    bool perform();
protected:
    static QSet<QBitArray> allCandidatesCombinationsMasks;
    QVector<House::Ptr>& areas();
    QVector<SquareHouse>& squares();
    QVector<RowHouse>& rows();
    QVector<ColumnHouse>& columns();
    QVector<Cell::Ptr>& cells();
    Cell::Ptr cell(const Coord& c);

    virtual bool run() = 0;
    Field& field;
    quint8 N;
signals:
    void started();
    void done();
    void applied();

    void cellAnalyzeStarted(Cell::Ptr);
    void cellAnalyzeFinished(Cell::Ptr);
};

class PerHouseTechnique: public Technique
{
    Q_OBJECT
public:
    PerHouseTechnique (Field& field, const QString name, QObject* parent = nullptr)
        :Technique(field, name, parent)
    {}
protected:
    virtual bool runPerHouse(House* ) =0;
    virtual bool run() final;
};

class PerCellTechnique: public Technique
{
    Q_OBJECT
public:
    PerCellTechnique (Field& field, const QString name, QObject* parent = nullptr)
        :Technique(field, name, parent)
    {}
protected:
    virtual bool runPerCell(Cell::Ptr ) =0;
    virtual bool run() final;
};

class NakedSingleTechnique : public PerCellTechnique
{
    Q_OBJECT
public:
    NakedSingleTechnique(Field& field, QObject* parent = nullptr);
    virtual void setEnabled(bool enabled = true) override;
    virtual bool canBeDisabled() const override { return false;}
protected:
    virtual bool runPerCell(Cell::Ptr) override;
};

class HiddenSingleTechnique : public PerHouseTechnique
{
    Q_OBJECT
protected:
    bool runPerHouse(House* house);
public:
    HiddenSingleTechnique(Field& field, QObject* parent = nullptr);
};

class NakedGroupTechnique : public PerHouseTechnique
{
    Q_OBJECT
protected:
    bool runPerHouse(House* house);
public:
    NakedGroupTechnique(Field& field, QObject* parent = nullptr);
};

class HiddenGroupTechnique: public PerHouseTechnique
{
    Q_OBJECT
protected:
    bool runPerHouse(House* house);
public:
    HiddenGroupTechnique(Field& field, QObject* parent = nullptr);
};


class IntersectionsTechnique: public Technique
{
    Q_OBJECT
private:
    bool reduceIntersection(SquareHouse& square, LineHouse& area);
public:
    IntersectionsTechnique(Field& field, QObject* parent = nullptr);
protected:
    virtual bool run() override;
};

class BiLocationColoringTechnique: public Technique
{
    Q_OBJECT
public:
    BiLocationColoringTechnique(Field& field, QObject* parent = nullptr);
protected:
    bool run() override;
    QVector<BiLocationLink> findBiLocationLinks(CellValue val);

};


class XWingTechnique : public Technique
{
    Q_OBJECT
public:
    XWingTechnique(Field& field, QObject* parent = nullptr);
protected:
    bool run() override;
};

class YWingTechnique : public PerCellTechnique
{
    Q_OBJECT
public:
    YWingTechnique(Field& field, QObject* parent = nullptr);
protected:
    bool runPerCell(Cell::Ptr) override;
};

class XYZWingTechnique: public PerCellTechnique
{
    Q_OBJECT
public:
    XYZWingTechnique(Field& field, QObject* parent = nullptr);
protected:
    bool runPerCell(Cell::Ptr) override;
};

#endif // TECHNIQUE_H
