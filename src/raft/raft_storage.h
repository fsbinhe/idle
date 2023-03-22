#pragma once

#include <mutex>
#include <map>
#include <stdio.h>

#include "../concurrency/executor.h"

namespace idle
{
  namespace distributed
  {
    struct Option
    {
      int64_t snapshot_interval_; // second
      int64_t log_size_;          // MB
      Option() : snapshot_interval_(60), log_size_(128) {}
    };

    struct MarkerEntry
    {
      std::string key_;
      std::string val_;
      MarkerEntry() {}
      MarkerEntry(const std::string &key, const std::string &value) : key_(key), val_(value) {}
    };

    enum StatusCode
    {
      kBadParemeter = 0,
      kOK = 1,
      kWriteError = 2,
      kNotFound = 3,
      kReadError = 4,
    };

    class LogDB
    {
    public:
      LogDB();
      ~LogDB();

      static void Open(const std::string &path, const Option &option, LogDB **dbptr);
      static StatusCode DestroyDB(const std::string &path);

      StatusCode Write(int64_t index, const std::string &entry);
      StatusCode Read(int64_t index, std::string *entry);

      StatusCode WriteMarker(const std::string &key, const std::string &value);
      StatusCode WriteMarker(const std::string &key, int64_t value);

      StatusCode ReadMarker(const std::string &key, std::string *value);
      StatusCode ReadMarker(const std::string &key, int64_t *value);
      StatusCode ReadIndex(FILE *fp, int64_t expect_index, int64_t *index, int64_t *offset);
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
      bool NewWriteLog(int64_t index);
      bool CloseFile(FILE *fp, const std::string &hint);
      bool OpenFile(FILE **fp, const std::string &name, const char *mode);
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
  }
}