#include "cell.h"
#include "house.h"
#include <iostream>

Cell::Cell()
    :val(0), initial_value(false)
{}

Cell::Cell(quint8 val)
    :val(val), initial_value(true)
{
    candidateMask.fill(false);
    candidateMask.setBit(val-1);
}

void Cell::setValue(quint8 val, bool init_value)
{
    this->val = val;
    candidateMask.fill(false);
    candidateMask.setBit(val-1, true);
    initial_value = init_value;
    std::cout << "\tvalue " << (int)value() << " set into " << coord() << std::endl;
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
    return true;
}

bool Cell::removeCandidate(const QBitArray& candidate)
{
    if (isResolved())
    {
        return false;
//        throw std::runtime_error("removing guess from knonw value");
    }
    if ((candidateMask | candidate) == candidate)
        return false; // no other candidates
    candidateMask &= ~candidate;
    if (candidateMask.count(true) == 0)
        throw std::runtime_error("no guesses left -- something wrong with algorithm or sudoku");
    std::cout << "\tcandidates " << candidate << "removed from " << coord() << std::endl;
    return true;
}

bool Cell::candidatesExactMatch(const QBitArray& mask) const
{
    return (candidateMask & mask) == candidateMask;
}

bool Cell::hasCandidate(quint8 guessVal) const
{
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
        std::cout << value();
}

void Cell::registerInHouse(House& area)
{
    houses.append(&area);
    area.addCell(this);
//    if (value > 0)
    //        area.removeGuess(value);
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
