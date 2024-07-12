#ifndef UTILS_WALLCLOCK_H_
#define UTILS_WALLCLOCK_H_

#include <time.h>

#include "bitcask/Base.h"

namespace bitcask {
namespace time {

/**
 * The class provides several utility methods to allow fast access the time
 *
 * The *fast* versions are way faster than the *slow* versions, but not as
 * precise as the *slow* versions. Choose wisely
 *
 */
class WallClock final {
 public:
  WallClock() = delete;

  static int64_t slowNowInSec();
  static int64_t fastNowInSec();

  static int64_t slowNowInMilliSec();
  static int64_t fastNowInMilliSec();

  static int64_t slowNowInMicroSec();
  static int64_t fastNowInMicroSec();
};

}  // namespace time
}  // namespace bitcask
#endif  // UTILS_WALLCLOCK_H_
