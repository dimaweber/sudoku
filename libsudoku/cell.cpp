#include "cell.h"
#include "house.h"
#include <iostream>
#include <chrono>
#include <thread>

//#define DELAY_SET_VALUE

Cell::Cell(quint8 n, QObject* parent)
    :QObject(parent)
    #ifdef MT
    , accessLock(QReadWriteLock::Recursive)
    #endif
{
    if (n>0)
        resetCandidates(n);
}

Cell::~Cell()
{
    // unrigisterFromHouses();
}

CellValue Cell::value() const
{
#ifdef MT
    QReadLocker locker(&accessLock);
#endif
    return val;
}

void Cell::setValue(CellValue val, bool init_value)
{
    {
        #ifdef MT
            QWriteLocker locker(&accessLock);
        #endif
        this->val = val;
        candidateMask.fill(false);
        candidateMask.setBit(val-1, true);
    }
    initial_value = init_value;
    std::cout << "\tvalue " << (int)val << " set into " << coord() << std::endl;
    emit valueAboutToBeSet(val);
#ifdef  DELAY_SET_VALUE
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 100ms;
        std::this_thread::sleep_until (t0);
    }
#endif

    for(House::Ptr pArea: houses)
    {
        pArea->removeCandidate(val);
    }
#ifdef  DELAY_SET_VALUE
    if(useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 100ms;
        std::this_thread::sleep_until (t0);
    }
#endif
    emit valueSet(val);
}

bool Cell::removeCandidate(CellValue guessVal)
{
    if (isResolved())
    {
        return false;
//        throw std::runtime_error("removing guess from known value");
    }
    if (!candidateMask.testBit(guessVal-1))
    {
        //throw std::runtime_error("removing unset guess");
        return false;
    }
    emit candidateAboutToBeRemoved(guessVal);
#ifdef  DELAY_SET_VALUE
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 500ms;
        std::this_thread::sleep_until (t0);
    }
#endif
    {
#ifdef MT
        QWriteLocker locker(&accessLock);
#endif
        candidateMask.clearBit(guessVal-1);
    }

    if (candidateMask.count(true) == 0)
        throw std::runtime_error("no guesses left -- something wrong with algorithm or puzzle");
    std::cout << "\tcandidate " << (int)guessVal << " removed from " << coord() << std::endl;
    emit candidateRemoved(guessVal);
#ifdef  DELAY_SET_VALUE
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 50ms;
        std::this_thread::sleep_until (t0);
    }
#endif
    return true;
}

bool Cell::removeCandidate(const QBitArray& candidate)
{
    if (isResolved())
    {
        return false;
//        throw std::runtime_error("trying to remove candaidate from resolved cell");
    }
    if ((candidateMask & candidate).count(true) == 0)
        return false; // nothing will be removed
    emit candidatesAboutToBeRemoved(candidate);
#ifdef  DELAY_SET_VALUE
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 500ms;
        std::this_thread::sleep_until (t0);
    }
#endif
    QBitArray oldCandidates = candidateMask;
    {
        #ifdef MT
            QWriteLocker locker(&accessLock);
        #endif
        candidateMask &= ~candidate;
    }
    if (candidateMask.count(true) == 0)
        throw std::runtime_error("no guesses left -- something wrong with algorithm or sudoku");
    std::cout << "\tcandidates " << candidate << "removed from " << coord() << std::endl;
    emit candidatesRemoved(candidate);
#ifdef  DELAY_SET_VALUE
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 50ms;
        std::this_thread::sleep_until (t0);
    }
#endif
    return oldCandidates != candidateMask;
}

bool Cell::candidatesExactMatch(const QBitArray& mask) const
{
#ifdef MT
    QReadLocker locker(&accessLock);
#endif
    return (candidateMask & mask) == candidateMask;
}

bool Cell::candidatesExactMatch(Cell::CPtr o) const
{
#ifdef MT
    QReadLocker locker(&accessLock);
#endif
    return candidateMask == o->candidateMask;
}

bool Cell::hasCandidate(CellValue guessVal) const
{
#ifdef MT
    QReadLocker locker(&accessLock);
#endif
    if (guessVal > candidateMask.count() || guessVal < 1)
    {
        throw std::out_of_range("candidate is out of range");
        //return false;
    }
    return candidateMask.testBit(guessVal-1);
}

int Cell::hasAnyOfCandidates(const QBitArray& mask) const
{
#ifdef MT
    QReadLocker locker(&accessLock);
#endif
    return (candidateMask & mask).count(true);
}

void Cell::print() const
{
    if (!isResolved())
    {
        std::cout << candidateMask;
    }
    else
        std::cout << (int)value();
}

void Cell::registerInHouse(House& area)
{
    houses.append(&area);
    area.addCell(this);
//    if (value > 0)
    //        area.removeGuess(value);
}

void Cell::resetCandidates(quint8 n)
{
#ifdef MT
    QWriteLocker locker(&accessLock);
#endif
    val = 0;
    candidateMask.resize(n);
    candidateMask.fill(true);
}

bool Cell::isValid() const
{
    return     (isResolved() && candidateMask.count(true) == 1 && candidateMask.at(value()-1))
            || (!isResolved() && candidateMask.count(true) > 1);
}

QVector<CellValue> Cell::candidates() const
{
    QVector<CellValue> ret;
    for (CellValue i=1; i<=candidatesCapacity(); i++)
        if (hasCandidate(i))
            ret.append(i);
    return ret;
}

void Cell::setDelay(bool use)
{
    useDelay =  use;
}

QBitArray Cell::commonCandidates(Cell::CPtr a) const
{
    QBitArray ret(candidatesCapacity());
    ret = candidateMask & a->candidateMask;
    return ret;
}

int Cell::commonCandidatesCount(Cell::CPtr a) const
{
    return commonCandidates(a).count(true);
}

bool Cell::operator ==(const Cell& other) const
{
    return coord() == other.coord();
}

void Cell::reset(quint8 n, quint8 idx)
{
    houses.clear();
    coord().setRawIndex(idx);
    resetCandidates(n);
    setDelay(false);
}

std::ostream& operator <<(std::ostream& stream, const QBitArray& arr)
{
    stream << "{";
    for (int i=0; i<arr.count(); i++)
    {
        if (arr.testBit(i))
        {
            stream << i+1;
        }
    }
    stream << "}";
    return stream;
}
