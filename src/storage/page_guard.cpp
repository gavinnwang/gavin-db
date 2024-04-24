#include "storage/page_guard.hpp"
#include "buffer/buffer_pool_manager.hpp"

namespace db {

BasicPageGuard::BasicPageGuard(BasicPageGuard &&that) noexcept {
  bpm_ = that.bpm_;
  page_ = that.page_;
  is_dirty_ = that.is_dirty_;
  that.page_ = nullptr;
  that.bpm_ = nullptr;
}

void BasicPageGuard::Drop() {
  if (page_ == nullptr) {
    return;
  }

  bpm_->UnpinPage(page_->GetPageId(), is_dirty_);
  page_ = nullptr;
  bpm_ = nullptr;
}

// move assignment operator
auto BasicPageGuard::operator=(BasicPageGuard &&that) noexcept
    -> BasicPageGuard & {
  if (this == &that) {
    return *this;
  }

  Drop();
  bpm_ = that.bpm_;
  page_ = that.page_;
  is_dirty_ = that.is_dirty_;

  that.page_ = nullptr;
  that.bpm_ = nullptr;

  return *this;
}

BasicPageGuard::~BasicPageGuard() { Drop(); };

ReadPageGuard::ReadPageGuard(ReadPageGuard &&that) noexcept = default;

auto ReadPageGuard::operator=(ReadPageGuard &&that) noexcept
    -> ReadPageGuard & {
  if (this == &that) {
    return *this;
  }

  Drop();
  guard_ = std::move(that.guard_);
  return *this;
}

void ReadPageGuard::Drop() {
  if (guard_.page_ != nullptr) {
    guard_.page_->RUnlatch();
  }
  guard_.Drop();
}

ReadPageGuard::~ReadPageGuard() { Drop(); }

WritePageGuard::WritePageGuard(WritePageGuard &&that) noexcept = default;

auto WritePageGuard::operator=(WritePageGuard &&that) noexcept
    -> WritePageGuard & {
  if (this == &that) {
    return *this;
  }

  Drop();
  guard_ = std::move(that.guard_);
  return *this;
}

void WritePageGuard::Drop() {
  if (guard_.page_ != nullptr) {
    guard_.page_->WUnlatch();
  }

  guard_.Drop();
}

WritePageGuard::~WritePageGuard() { Drop(); }

auto BasicPageGuard::UpgradeRead() -> ReadPageGuard {
  page_->RLatch();

  auto ret = ReadPageGuard{bpm_, page_};
  bpm_ = nullptr;
  page_ = nullptr;
  return ret;
}

auto BasicPageGuard::UpgradeWrite() -> WritePageGuard {
  page_->WLatch();

  auto ret = WritePageGuard{bpm_, page_};
  bpm_ = nullptr;
  page_ = nullptr;
  return ret;
}

} // namespace db
