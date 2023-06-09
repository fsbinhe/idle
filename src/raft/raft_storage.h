#pragma once

#include <map>
#include <mutex>
#include <stdio.h>

#include "../concurrency/executor.h"
#include <folly/executors/GlobalExecutor.h>

namespace idle {
namespace distributed {
struct Option {
  int64_t snapshot_interval_; // second
  int64_t log_size_;          // MB
  Option() : snapshot_interval_(60), log_size_(128) {}
};

struct MarkerEntry {
  std::string key_;
  std::string val_;
  MarkerEntry() {}
  MarkerEntry(const std::string &key, const std::string &value)
      : key_(key), val_(value) {}
};

enum StatusCode {
  kBadParemeter = 0,
  kOK = 1,
  kWriteError = 2,
  kNotFound = 3,
  kReadError = 4,
};

// LogDB contains three kinds of files
// 1. marker: which is used to record the last successful write, when BFS
// restarts, it will use marker file to recover.
// 2. log: which is used to record log content.
// 3. index: which is used to record log's position to increase the speed to
// read.
class LogDB {
public:
  LogDB();
  ~LogDB();
  // Open
  // @param path: the database file path
  // @param option: the option that is used to customise the Open behaviour
  // @param dbptr: the database handle for users to use
  static void Open(const std::string &path, const Option &option,
                   LogDB **dbptr);

  // DestroyDB
  // @param path: the database file path
  static StatusCode DestroyDB(const std::string &path);

  StatusCode Write(int64_t index, const std::string &entry);
  StatusCode Read(int64_t index, std::string *entry);

  StatusCode WriteMarker(const std::string &key, const std::string &value);
  StatusCode WriteMarker(const std::string &key, int64_t value);
  StatusCode WriteMarkerNoLock(const std::string &key,
                               const std::string &value);

  StatusCode ReadMarker(const std::string &key, std::string *value);
  StatusCode ReadMarker(const std::string &key, int64_t *value);
  StatusCode ReadIndex(FILE *fp, int64_t expect_index, int64_t *index,
                       int64_t *offset);

  int ReadOne(FILE *fp, std::string *data);

  StatusCode GetLargestIndex(int64_t *value);
  StatusCode DeleteUpTo(int64_t index);
  StatusCode DeleteFrom(int64_t index);

private:
  bool RecoverMarker();
  bool BuildFileCache();
  bool CheckLogIndex();
  void WriteMarkerSnapshot();
  void CloseCurrent();

  void EncodeMarker(const MarkerEntry &marker, std::string *data);
  void DecodeMarker(const std::string &data, MarkerEntry *marker);

  bool NewWriteLog(int64_t index);
  void FormLogName(int64_t index, std::string *log_name, std::string *idx_name);

private:
  bool OpenFile(FILE **fp, const std::string &name, const char *mode);
  bool CloseFile(FILE *fp, const std::string &hint);
  bool RemoveFile(const std::string &name);
  bool RemoveFile(FILE *fp, const std::string &name);

private:
  std::string dbpath_;
  std::mutex mu_;
  std::unique_ptr<concurrency::Executor> executor_;

  int64_t snapshot_interval_;
  int64_t log_size_;
  std::map<std::string, std::string> markers_;
  int64_t next_index_;
  int64_t smallest_index_;

  using FileCache = std::map<int64_t, std::pair<FILE *, FILE *>>;
  FILE *write_log_;
  FILE *write_index_;
  FileCache read_log_;
  FILE *marker_log_;
};
} // namespace distributed
} // namespace idle