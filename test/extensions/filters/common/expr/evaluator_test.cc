#include "test/extensions/filters/common/expr/evaluator_test.h"

#include "source/extensions/filters/common/expr/evaluator.h"

#include "test/mocks/common.h"
#include "test/mocks/stream_info/mocks.h"
#include "test/test_common/utility.h"

#include "absl/time/time.h"
#include "eval/public/structs/cel_proto_wrapper.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace Filters {
namespace Common {
namespace Expr {
namespace {

using ::testing::MatchesRegex;

TEST(Evaluator, Print) {
  EXPECT_EQ(print(CelValue::CreateBool(false)), "false");
  EXPECT_EQ(print(CelValue::CreateInt64(123)), "123");
  EXPECT_EQ(print(CelValue::CreateUint64(123)), "123");
  EXPECT_EQ(print(CelValue::CreateDouble(1.23)), "1.23");

  std::string test = "test";
  EXPECT_EQ(print(CelValue::CreateString(&test)), "test");
  EXPECT_EQ(print(CelValue::CreateBytes(&test)), "test");

  ProtobufWkt::Arena arena;
  envoy::config::core::v3::Node node;
  std::string node_yaml = "id: test";
  TestUtility::loadFromYaml(node_yaml, node);
  EXPECT_EQ(print(CelValue::CreateNull()), "NULL");
  EXPECT_THAT(print(google::api::expr::runtime::CelProtoWrapper::CreateMessage(&node, &arena)),
              MatchesRegex(".*id:\\s+\"test\""));

  EXPECT_EQ(print(CelValue::CreateDuration(absl::Minutes(1))), "1m");
  absl::Time time = TestUtility::parseTime("Dec 22 01:50:34 2020 GMT", "%b %e %H:%M:%S %Y GMT");
  EXPECT_EQ(print(CelValue::CreateTimestamp(time)), "2020-12-22T01:50:34+00:00");

  absl::Status status = absl::UnimplementedError("unimplemented");
  EXPECT_EQ(print(CelValue::CreateError(&status)), "CelError value");
}

TEST(Evaluator, Activation) {
  NiceMock<StreamInfo::MockStreamInfo> info;
  auto filter_state =
      std::make_shared<StreamInfo::FilterStateImpl>(StreamInfo::FilterState::LifeSpan::FilterChain);
  info.upstreamInfo()->setUpstreamFilterState(filter_state);
  ProtobufWkt::Arena arena;
  const auto activation = createActivation(nullptr, info, nullptr, nullptr, nullptr);
  EXPECT_TRUE(activation->FindValue("filter_state", &arena).has_value());
  EXPECT_TRUE(activation->FindValue("upstream_filter_state", &arena).has_value());
}

TEST(Evaluator, RandomFunction) {
  Protobuf::Arena arena;
  auto builder = createBuilder(&arena);

  google::api::expr::v1alpha1::CheckedExpr checked;
  Protobuf::TextFormat::ParseFromString(RandomCelString, &checked);
  auto expr = createExpression(*builder, checked.expr());

  NiceMock<StreamInfo::MockStreamInfo> info;
  NiceMock<Random::MockRandomGenerator> random;
  EXPECT_CALL(random, random()).WillOnce(testing::Return(42));
  auto value = evaluate(*expr, arena, nullptr, info, nullptr, nullptr, nullptr, random);
  EXPECT_TRUE(value.has_value());
  ASSERT_TRUE(value.value().IsUint64());
  EXPECT_EQ(42, value.value().Uint64OrDie());

  EXPECT_CALL(random, random()).WillOnce(testing::Return(43));
  value = evaluate(*expr, arena, nullptr, info, nullptr, nullptr, nullptr, random);
  EXPECT_TRUE(value.has_value());
  ASSERT_TRUE(value.value().IsUint64());
  EXPECT_EQ(43, value.value().Uint64OrDie());
}

} // namespace
} // namespace Expr
} // namespace Common
} // namespace Filters
} // namespace Extensions
} // namespace Envoy
