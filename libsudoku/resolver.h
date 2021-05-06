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
    ~Resolver();
    quint64 resolveTime() const;
    template<class TECH> TECH* registerTechnique(bool enabled = true)
    {
        TECH* ptr = new TECH(field, enabled, this);
        techniques.append(ptr);
        return ptr;
    }
    void process();
    Technique* technique(const QString& techName);
public slots:
    void stop();
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
