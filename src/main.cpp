#include "common/Base.h"

int main(int, char **) {
  LOG(INFO) << "test";
  FLOG_INFO("This is an info message with number: {}", 42);
  // FLOG_WARN("This is a warning message with number: {}", 42);
  // FLOG_ERROR("This is an error message with number: {}", 42);
  // // FLOG_FATAL("This is a fatal message with number: {}", 42); // Uncomment to test fatal
  // logging

  // FVLOG1("This is a verbose level 1 message with number: {}", 42);
  // FVLOG2("This is a verbose level 2 message with number: {}", 42);
  // FVLOG3("This is a verbose level 3 message with number: {}", 42);
  // FVLOG4("This is a verbose level 4 message with number: {}", 42);
  return 0;
}
