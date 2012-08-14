#ifndef DUNE_STUFF_MATH_HH
#define DUNE_STUFF_MATH_HH

#include <vector>
#include <limits>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <dune/common/deprecated.hh>
#include <dune/common/static_assert.hh>

#include <boost/static_assert.hpp>
#include <boost/fusion/include/void.hpp>
#include <boost/format.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#if HAVE_DUNE_FEM
#include <dune/fem/function/blockvectorfunction/blockvectorfunction.hh>
namespace boost {
namespace math {
//! isinf specialization for Dune::StraightenBlockVector
template <class BlockVectorImp, class DofImp>
inline bool isinf(const Dune::StraightenBlockVector<BlockVectorImp, DofImp>& x)
{
  for (size_t i = 0; i < x.size(); ++i) {
    if (std::isinf(x[i]))
      return true;
  }
  return false;
} // isinf
}
}
#endif // if HAVE_DUNE_FEM

namespace Dune {
namespace Stuff {
namespace Common {
namespace Math {

/** \todo DOCME **/
template <class SomeRangeType, class OtherRangeType>
static double colonProduct(const SomeRangeType& arg1, const OtherRangeType& arg2)
{
  dune_static_assert(SomeRangeType::cols == SomeRangeType::rows && OtherRangeType::cols == OtherRangeType::rows
                         && int(OtherRangeType::cols) == int(SomeRangeType::rows),
                     "RangeTypes_dont_fit");

  double ret = 0.0;
  // iterators
  typedef typename SomeRangeType::ConstRowIterator ConstRowIteratorType;
  typedef typename SomeRangeType::row_type::ConstIterator ConstIteratorType;
  ConstRowIteratorType arg1RowItEnd = arg1.end();
  ConstRowIteratorType arg2RowItEnd = arg2.end();
  ConstRowIteratorType arg2RowIt = arg2.begin();
  for (ConstRowIteratorType arg1RowIt = arg1.begin(); arg1RowIt != arg1RowItEnd && arg2RowIt != arg2RowItEnd;
       ++arg1RowIt, ++arg2RowIt) {
    ConstIteratorType row1ItEnd = arg1RowIt->end();
    ConstIteratorType row2ItEnd = arg2RowIt->end();
    ConstIteratorType row2It = arg2RowIt->begin();
    for (ConstIteratorType row1It = arg1RowIt->begin(); row1It != row1ItEnd && row2It != row2ItEnd;
         ++row1It, ++row2It) {
      ret += *row1It * *row2It;
    }
  }
  return ret;
} // colonProduct

/**
 * since std::numeric_limits<T>::epsilon() is 0 for integral types
 * use this to get the minimum increment/difference for all basic types
 * (or add specializations as necessary ofc)
 **/
template <class T, bool is_integral = std::is_integral<T>::value>
struct Epsilon
{
};

template <class T>
struct Epsilon<T, true>
{
  static const T value;
};

template <class T>
struct Epsilon<T, false>
{
  static const T value;
};

template <class T>
const T Epsilon<T, true>::value = 1;
template <class T>
const T Epsilon<T, false>::value = std::numeric_limits<T>::epsilon();

//! get a vector with values in [start : increment : end)
template <class T, class sequence = std::vector<T>>
sequence range(const T start, const T end, const T increment = Epsilon<T>::value)
{
  sequence ret(typename sequence::size_type(std::abs((end - start) / increment)), start);
  typename sequence::size_type i = 0;
  std::generate(std::begin(ret), std::end(ret), [&]() { return start + (increment * i++); });
  return ret;
}

//! get a vector with values in [0 : Epsilon<T> : end)
template <class T, class sequence = std::vector<T>>
sequence range(const T end)
{
  return range(T(0), end);
}

/**
   *  \brief  dyadic product
   *
   *          Implements \f$\left(arg_{1} \otimes arg_{2}\right)_{i,j}:={arg_{1}}_{i} {arg_{2}}_{j}\f$
   *			RangeType1 should be fieldmatrix, RangeType2 fieldvector
   **/
template <class RangeType1, class RangeType2>
static RangeType1 dyadicProduct(const RangeType2& arg1, const RangeType2& arg2)
{
  RangeType1 ret(0.0);

  typedef typename RangeType1::RowIterator MatrixRowIteratorType;
  typedef typename RangeType2::ConstIterator ConstVectorIteratorType;
  typedef typename RangeType2::Iterator VectorIteratorType;
  MatrixRowIteratorType rItEnd   = ret.end();
  ConstVectorIteratorType arg1It = arg1.begin();
  for (MatrixRowIteratorType rIt = ret.begin(); rIt != rItEnd; ++rIt) {
    ConstVectorIteratorType arg2It = arg2.begin();
    VectorIteratorType vItEnd = rIt->end();
    for (VectorIteratorType vIt = rIt->begin(); vIt != vItEnd; ++vIt) {
      *vIt = *arg1It * *arg2It;
      ++arg2It;
    }
    ++arg1It;
  }
  return ret;
} // dyadicProduct

//! a vector wrapper for continiously updating min,max,avg of some element type vector
template <class ElementType>
class MinMaxAvg
{
protected:
  typedef MinMaxAvg<ElementType> ThisType;
  typedef std::vector<ElementType> ElementsVec;
  typedef typename ElementsVec::const_iterator ElementsVecConstIterator;

public:
  MinMaxAvg()
  {
  }

