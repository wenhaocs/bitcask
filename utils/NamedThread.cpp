#include "utils/NamedThread.h"

namespace nebula {
namespace thread {

class TLSThreadID {
 public:
  TLSThreadID() {
    tid_ = ::syscall(SYS_gettid);
  }

  ~TLSThreadID() {
    tid_ = 0;
  }

  pid_t tid() {
    return tid_;
  }

 private:
  pid_t tid_;
};

pid_t gettid() {
  static thread_local TLSThreadID tlstid;
  return tlstid.tid();
}

}  // namespace thread
}  // namespace nebula
