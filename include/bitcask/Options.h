#ifndef BITCASK_OPTIONS_H_
#define BITCASK_OPTIONS_H_

#include "bitcask/Base.h"

namespace bitcask {

// Options to control the behavior of a database (passed to DB::Open)
struct Options {
  // Create an Options object with default values for all fields.
  Options() = default;

  // If true, the database will be created if it is missing.
  bool persist = false;

  size_t maxFileSize = 2 * 1024 * 1024;
};

}  // namespace bitcask

#endif  // BITCASK_OPTIONS_H_
