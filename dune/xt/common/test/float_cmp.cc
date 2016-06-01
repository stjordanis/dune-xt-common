// This file is part of the dune-xt-common project:
//   https://github.com/dune-community/dune-xt-common
// The copyright lies with the authors of this file (see below).
// License: Dual licensed as  BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Felix Schindler (2015 - 2016)
//   Rene Milk       (2015)

#define DUNE_XT_COMMON_TEST_MAIN_CATCH_EXCEPTIONS 1

#include <dune/xt/common/test/main.hxx>

#include <vector>

#include <dune/common/dynvector.hh>
#include <dune/common/fvector.hh>

#include <dune/xt/common/float_cmp.hh>
#include <dune/xt/common/test/float_cmp.hh>
#include <dune/xt/common/fvector.hh>

using namespace Dune;
using XT::Common::create;
using namespace XT::Common::FloatCmp;
using XT::Common::VectorAbstraction;

struct FloatCmpTest : public testing::Test
{
  typedef TESTTYPE V;
  static const size_t s_size = VectorAbstraction<V>::has_static_size ? VectorAbstraction<V>::static_size : VECSIZE;

  typedef typename VectorAbstraction<V>::ScalarType S;
  typedef typename VectorAbstraction<V>::RealType R;

  FloatCmpTest()
    : zero(create<V>(s_size, create<S>(0, 0)))
    , one(create<V>(s_size, create<S>(0, 1)))
    , epsilon(create<V>(s_size, XT::Common::FloatCmp::DEFAULT_EPSILON::value()))
    , eps_plus(create<V>(s_size, XT::Common::FloatCmp::DEFAULT_EPSILON::value() * 1.1))
    , eps_minus(create<V>(s_size, XT::Common::FloatCmp::DEFAULT_EPSILON::value() * 0.9))
    , two(create<V>(s_size, create<S>(0, 2)))
    , test_config(DXTC_CONFIG.sub("test_common_float_cmp"))
  {
  }

  const V zero;
  const V one;
  const V epsilon;
  const V eps_plus;
  const V eps_minus;
  const V two;
  const typename XT::Common::Configuration test_config;

  void check_eq()
  {

    TEST_DXTC_EXPECT_FLOAT_EQ(zero, zero);
    EXPECT_FALSE(FLOATCMP_EQ(zero, eps_plus));
    EXPECT_FALSE(FLOATCMP_EQ(zero, one));
    TEST_DXTC_EXPECT_FLOAT_EQ(one, one + eps_minus);
    EXPECT_FALSE(FLOATCMP_EQ(one, two));

    if (test_config["cmpstyle_is_relative"] == "true")
      EXPECT_FALSE(FLOATCMP_EQ(zero, eps_minus));
    else
      TEST_DXTC_EXPECT_FLOAT_EQ(zero, eps_minus);

    if (test_config["cmpstyle_is_numpy"] == "true")
      TEST_DXTC_EXPECT_FLOAT_EQ(one, one + eps_plus);
    else
      EXPECT_FALSE(FLOATCMP_EQ(one, one + eps_minus));
  }

  void check_ne()
  {
    EXPECT_FALSE(FLOATCMP_NE(zero, zero));
    TEST_DXTC_EXPECT_FLOAT_NE(zero, eps_minus);
    TEST_DXTC_EXPECT_FLOAT_NE(zero, one);
    EXPECT_FALSE(FLOATCMP_NE(one, one + eps_minus));
    TEST_DXTC_EXPECT_FLOAT_NE(one, two);

    if (test_config["cmpstyle_is_relative"] == "true")
      TEST_DXTC_EXPECT_FLOAT_NE(zero, eps_minus);
    else
      EXPECT_FALSE(FLOATCMP_NE(zero, eps_minus));

    if (test_config["cmpstyle_is_numpy"] == "true")
      EXPECT_FALSE(FLOATCMP_NE(one, one + eps_minus));
    else
      TEST_DXTC_EXPECT_FLOAT_NE(one, one + eps_minus);
  }

  void check_gt()
  {
    EXPECT_FALSE(FLOATCMP_GT(zero, zero));
    TEST_DXTC_EXPECT_FLOAT_GT(eps_minus, zero);
    TEST_DXTC_EXPECT_FLOAT_GT(one, zero);
    EXPECT_FALSE(FLOATCMP_GT(one + eps_minus, one));
    TEST_DXTC_EXPECT_FLOAT_GT(two, one);

    if (test_config["cmpstyle_is_relative"] == "true")
      TEST_DXTC_EXPECT_FLOAT_GT(eps_minus, zero);
    else
      EXPECT_FALSE(FLOATCMP_GT(eps_minus, zero));

    if (test_config["cmpstyle_is_numpy"] == "true")
      EXPECT_FALSE(FLOATCMP_GT(one + eps_minus, one));
    else
      TEST_DXTC_EXPECT_FLOAT_GT(one + eps_minus, one);
  }

  void check_lt()
  {
    EXPECT_FALSE(FLOATCMP_LT(zero, zero));
    TEST_DXTC_EXPECT_FLOAT_LT(zero, eps_minus);
    TEST_DXTC_EXPECT_FLOAT_LT(zero, one);
    EXPECT_FALSE(FLOATCMP_LT(one, one + eps_minus));
    TEST_DXTC_EXPECT_FLOAT_LT(one, two);

    if (test_config["cmpstyle_is_relative"] == "true")
      TEST_DXTC_EXPECT_FLOAT_LT(zero, eps_minus);
    else
      EXPECT_FALSE(FLOATCMP_LT(zero, eps_minus));

    if (test_config["cmpstyle_is_numpy"] == "true")
      EXPECT_FALSE(FLOATCMP_LT(one, one + eps_minus));
    else
      TEST_DXTC_EXPECT_FLOAT_LT(one, one + eps_minus);
  }

  void check_ge()
  {
    TEST_DXTC_EXPECT_FLOAT_GE(zero, zero);
    TEST_DXTC_EXPECT_FLOAT_GE(eps_minus, zero);
    TEST_DXTC_EXPECT_FLOAT_GE(eps_minus, zero);
    TEST_DXTC_EXPECT_FLOAT_GE(one, zero);
    TEST_DXTC_EXPECT_FLOAT_GE(one + eps_minus, one);
    TEST_DXTC_EXPECT_FLOAT_GE(one + eps_minus, one);
    TEST_DXTC_EXPECT_FLOAT_GE(two, one);
  }

  void check_le()
  {
    TEST_DXTC_EXPECT_FLOAT_LE(zero, zero);
    TEST_DXTC_EXPECT_FLOAT_LE(zero, eps_minus);
    TEST_DXTC_EXPECT_FLOAT_LE(zero, eps_minus);
    TEST_DXTC_EXPECT_FLOAT_LE(zero, one);
    TEST_DXTC_EXPECT_FLOAT_LE(one, one + eps_minus);
    TEST_DXTC_EXPECT_FLOAT_LE(one, one + eps_minus);
    TEST_DXTC_EXPECT_FLOAT_LE(one, two);
  }
}; // struct FloatCmpBase


TEST_F(FloatCmpTest, eq)
{
  this->check_eq();
}

TEST_F(FloatCmpTest, ne)
{
  this->check_ne();
}

TEST_F(FloatCmpTest, gt)
{
  this->check_gt();
}

TEST_F(FloatCmpTest, lt)
{
  this->check_lt();
}

TEST_F(FloatCmpTest, ge)
{
  this->check_ge();
}

TEST_F(FloatCmpTest, le)
{
  this->check_le();
}
