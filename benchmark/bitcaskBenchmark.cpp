#include <benchmark/benchmark.h>

#include "bitcask/DB.h"

class DBBenchmark : public benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State& state) override {
    if (!initialized.load(std::memory_order_acquire)) {
      std::lock_guard<std::mutex> lock(db_mutex);
      if (!initialized.load(std::memory_order_relaxed)) {
        db = openDatabaseAndLoadData(numKeys, state.range(0));
        initialized.store(true, std::memory_order_release);
      }
    }
  }

  void TearDown(const ::benchmark::State& state) override {
    if (state.thread_index() == 0) {
      std::lock_guard<std::mutex> lock(db_mutex);
      if (initialized.load(std::memory_order_relaxed)) {
        db->close();
        db.reset();
        initialized.store(false, std::memory_order_release);
      }
    }
  }

  std::unique_ptr<bitcask::DB> openDatabase() {
    bitcask::Options options;
    options.maxFileSize = 64 * 1024 * 1024;  // 64MB max file size
    options.readOnly = false;

    auto dbRet = bitcask::DB::open(DB_PATH, options);
    if (!dbRet.ok()) {
      LOG(ERROR) << dbRet.status();
      return nullptr;
    }
    return std::move(dbRet).value();
  }

  std::unique_ptr<bitcask::DB> openDatabaseAndLoadData(size_t numKeys, size_t valueSize) {
    auto db = openDatabase();
    std::string value = std::string(valueSize, 'x');
    for (size_t i = 0; i < numKeys; ++i) {
      bitcask::KeyType key = static_cast<bitcask::KeyType>(i);
      auto status = db->put(key, value);
      if (!status.ok()) {
        LOG(ERROR) << status.toString();
        return nullptr;
      }
    }
    return db;
  }

  std::unique_ptr<bitcask::DB> db;
  std::atomic<bool> initialized{false};
  std::mutex db_mutex;
  const size_t numKeys = 10000;
  const std::string DB_PATH = "/tmp/bitcask_benchmark";
};

// put values
BENCHMARK_DEFINE_F(DBBenchmark, put)(benchmark::State& state) {
  std::string value = std::string(state.range(0), 'x');

  int64_t iterIndex = 0;

  for (auto _ : state) {
    // Each thread will perform this work concurrently
    bitcask::KeyType key = state.thread_index() * state.iterations() + iterIndex;
    auto status = db->put(key, value);
    if (!status.ok()) {
      state.SkipWithError(status.toString().c_str());
    }
    iterIndex++;
  }
}

// get values
BENCHMARK_DEFINE_F(DBBenchmark, get)(benchmark::State& state) {
  int64_t iterIndex = 0;

  for (auto _ : state) {
    auto valueRet = db->get(iterIndex % numKeys);
    if (!valueRet.ok()) {
      state.SkipWithError(valueRet.status().toString().c_str());
    }
    iterIndex++;
  }
}

// Test value size from 64B to 4KB
BENCHMARK_REGISTER_F(DBBenchmark, put)->RangeMultiplier(4)->Range(64, 4096);

// Test 1KB value size with 10 threads
BENCHMARK_REGISTER_F(DBBenchmark, put)->Arg(1024)->Threads(10);

// Test value size from 64B to 4KB
BENCHMARK_REGISTER_F(DBBenchmark, get)->RangeMultiplier(4)->Range(64, 4096);

// // Test 1KB value size with 10 threads
BENCHMARK_REGISTER_F(DBBenchmark, get)->Arg(1024)->Threads(10);

BENCHMARK_MAIN();
