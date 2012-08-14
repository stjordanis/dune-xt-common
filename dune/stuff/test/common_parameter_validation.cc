#include "test_common.hh"

#include <dune/stuff/common/parameter/validation.hh>
#include <dune/stuff/common/type_utils.hh>
#include <dune/stuff/common/math.hh>
#include <dune/common/tuples.hh>
#include <dune/common/tupleutility.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/bigunsignedint.hh>
#include <limits>
#include <iostream>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>

using namespace Dune::Stuff::Common;
using namespace Dune::Stuff::Common::Parameter;

typedef testing::Types<double, float, // Dune::bigunsignedint,
                       int, unsigned int, unsigned long, long long, char> MathTestTypes;

template <class T>
struct ValidationTest : public testing::Test
{

  /** for some weird reason my compiler thinks ValidationTest is an abstract class
   * if I don't implement "void TestBody();"
   * \see common_math.cc testcases for why I think it's weird
   **/
  void TestBody()
  {
    const int samples = 100000;
    std::cout << "\tTesting Validators for type " << Typename<T>::value() << "\n\t\t" << samples
              << " random numbers ..." << std::endl;
    {
      std::default_random_engine generator;
      const T lower = std::numeric_limits<T>::min();
      const T upper = std::numeric_limits<T>::max();
      typename Distribution<T, boost::is_integral<T>::value>::type distribution(lower, upper);
      for (int i = samples; i > 0; --i) {
        const T arg = distribution(generator);
        test(lower, upper, arg);
      }
    }
    std::cout << "\t\tfixed interval" << std::endl;
    {
      const T lower = T(0);
      const T upper = T(2);
      const T arg = T(1);
      test(lower, upper, arg);
    }
    std::cout << "\t\tdone." << std::endl;
  }

  void test(const T lower, const T upper, const T arg) const
  {
    const T eps         = Math::Epsilon<T>::value;
    const T clamped_arg = Math::clamp(arg, T(lower + eps), T(upper - eps));
    EXPECT_TRUE(ValidateAny<T>()(arg));
    EXPECT_TRUE(ValidateLess<T>(clamped_arg)(upper));
    EXPECT_TRUE(ValidateGreaterOrEqual<T>(arg)(lower));
    EXPECT_TRUE(ValidateGreater<T>(clamped_arg)(lower));
    EXPECT_TRUE(ValidateInterval<T>(lower, upper)(arg));
  }
};

TYPED_TEST_CASE(ValidationTest, MathTestTypes);
TYPED_TEST(ValidationTest, All)
{
  ValidationTest<TypeParam> k;
  k.TestBody();
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  Dune::MPIHelper::instance(argc, argv);
  return RUN_ALL_TESTS();
}