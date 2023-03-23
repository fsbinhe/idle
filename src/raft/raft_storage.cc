#include "raft_storage.h"
#include <sys/stat.h>
#include <glog/logging.h>
#include <memory>
#include <string>
#include <dirent.h>

namespace idle
{
  namespace distributed
  {
    void LogDB::Open(const std::string &path, const Option &option, LogDB **dbptr)
    {
      *dbptr = nullptr;
      LogDB *db = new LogDB();
      db->dbpath_ = path + "/";
      db->snapshot_interval_ = option.snapshot_interval_ * 1000;
      db->log_size_ = option.log_size_ << 20;
      // 0755: rwx
      mkdir(db->dbpath_.c_str(), 0755);
      if (!db->RecoverMarker())
      {
        LOG(WARNING) << "[raftd] RecoverMarker failed";
        delete db;
        return;
      }
      auto it = db->markers_.find(".smallest_index_");
      if (it != db->markers_.end())
      {
        db->smallest_index_ = std::atol(it->second.c_str());
      }
      if (!db->BuildFileCache())
      {
        LOG(WARNING) << "[raftd] BuildFileCache failed";
        delete db;
        return;
      }
      db->executor_ = std::make_unique<concurrency::Executor>();
      db->WriteMarkerSnapshot();
      *dbptr = db;
      return;
    }

    LogDB::LogDB() : next_index_(0),
                     smallest_index_(-1),
                     write_log_(nullptr),
                     write_index_(nullptr),
                     marker_log_(nullptr)
    {
    }
    LogDB::~LogDB() {}

    StatusCode LogDB::Write(int64_t index, const std::string &entry)
    {
      std::lock_guard<std::mutex> guard(mu_);
      if (index != next_index_ && smallest_index_ != -1)
      {
        LOG(WARNING) << "[raftd] Write with invalid index = " << index << " smallest_index = " << smallest_index_ << " next_index = " << next_index_;
        return kBadParemeter;
      }
      // Empty db
      if (smallest_index_ == -1)
      {
        StatusCode s = WriteMarker(".smallest_index_", std::to_string(index));
        if (s != kOK)
        {
          return s;
        }
        smallest_index_ = index;
        LOG(INFO) << "[raftd] smallest_index_ to " << smallest_index_;
      }
      uint32_t len = entry.length();
      // Serialization in cpp way
      // "abc" => 00000000  03 00 00 00 61 62 63                                 ....abc
      std::string data;
      data.append(reinterpret_cast<char *>(&len), sizeof(len));
      data.append(entry);
      // If there is no log, create one
      if (!write_log_)
      {
        if (!NewWriteLog(index))
        {
          return kWriteError;
        }
      }
      int64_t offset = ftell(write_log_);
      // If one log is too big, then
      // create a new one
      if (offset > log_size_)
      {
        if (!NewWriteLog(index))
        {
          return kWriteError;
        }
        offset = 0;
      }
      if (fwrite(data.c_str(), 1, data.length(), write_log_) != data.length() || fflush(write_log_) != 0)
      {
        LOG(WARNING) << "[raftd] Write log " << index << " failed";
        CloseCurrent();
        return kWriteError;
      }
      if (fwrite(reinterpret_cast<char *>(&index), 1, 8, write_index_) != 8)
      {
        LOG(WARNING) << "[raftd] Write index " << index << " failed";
        CloseCurrent();
        return kWriteError;
      }
      if (fwrite(reinterpret_cast<char *>(&offset), 1, 8, write_index_) != 8 || fflush(write_index_) != 0)
      {
        LOG(WARNING) << "[raftd] Write index " << index << "failed";
        CloseCurrent();
        return kWriteError;
      }
      next_index_ = index + 1;
      return kOK;
    }

