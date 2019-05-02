#ifndef RESOLVER_H
#define RESOLVER_H

#include <QThread>
#include <QVector>

class Field;
class Technique;

class Resolver : public QThread
{
    Q_OBJECT
    Field& field;
    quint64 elaps;
    quint32 enabledTechniques;

public:
    QVector<Technique*> techniques; /// TODO: make in private
    Resolver(Field& field, QObject* parent = nullptr);
    quint64 resolveTime() const;
    void registerTechnique(Technique* tech);
    void process();
protected:
    void run();
signals:
    void done(quint64);
    void resolved(quint64);
    void unresolved(quint64);
    void failed(quint64);
    void newIteration();
};

#endif // RESOLVER_H
