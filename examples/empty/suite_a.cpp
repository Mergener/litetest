#include <litetest.h>

TEST_SUITE(SuiteA);

TEST_CASE(CaseA) {
    EXPECT(2).to_be(2); // Success expected
}
