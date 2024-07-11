#ifndef BITCASK_OPTIONS_H_
#define BITCASK_OPTIONS_H_

#include "bitcask/Base.h"

namespace bitcask {

// Options to control the behavior of a database (passed to DB::Open)
struct Options {
  // Create an Options object with default values for all fields.
  Options() = default;

  // If true, the database is open in read only mode.
  bool readOnly = false;

  // If this writer would prefer to sync the write file after every write operation
  bool syncOnPut = false;

  // Max data file size in bytes.
  size_t maxFileSize = 64 * 1024 * 1024;
};

}  // namespace bitcask

#endif  // BITCASK_OPTIONS_H_
