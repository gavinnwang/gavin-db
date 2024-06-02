#pragma once
#include "storage/page.hpp"
namespace db {
class BufferPoolManager;
class ReadPageGuard;
class WritePageGuard;

class BasicPageGuard {
  friend class ReadPageGuard;
  friend class WritePageGuard;

public:
  BasicPageGuard() = default;
  BasicPageGuard(BufferPoolManager &bpm, Page &page) : bpm_(&bpm), page_(&page) {}
  // copy constructor is disabled
  BasicPageGuard(const BasicPageGuard &) = delete;
  // move constructor, BPG(std::move(other_guard)), the other guard should not
  // be usable
  BasicPageGuard(BasicPageGuard &&that) noexcept;
  // move assignment operator, this should be dropped and become that
  auto operator=(BasicPageGuard &&that) noexcept -> BasicPageGuard &;
  ~BasicPageGuard();
  // basic page guard should be made invalid after this
  auto UpgradeRead() -> ReadPageGuard;
  auto UpgradeWrite() -> WritePageGuard;
  // clear all content and unpin page
  void Drop();

  auto PageId() -> page_id_t { return page_->GetPageId().page_number_; }
  auto GetData() -> const char * { return page_->GetData(); }
  template <class T> auto As() -> const T * {
    return reinterpret_cast<const T *>(GetData());
  }
  auto GetDataMut() -> char * {
    is_dirty_ = true;
    return page_->GetData();
  }
  template <class T> auto AsMut() -> T * {
    return reinterpret_cast<T *>(GetDataMut());
  }

private:
  // TODO convert to shared_ptr maybe?
  BufferPoolManager *bpm_{nullptr};
  Page *page_{nullptr};
  bool is_dirty_{false};
};
class ReadPageGuard {
public:
  ReadPageGuard() = default;
  ReadPageGuard(BufferPoolManager &bpm, Page &page) : guard_(bpm, page) {}
  ReadPageGuard(const ReadPageGuard &) = delete;
  auto operator=(const ReadPageGuard &) -> ReadPageGuard & = delete;
  ReadPageGuard(ReadPageGuard &&that) noexcept;
  auto operator=(ReadPageGuard &&that) noexcept -> ReadPageGuard &;
  void Drop();
  ~ReadPageGuard();
  auto PageId() -> page_id_t { return guard_.PageId(); }
  auto GetData() -> const char * { return guard_.GetData(); }
  template <class T> auto As() -> const T * { return guard_.As<T>(); }

private:
  BasicPageGuard guard_;
};
class WritePageGuard {
public:
  WritePageGuard() = default;
  WritePageGuard(BufferPoolManager &bpm, Page &page) : guard_(bpm, page) {}
  WritePageGuard(const WritePageGuard &) = delete;
  auto operator=(const WritePageGuard &) -> WritePageGuard & = delete;
  WritePageGuard(WritePageGuard &&that) noexcept;
  auto operator=(WritePageGuard &&that) noexcept -> WritePageGuard &;
  void Drop();
  ~WritePageGuard();
  auto PageId() -> page_id_t { return guard_.PageId(); }
  auto GetData() -> const char * { return guard_.GetData(); }
  template <class T> auto As() -> const T * { return guard_.As<T>(); }
  auto GetDataMut() -> char * { return guard_.GetDataMut(); }
  template <class T> auto AsMut() -> T * { return guard_.AsMut<T>(); }

private:
  BasicPageGuard guard_;
};
} // namespace db