    StatusCode LogDB::Read(int64_t index, std::string *entry)
    {
      std::lock_guard<std::mutex> lock(mu_);
      if (read_log_.empty() || index >= next_index_ || index < smallest_index_)
      {
        LOG(WARNING) << "[raftd] Read with invalid index = " << index << " next_index = " << next_index_ << " smallest_index_ " << smallest_index_;
        return kNotFound;
      }
      FileCache::iterator it = read_log_.lower_bound(index);
      if (it == read_log_.end() || (it != read_log_.begin() && index != it->first))
      {
        --it;
      }
      if (index < it->first)
      {
        LOG(WARNING) << "[raftd] Read connot find index file " << index;
        return kReadError;
      }
      FILE *idx_fp = (it->second).first;
      FILE *log_fp = (it->second).second;
      int offset = 16 * (index - it->first);
      int64_t read_index = -1;
      int64_t entry_offset = -1;

      if (fseek(idx_fp, offset, SEEK_SET) != 0)
      {
        LOG(WARNING) << "[raftd] Read cannot find index " << index;
        return kReadError;
      }
      StatusCode s = ReadIndex(idx_fp, index, &read_index, &entry_offset);
      if (s != kOK)
      {
        return s;
      }
      if (fseek(log_fp, entry_offset, SEEK_SET) != 0)
      {
        LOG(WARNING) << "[raftd] Read " << index << " with invalid offset " << entry_offset;
        return kReadError;
      }
      int ret = ReadOne(log_fp, entry);
      if (ret <= 0)
      {
        LOG(WARNING) << "[raftd] Read log error " << index;
        return kReadError;
      }

      return kOK;
    }

    bool LogDB::BuildFileCache()
    {
      LOG(INFO) << "[raftd] BuildFileCache";
      struct dirent *entry = nullptr;
      DIR *dir_ptr = opendir(dbpath_.c_str());
      if (dir_ptr == nullptr)
      {
        LOG(WARNING) << "[raftd] BuildFileCache failed " << dbpath_.c_str();
        return false;
      }
      bool error = false;
      while ((entry = readdir(dir_ptr)) != nullptr)
      {
        size_t idx = std::string(entry->d_name).find(".idx");
        if (idx != std::string::npos)
        {
          std::string file_name = std::string(entry->d_name);
          int64_t index = std::atol(file_name.substr(0, idx).c_str());
          std::string log_name, idx_name;
          // FormLogName(index, &log_name, &idx_name);
          FILE *idx_fp;
          FILE *log_fp;
          if (!OpenFile(&idx_fp, idx_name, "r"))
          {
            error = true;
            break;
          }
          if (!OpenFile(&log_fp, log_name, "r"))
          {
            fclose(idx_fp);
            error = true;
            break;
          }
          read_log_[index] = std::make_pair(idx_fp, log_fp);
          LOG(INFO) << "[raftd] BuildFileCache index = " << index << " for " << file_name.c_str();
        }
      }
      closedir(dir_ptr);
      if (error || !CheckLogIndex())
      {
        LOG(WARNING) << "[raftd] BuildFileCache failed error = " << error;
        for (FileCache::iterator it = read_log_.begin(); it != read_log_.end(); ++it)
        {
          fclose((it->second).first);
          fclose((it->second).second);
          return false;
        }
        return true;
      }
    }

    void LogDB::EncodeMarker(const MarkerEntry &marker, std::string *data)
    {
      int klen = (marker.key_).length();
      int vlen = (marker.val_).length();
      data->append(reinterpret_cast<char *>(&klen), sizeof(klen));
      data->append(marker.key_);
      data->append(reinterpret_cast<char *>(&vlen), sizeof(vlen));
      data->append(marker.val_);
    }

    StatusCode LogDB::WriteMarkerNoLock(const std::string &key, const std::string &value)
    {
      if (marker_log_ == nullptr)
      {
        marker_log_ = fopen((dbpath_ + "marker.mak").c_str(), "a");
        if (marker_log_ == nullptr)
        {
          LOG(WARNING) << "[raftd] WriteMarkerNoLock open marker.mak failed errormsg = " << strerror(errno);
          return kWriteError;
        }
      }

      std::string data;
      uint32_t len = 4 + key.length() + 4 + value.length();
      data.append(reinterpret_cast<char *>(&len), sizeof(len));
      EncodeMarker(MarkerEntry(key, value), &data);
      if (fwrite(data.c_str(), 1, data.length(), marker_log_) != data.length() || fflush(marker_log_) != 0)
      {
        LOG(WARNING) << "[raftd] WriterMarkerNoLock failed key = " << key << " value = " << value;
        return kWriteError;
      }

      fflush(marker_log_);
      markers_[key] = value;
      return kOK;
    }

