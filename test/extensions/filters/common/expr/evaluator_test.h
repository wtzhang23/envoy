#pragma once

namespace Envoy {
namespace Extensions {
namespace Filters {
namespace Common {
namespace Expr {

// random()
inline constexpr char RandomCelString[] = R"pb(
  expr: {
    id: 1
    call_expr: {
      function: "random"
    }
  }
  source_info: {
    location: "<input>"
    line_offsets: 9
    positions: {
      key: 1
      value: 6
    }
  }
)pb";

} // namespace Expr
} // namespace Common
} // namespace Filters
} // namespace Extensions
} // namespace Envoy
