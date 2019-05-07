#include "coord.h"
#include <stdexcept>
#include <QtMath>
#include <QVector>

quint8 Coord::N = 0;

Coord::Coord()
    :rowIdx(0), colIdx(0), rawIdx(0)
{    }

Coord::Coord(quint8 row, quint8 col)
    :rowIdx(row), colIdx(col), rawIdx((row-1)*Coord::N+(col-1))
{
    if (N<4)
        throw std::runtime_error("Coord::N not set");
#ifdef INVALID_COORD_EXCEPTION
    if (row < 1 || row > Coord::N)
        throw std::out_of_range("coord row is out of range");
    if (col < 1 || col > Coord::N)
        throw std::out_of_range("coord col is out of range");
    if (rawIdx >= Coord::N * Coord::N)
        throw std::out_of_range("coord rawIndex is out of range");
#endif
}

Coord&Coord::operator++(int)
{
#ifdef INVALID_COORD_EXCEPTION
    if (!isValid())
        throw std::runtime_error("uninitialized coord");
    if (rawIdx == N*N)
        throw std::out_of_range("next index is out of range");
#endif
    colIdx++;
    if (colIdx>N)
    {
        rowIdx++;
        colIdx = 1;
    }
    rawIdx++;
    return *this;
}

quint8 Coord::row() const
{
    return rowIdx;
}

quint8 Coord::col() const
{
    return colIdx;
}

quint8 Coord::squareIdx() const
{
    quint8 s_n = static_cast<quint8>(qSqrt(Coord::N));
    quint8 s_col = (colIdx - 1) / s_n;
    quint8 s_row = (rowIdx - 1) / s_n;
    return s_row * s_n + s_col;
}

quint16 Coord::rawIndex() const
{
    return rawIdx;
}

void Coord::setRawIndex(quint16 idx)
{
#ifdef INVALID_COORD_EXCEPTION
    if (idx >= Coord::N * Coord::N)
        throw std::out_of_range("coord rawIndex is out of range");
#endif
    rawIdx = idx;
    colIdx = idx % Coord::N + 1;
    rowIdx = idx / Coord::N + 1;
}

void Coord::setRowCol(quint8 row, quint8 col)
{
#ifdef INVALID_COORD_EXCEPTION
    if (row < 1 || row > Coord::N)
        throw std::out_of_range("coord row is out of range");
    if (col < 1 || col > Coord::N)
        throw std::out_of_range("coord col is out of range");
#endif
    this->rowIdx = row;
    this->colIdx = col;
    rawIdx = (row-1) * Coord::N + (col-1);
}

quint16 Coord::maxRawIndex()
{
    return N*N-1;
}

bool Coord::isValid() const
{
    return rowIdx > 0 && rowIdx <= N && colIdx > 0 && colIdx <= N && rawIdx <= maxRawIndex();
}

void Coord::init(quint8 n)
{
    Coord::N = n;
}

Coord Coord::first()
{
    if (N<4)
        throw std::runtime_error("Coord::N not set");
    return {1,1};
}

Coord Coord::last()
{
    if (N<4)
        throw std::runtime_error("Coord::N not set");
    return {N, N};
}

QVector<Coord> Coord::sameColumnCoordinates() const
{
    QVector<Coord> ret;
    for(quint8 r=1; r<=Coord::N; r++)
        if (r != row())
            ret.append(Coord(r, col()));
    return ret;
}

QVector<Coord> Coord::sameRowCoordinates() const
{
    QVector<Coord> ret;
    for(quint8 c=1; c<=Coord::N; c++)
        if (c != col())
            ret.append(Coord(row(), c));
    return ret;
}

QVector<Coord> Coord::sameSquareCoordinates() const
{
    QVector<Coord> ret;
    auto s_n = static_cast<quint8>(qSqrt(Coord::N));
    quint8 s_row = squareIdx() / s_n;
    quint8 s_col = squareIdx() % s_n;
    Coord co;
    for (quint8 r = s_row * s_n; r < (s_row+1) * s_n; r++)
        for (quint8 c = s_col * s_n; c < (s_col+1) * s_n; c++)
        {
            co = Coord(r+1,c+1);
            if (*this == co)
                continue;
            ret.append(co);
        }
    return ret;
}

bool Coord::operator <(const Coord& o) const
{
    return rawIdx < o.rawIdx;
}

bool Coord::operator >(const Coord& o) const
{
    return rawIdx > o.rawIdx;
}

bool Coord::operator ==(const Coord& o) const
{
    return  rawIdx == o.rawIdx;
}

bool Coord::operator !=(const Coord& o) const
{
    return rawIdx != o.rawIdx;
}

std::ostream& operator <<(std::ostream& stream, const Coord& coord)
{
    stream << '['
           << 'R' << static_cast<int>(coord.row())
           << 'C' << static_cast<int>(coord.col())
           << ']';
    return stream;
}
