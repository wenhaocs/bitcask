#include <benchmark/benchmark.h>

#include "bitcask/DB.h"

const std::string DB_PATH = "/tmp/bitcask_benchmark";

void BM_Put(benchmark::State& state) {
  bitcask::Options options;
  options.maxFileSize = 64 * 1024 * 1024;  // 64MB max file size
  options.readOnly = false;
  auto dbRet = bitcask::DB::open(DB_PATH, options);

  if (!dbRet.ok()) {
    LOG(ERROR) << dbRet.status();
  }

  auto db = std::move(dbRet).value();

  bitcask::KeyType key = 1234;
  std::string value = std::string(state.range(0), 'x');  // value of size state.range(0)

  for (auto _ : state) {
    auto status = db->put(key, value);
    if (!status.ok()) {
      state.SkipWithError(status.toString().c_str());
    }
  }

  db->close();
}

void BM_Get(benchmark::State& state) {
  bitcask::Options options;
  options.maxFileSize = 64 * 1024 * 1024;
  options.readOnly = false;
  auto dbRet = bitcask::DB::open(DB_PATH, options);

  if (!dbRet.ok()) {
    LOG(ERROR) << dbRet.status();
  }

  auto db = std::move(dbRet).value();

  bitcask::KeyType key = 1234;
  std::string value = std::string(state.range(0), 'x');  // value of size state.range(0)

  // Prepopulate the database
  auto status = db->put(key, value);
  if (!status.ok()) {
    state.SkipWithError(status.toString().c_str());
  }

  for (auto _ : state) {
    auto valueRet = db->get(key);
    if (!valueRet.ok()) {
      state.SkipWithError(valueRet.status().toString().c_str());
    }
  }

  db->close();
}

void BM_ConcurrentPut(benchmark::State& state) {
  bitcask::Options options;
  options.maxFileSize = 64 * 1024 * 1024;  // 1MB max file size
  options.readOnly = false;
  auto dbRet = bitcask::DB::open(DB_PATH, options);

  if (!dbRet.ok()) {
    LOG(ERROR) << dbRet.status();
  }

  auto db = std::move(dbRet).value();

  std::vector<std::thread> threads;
  std::atomic<bitcask::KeyType> counter{0};
  std::string value = std::string(state.range(0), 'x');  // value of size state.range(0)

  for (auto _ : state) {
    for (int i = 0; i < state.range(1); ++i) {
      threads.emplace_back([&db, &counter, &value]() {
        bitcask::KeyType key = counter.fetch_add(1);
        auto status = db->put(key, value);
        if (!status.ok()) {
          LOG(ERROR) << status.toString();
        }
      });
    }

    for (auto& t : threads) {
      t.join();
    }
    threads.clear();
  }

  db->close();
}

BENCHMARK(BM_Put)->Arg(1024);                  // Test with 1KB value size
BENCHMARK(BM_Get)->Arg(1024);                  // Test with 1KB value size
BENCHMARK(BM_ConcurrentPut)->Args({1024, 8});  // Test with 1KB value size and 8 threads

BENCHMARK_MAIN();
