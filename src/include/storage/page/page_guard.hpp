#pragma once
#include "storage/page/page.hpp"
namespace db {
class BufferPool;
class ReadPageGuard;
class WritePageGuard;

class BasicPageGuard {
	friend class ReadPageGuard;
	friend class WritePageGuard;

public:
	BasicPageGuard() = default;
	BasicPageGuard(BufferPool &bpm, Page &page) : bpm_(&bpm), page_(&page) {
	}
	// copy constructor is disabled
	BasicPageGuard(const BasicPageGuard &) = delete;
	// move constructor, BPG(std::move(other_guard)), the other guard should not
	// be usable
	BasicPageGuard(BasicPageGuard &&that) noexcept;
	// move assignment operator, this should be dropped and become that
	BasicPageGuard &operator=(BasicPageGuard &&that) noexcept;
	~BasicPageGuard();
	// basic page guard should be made invalid after this
	[[nodiscard]] ReadPageGuard UpgradeRead();
	[[nodiscard]] WritePageGuard UpgradeWrite();
	// clear all content and unpin page
	void Drop();

	page_id_t PageId() {
		return page_->GetPageId().page_number_;
	}
	const char *GetData() {
		return page_->GetData();
	}
	template <class T>
	[[nodiscard]] const T &As() {
		return reinterpret_cast<const T &>(*GetData());
	}
	[[nodiscard]] char *GetDataMut() {
		is_dirty_ = true;
		return page_->GetData();
	}
	template <class T>
	[[nodiscard]] T &AsMut() {
		return reinterpret_cast<T &>(*GetDataMut());
	}

private:
	// TODO(gavinnwang): convert to shared_ptr maybe?
	BufferPool *bpm_ {nullptr};
	Page *page_ {nullptr};
	bool is_dirty_ {false};
};
class WritePageGuard {
public:
	WritePageGuard() = default;
	WritePageGuard(BufferPool &bpm, Page &page) : guard_(bpm, page) {
	}
	WritePageGuard(const WritePageGuard &) = delete;
	WritePageGuard &operator=(const WritePageGuard &) = delete;
	WritePageGuard(WritePageGuard &&that) noexcept;
	WritePageGuard &operator=(WritePageGuard &&that) noexcept;
	void Drop();
	~WritePageGuard();
	page_id_t PageId() {
		return guard_.PageId();
	}
	const char *GetData() {
		return guard_.GetData();
	}
	template <class T>
	[[nodiscard]] const T &As() {
		return guard_.As<T>();
	}
	[[nodiscard]] char *GetDataMut() {
		return guard_.GetDataMut();
	}
	template <class T>
	[[nodiscard]] T &AsMut() {
		return guard_.AsMut<T>();
	}

private:
	BasicPageGuard guard_;
};
class ReadPageGuard {
public:
	ReadPageGuard() = default;
	ReadPageGuard(BufferPool &bpm, Page &page) : guard_(bpm, page) {
	}
	ReadPageGuard(const ReadPageGuard &) = delete;
	ReadPageGuard &operator=(const ReadPageGuard &) = delete;
	ReadPageGuard(ReadPageGuard &&that) noexcept;
	ReadPageGuard &operator=(ReadPageGuard &&that) noexcept;
	void Drop();
	~ReadPageGuard();

	WritePageGuard UpgradeWrite() {
		guard_.page_->RUnlatch();
		return guard_.UpgradeWrite();
	}
	[[nodiscard]] page_id_t PageId() {
		return guard_.PageId();
	}
	[[nodiscard]] const char *GetData() {
		return guard_.GetData();
	}
	template <class T>
	[[nodiscard]] const T &As() {
		return guard_.As<T>();
	}

private:
	BasicPageGuard guard_;
};
} // namespace db