  template <class stl_container_type>
  MinMaxAvg(const stl_container_type& elements)
  {
    dune_static_assert((boost::is_same<ElementType, typename stl_container_type::value_type>::value),
                       "cannot assign mismatching types");
    acc_ = std::for_each(elements.begin(), elements.end(), acc_);
  }

  ElementType min() const
  {
    return boost::accumulators::min(acc_);
  }
  ElementType max() const
  {
    return boost::accumulators::max(acc_);
  }
  ElementType average() const
  {
    return boost::accumulators::mean(acc_);
  }

  void operator()(const ElementType& el)
  {
    acc_(el);
  }

  void output(std::ofstream& stream)
  {
    stream << boost::format("min: %e\tmax: %e\tavg: %e\n") % min() % max() % average();
  }

protected:
  typedef boost::accumulators::stats<boost::accumulators::tag::max, boost::accumulators::tag::min,
                                     boost::accumulators::tag::mean> StatsType;
  boost::accumulators::accumulator_set<ElementType, StatsType> acc_;

  MinMaxAvg(const ThisType& other);
};

//! bound \param var in [\param min,\param max]
template <typename T>
T clamp(const T var, const T min, const T max)
{
  return (var < min) ? min : (var > max) ? max : var;
}

//! float comparison
template <class T>
bool aboutEqual(const T& x, const T& y, T relative_tolerance = 1e-10, T absolute_tolerance = 1e-10)
    DUNE_DEPRECATED_MSG("use dune/common/float_cmp:Dune::FloatCmp::eq instead ");

template <class T>
bool aboutEqual(const T& x, const T& y, T relative_tolerance, T absolute_tolerance)
{
  return std::fabs(x - y) <= std::max(absolute_tolerance, relative_tolerance * std::max(std::fabs(x), std::fabs(y)));
}

/**
   * \brief MovingAverage
   * \deprecated in favor of MinMaxAvg<double>
   **/
class MovingAverage
{
  double avg_;
  size_t steps_;

public:
  DUNE_DEPRECATED_MSG("use MinMaxAvg this from header instead")
  MovingAverage()
    : avg_(0.0)
    , steps_(0)
  {
  }
  MovingAverage& operator+=(double val)
  {
    avg_ += (val - avg_) / ++steps_;
    return *this;
  }

  operator double()
  {
    return avg_;
  }
};

//! no-branch sign function
long sign(long x)
{
  return long(x != 0) | (long(x >= 0) - 1);
}

} // namespace Math
} // namespace Common
} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_MATH_HH

/** Copyright (c) 2012, Rene Milk
   * All rights reserved.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions are met:
   *
   * 1. Redistributions of source code must retain the above copyright notice, this
   *    list of conditions and the following disclaimer.
   * 2. Redistributions in binary form must reproduce the above copyright notice,
   *    this list of conditions and the following disclaimer in the documentation
   *    and/or other materials provided with the distribution.
   *
   * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   *
   * The views and conclusions contained in the software and documentation are those
   * of the authors and should not be interpreted as representing official policies,
   * either expressed or implied, of the FreeBSD Project.
   **/