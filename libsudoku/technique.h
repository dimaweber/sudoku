#ifndef TECHNIQUE_H
#define TECHNIQUE_H

#include "house.h"

#include <QtCore/qglobal.h>
#include <QString>
#include <QBitArray>

class Field;

class Technique
{
    bool enabled;
    const QString techniqueName;
    static void fillCandidatesCombinationsMasks(size_t n);
public:
    Technique (Field& field, const QString name);
    virtual ~Technique()
    {}
    const QString& name() const {return techniqueName;}
    virtual void setEnabled(bool enabled = true);
    bool isEnabled() const {return enabled;}
    bool perform()
    {
        if (!enabled)
            return false;
        return run();
    }
protected:
    static QVector<QBitArray> allCandidatesCombinationsMasks;
    QVector<House*> areas();
    QVector<SquareHouse> squares();
    QVector<RowHouse> rows();
    QVector<ColumnHouse> columns();
    virtual bool run() = 0;
    Field& field;
    int N;
};

class NakedSingleTechnique : public Technique
{
public:
    NakedSingleTechnique(Field& field);
    virtual void setEnabled(bool enabled = true) override;
protected:
    virtual bool run() override;
};

class PerHouseTechnique: public Technique
{
public:
    PerHouseTechnique (Field& field, const QString name)
        :Technique(field, name)
    {}
protected:
    virtual bool runPerHouse(House* ) =0;
    virtual bool run() final;
};

class HiddenSingleTechnique : public PerHouseTechnique
{
protected:
    bool runPerHouse(House* house);
public:
    HiddenSingleTechnique(Field& field);
};

class NakedGroupTechnique : public PerHouseTechnique
{
protected:
    bool runPerHouse(House* house);
public:
    NakedGroupTechnique(Field& field);
};

class HiddenGroupTechnique: public PerHouseTechnique
{
protected:
    bool runPerHouse(House* house);
public:
    HiddenGroupTechnique(Field& field);
};


class IntersectionsTechnique: public Technique
{
private:
    bool reduceIntersection(SquareHouse& square, LineHouse& area);
public:
    IntersectionsTechnique(Field& field);
protected:
    virtual bool run() override;
};

#endif // TECHNIQUE_H
