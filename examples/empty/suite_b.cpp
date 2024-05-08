#include <litetest.h>

TEST_SUITE(SuiteB);

TEST_CASE(CaseB) {
    EXPECT(3).to_be(2); // Fail expected
}