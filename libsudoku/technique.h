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
    const QString techniqueName;
    bool enabled;
    static void fillCandidatesCombinationsMasks(quint8 n);
public:
    Technique (Field& field, const QString& name, bool enabled = true, QObject* parent = nullptr);
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
    quint8 N;
    Field& field;
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
    PerHouseTechnique (Field& field, const QString& name, bool enabled = true, QObject* parent = nullptr)
        :Technique(field, name, enabled, parent)
    {}
protected:
    virtual bool runPerHouse(House* ) =0;
    bool run() final;
};

class PerCellTechnique: public Technique
{
    Q_OBJECT
public:
    PerCellTechnique (Field& field, const QString& name, bool enabled = true, QObject* parent = nullptr)
        :Technique(field, name, enabled, parent)
    {}
protected:
    virtual bool runPerCell(Cell::Ptr ) =0;
    bool run() final;
};

class PerCandidateTechnique: public Technique
{
    Q_OBJECT
public:
    PerCandidateTechnique(Field& field, const QString& name, bool enabled = true, QObject* parent = nullptr)
        :Technique(field, name, enabled, parent)
    {}
protected:
    virtual bool runPerCandidate(CellValue candidate) = 0;
    bool run() final;

};

class NakedSingleTechnique : public PerCellTechnique
{
    Q_OBJECT
public:
    NakedSingleTechnique(Field& field, bool enabled = true, QObject* parent = nullptr);
    void setEnabled(bool enabled = true) override;
    bool canBeDisabled() const override { return false;}
protected:
    bool runPerCell(Cell::Ptr) override;
};

class HiddenSingleTechnique : public PerHouseTechnique
{
    Q_OBJECT
protected:
    bool runPerHouse(House* house) override;
public:
    HiddenSingleTechnique(Field& field, bool enabled = true, QObject* parent = nullptr);
};

class NakedGroupTechnique : public PerHouseTechnique
{
    Q_OBJECT
protected:
    bool runPerHouse(House* house) override;
public:
    NakedGroupTechnique(Field& field, bool enabled = true, QObject* parent = nullptr);
};

class HiddenGroupTechnique: public PerHouseTechnique
{
    Q_OBJECT
protected:
    bool runPerHouse(House* house) override;
public:
    HiddenGroupTechnique(Field& field, bool enabled = true, QObject* parent = nullptr);
};


class IntersectionsTechnique: public Technique
{
    Q_OBJECT
private:
    bool reduceIntersection(SquareHouse& square, LineHouse& area);
public:
    IntersectionsTechnique(Field& field, bool enabled = true, QObject* parent = nullptr);
protected:
    bool run() override;
};

class BiLocationColoringTechnique: public PerCandidateTechnique
{
    Q_OBJECT
public:
    BiLocationColoringTechnique(Field& field, bool enabled = true, QObject* parent = nullptr);
protected:
    bool runPerCandidate(CellValue candidate) override;
    QVector<BiLocationLink> findBiLocationLinks(CellValue val);

};


class XWingTechnique : public Technique
{
    Q_OBJECT
public:
    XWingTechnique(Field& field, bool enabled = true, QObject* parent = nullptr);
protected:
    bool run() override;
};

class YWingTechnique : public PerCellTechnique
{
    Q_OBJECT
public:
    YWingTechnique(Field& field, bool enabled = true, QObject* parent = nullptr);
protected:
    bool runPerCell(Cell::Ptr) override;
};

class XYZWingTechnique: public PerCellTechnique
{
    Q_OBJECT
public:
    XYZWingTechnique(Field& field, bool enabled = true, QObject* parent = nullptr);
protected:
    bool runPerCell(Cell::Ptr) override;
};

class UniqueRectangle : public PerCellTechnique
{
    Q_OBJECT

    struct Rectangle
    {
        Field& field;
        Rectangle(Field& field):field(field){}
        Cell::Ptr cell              {nullptr};
        Cell::Ptr sameRowCell       {nullptr};
        Cell::Ptr sameColumnCell    {nullptr};
        Cell::Ptr diagonalCell      {nullptr};
        Cell::Ptr neigborCell       {nullptr};
        Cell::Ptr diagNeigborCell   {nullptr};

        bool applyType1Check();
        bool applyType2aCheck();
        bool applyType2bCheck();
        bool applyType2cCheck();
        bool applyType3aCheck();
        bool applyType3bCheck();
    };

public:
    UniqueRectangle(Field& field, bool enabled = true, QObject* parent = nullptr);
protected:
    bool runPerCell(Cell::Ptr) override;

    friend std::ostream& operator << (std::ostream& stream, UniqueRectangle::Rectangle& r);
};

#endif // TECHNIQUE_H
