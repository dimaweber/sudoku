#include "resolver.h"
#include "field.h"

#include <QElapsedTimer>

Resolver::Resolver(Field &field, QObject *parent)
    :QThread(parent), field(field), elaps(0), enabledTechniques(0xffff)
{}

quint64 Resolver::resolveTime() const
{
    return elaps;
}

void Resolver::registerTechnique(Technique *tech)
{
    techniques.append(tech);
}

void Resolver::run()
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

void Resolver::process()
{
    bool changed = false;

    do
    {
        changed = false;

        for(Technique* tech: techniques)
        {
            changed = tech->perform();
            if (changed)
                break;
        }
    }while(changed);
}