    StatusCode LogDB::WriteMarker(const std::string &key, const std::string &value)
    {
      std::lock_guard<std::mutex> l(mu_);
      return WriteMarkerNoLock(key, value);
    }

    StatusCode LogDB::WriteMarker(const std::string &key, int64_t value)
    {
      return WriteMarker(key, std::string(reinterpret_cast<char *>(&value), sizeof(value)));
    }

    StatusCode LogDB::ReadMarker(const std::string &key, std::string *value)
    {
      std::lock_guard<std::mutex> l(mu_);
      std::map<std::string, std::string>::iterator it = markers_.find(key);
      if (it == markers_.end())
      {
        return kNotFound;
      }
      *value = it->second;
      return kOK;
    }

    StatusCode LogDB::ReadMarker(const std::string &key, int64_t *value)
    {
      std::lock_guard<std::mutex> l(mu_);
      std::string v;
      auto status = ReadMarker(key, &v);
      if (status != kOK)
      {
        return status;
      }
      memcpy(value, &(v[0]), 8);
      return kOK;
    }

    StatusCode LogDB::GetLargestIndex(int64_t *value)
    {
      std::lock_guard<std::mutex> l(mu_);
      if (smallest_index_ == next_index_)
      {
        *value = -1;
        return kNotFound;
      }
      *value = next_index_ - 1;
      return kOK;
    }

    void LogDB::FormLogName(int64_t index, std::string *log_name, std::string *idx_name)
    {
      log_name->clear();
      log_name->append(dbpath_);
      log_name->append(std::to_string(index));
      log_name->append(".log");

      idx_name->clear();
      idx_name->append(dbpath_);
      idx_name->append(std::to_string(index));
      idx_name->append(".idx");
    }

    StatusCode LogDB::DeleteUpTo(int64_t index)
    {
      if (index < smallest_index_)
      {
        return kOK;
      }

      if (index >= next_index_)
      {
        LOG(INFO) << "[raftd] DeleteUpTo over limit index = " << index << " next_index_ = " << next_index_;
        return kBadParemeter;
      }

      std::lock_guard<std::mutex> l(mu_);
      smallest_index_ = index + 1;
      WriteMarkerNoLock(".smallest_index_", std::to_string(smallest_index_));
      FileCache::reverse_iterator upto = read_log_.rbegin();
      while (upto != read_log_.rend())
      {
        if (upto->first <= index)
        {
          break;
        }
        ++upto;
      }
      if (upto == read_log_.rend())
      {
        return kOK;
      }
      int64_t upto_index = upto->first;
      FileCache::iterator it = read_log_.begin();
      while (it->first != upto_index)
      {
        std::string log_name, idx_name;
        FormLogName(it->first, &log_name, &idx_name);
        if (!RemoveFile((it->second).first, log_name) || !RemoveFile((it->second).second, idx_name))
        {
          return kWriteError;
        }
        read_log_.erase(it++);
      }
      LOG(INFO) << "[raftd] DeleteUpTo done smallest_index_ = " << smallest_index_ << " next_index_ = " << next_index_;
      return kOK;
    }

    bool LogDB::CloseFile(FILE *fp, const std::string &name)
    {
      if (!fp)
      {
        return true;
      }

      if (fclose(fp) != 0)
      {
        LOG(ERROR) << "[raftd] CloseFile failed errormsg = " << strerror(errno);
        return false;
      }

      LOG(DEBUG) << "[raftd] CloseFile file = " << name;
      return true;
    }

    bool LogDB::OpenFile(FILE **fp, const std::string &name, const char *mode)
    {
      *fp = fopen(name.c_str(), mode);
      if (*fp == nullptr)
      {
        LOG(ERROR) << "[raftd] OpenFile failed errormsg = " << strerror(errno);
        return false;
      }
      LOG(DEBUG) << "[raftd] OpenFile file = " << name;
      return true;
    }

    bool LogDB::RemoveFile(const std::string &name)
    {
      if (remove(name.c_str()) != 0)
      {
        LOG(WARN) << "[raftd] RemoveFile failed errormsg = " << strerror(errno);
        return false;
      }
      LOG(DEBUG) << "[raftd] RemoveFile file = " << name;
      return true;
    }
  } // namespace distributed
} // namespace baidu;