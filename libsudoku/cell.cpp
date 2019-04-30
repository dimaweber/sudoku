#include "cell.h"
#include "house.h"
#include <iostream>

Cell::Cell(quint8 n, QObject* parent)
    :QObject(parent), val(0), initial_value(false), useDelay(false)
{
    if (n>0)
        resetCandidates(n);
}

#include <unistd.h>
void Cell::setValue(quint8 val, bool init_value)
{
    this->val = val;
    candidateMask.fill(false);
    candidateMask.setBit(val-1, true);
    initial_value = init_value;
    std::cout << "\tvalue " << (int)value() << " set into " << coord() << std::endl;
    emit valueSet(val);
    if(useDelay)
        usleep(100000);

    for(House* pArea: houses)
    {
        pArea->removeCandidate(val);
    }
}

bool Cell::removeCandidate(quint8 guessVal)
{
    if (isResolved())
    {
        return false;
//        throw std::runtime_error("removing guess from knonw value");
    }
    if (!candidateMask.testBit(guessVal-1))
    {
        //throw std::runtime_error("removing unset guess");
        return false;
    }
    candidateMask.clearBit(guessVal-1);
    if (candidateMask.count(true) == 0)
        throw std::runtime_error("no guesses left -- something wrong with algorithm or sudoku");
    std::cout << "\tcandidate " << (int)guessVal << " removed from " << coord() << std::endl;
    emit candidateRemoved(guessVal);
    if (useDelay)
    usleep(50000);
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
    candidateMask &= ~candidate;
    if (candidateMask.count(true) == 0)
        throw std::runtime_error("no guesses left -- something wrong with algorithm or sudoku");
    std::cout << "\tcandidates " << candidate << "removed from " << coord() << std::endl;
    emit candidatesRemoved(candidate);
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

QBitArray Cell::commonCandidates(const Cell& a) const
{
    QBitArray ret(candidatesCapacity());
    ret = candidateMask & a.candidateMask;
    return ret;
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
