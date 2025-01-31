#ifndef UTILS_TSCHELPER_H_
#define UTILS_TSCHELPER_H_

#include "bitcask/Base.h"

namespace bitcask {
namespace time {

#if defined(__x86_64__)
class TscHelper final {
 public:
  static uint64_t readTsc();
  // Interfaces to convert ticks to duration
  static uint64_t ticksToDurationInSec(uint64_t ticks);
  static uint64_t ticksToDurationInMSec(uint64_t ticks);
  static uint64_t ticksToDurationInUSec(uint64_t ticks);
  // Interface to convert tick to time point
  static uint64_t tickToTimePointInSec(uint64_t tick);
  static uint64_t tickToTimePointInMSec(uint64_t tick);
  static uint64_t tickToTimePointInUSec(uint64_t tick);

 private:
  TscHelper();
  ~TscHelper() = default;
  static TscHelper& get();
  uint64_t readTscImpl();
  void calibrate();

 private:
  std::chrono::steady_clock::time_point startMonoTime_;
  uint64_t firstTick_{0};
  struct timespec startRealTime_;

  std::atomic<double> ticksPerSecFactor_{0.0};
  std::atomic<double> ticksPerMSecFactor_{0.0};
  std::atomic<double> ticksPerUSecFactor_{0.0};
};
#endif  // defined(__x86_64__)

}  // namespace time
}  // namespace bitcask
#endif  // UTILS_TSCHELPER_H_
