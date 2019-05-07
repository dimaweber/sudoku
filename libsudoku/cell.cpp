#include "cell.h"
#include "house.h"
#include <iostream>
#include <chrono>
#include <thread>

Cell::Cell(quint8 n, QObject* parent)
    :QObject(parent), val(0), initial_value(false), useDelay(false)
{
    if (n>0)
        resetCandidates(n);
}

void Cell::setValue(quint8 val, bool init_value)
{
    this->val = val;
    candidateMask.fill(false);
    candidateMask.setBit(val-1, true);
    initial_value = init_value;
    std::cout << "\tvalue " << (int)value() << " set into " << coord() << std::endl;
    emit valueAboutToBeSet(val);
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 100ms;
        //std::this_thread::sleep_until (t0);
    }

    for(House::Ptr pArea: houses)
    {
        pArea->removeCandidate(val);
    }

    if(useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 100ms;
//        std::this_thread::sleep_until (t0);
    }
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
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 500ms;
//        std::this_thread::sleep_until (t0);
    }
    candidateMask.clearBit(guessVal-1);
    if (candidateMask.count(true) == 0)
        throw std::runtime_error("no guesses left -- something wrong with algorithm or sudoku");
    std::cout << "\tcandidate " << (int)guessVal << " removed from " << coord() << std::endl;
    emit candidateRemoved(guessVal);
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 50ms;
//        std::this_thread::sleep_until (t0);
    }
    return true;
}

bool Cell::removeCandidate(const QBitArray& candidate)
{
    if (isResolved())
    {
        return false;
//        throw std::runtime_error("trying to remove candaidate from resolved cell");
    }
    QBitArray a = candidateMask & candidate;
    if ((candidateMask & candidate).count(true) == 0)
        return false; // nothing will be removed
    emit candidatesAboutToBeRemoved(candidate);
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 500ms;
//        std::this_thread::sleep_until (t0);
    }
    candidateMask &= ~candidate;
    if (candidateMask.count(true) == 0)
        throw std::runtime_error("no guesses left -- something wrong with algorithm or sudoku");
    std::cout << "\tcandidates " << candidate << "removed from " << coord() << std::endl;
    emit candidatesRemoved(candidate);
    if (useDelay)
    {
        using namespace  std::chrono_literals;
        auto t0 = std::chrono::steady_clock::now() + 50ms;
//        std::this_thread::sleep_until (t0);
    }
    return true;
}

bool Cell::candidatesExactMatch(const QBitArray& mask) const
{
    return (candidateMask & mask) == candidateMask;
}

bool Cell::hasCandidate(quint8 guessVal) const
{
    if (guessVal > candidateMask.count() || guessVal < 1)
    {
        throw std::out_of_range("candidate is out of range");
        return false;
    }
    return candidateMask.testBit(guessVal-1);
}

int Cell::hasAnyOfCandidates(const QBitArray& mask) const
{
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
    candidateMask.resize(n);
    candidateMask.fill(true);
}

bool Cell::isValid() const
{
    return     (isResolved() && candidateMask.count(true) == 1 && candidateMask.at(value()-1) == true)
            || (!isResolved() && candidateMask.count(true) > 1);
}

QVector<CellValue> Cell::candidates() const
{
    QVector<CellValue> ret;
    for (int i=1; i<=candidatesCapacity(); i++)
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
