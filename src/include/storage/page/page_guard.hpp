#pragma once
#include "storage/page/page.hpp"
namespace db {
class BufferPoolManager;
class ReadPageGuard;
class WritePageGuard;

class BasicPageGuard {
	friend class ReadPageGuard;
	friend class WritePageGuard;

public:
	BasicPageGuard() = default;
	BasicPageGuard(BufferPoolManager &bpm, Page &page) : bpm_(&bpm), page_(&page) {
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
	ReadPageGuard UpgradeRead();
	WritePageGuard UpgradeWrite();
	// clear all content and unpin page
	void Drop();

	page_id_t PageId() {
		return page_->GetPageId().page_number_;
	}
	const char *GetData() {
		return page_->GetData();
	}
	template <class T>
	const T *As() {
		return reinterpret_cast<const T *>(GetData());
	}
	char *GetDataMut() {
		is_dirty_ = true;
		return page_->GetData();
	}
	template <class T>
	T *AsMut() {
		return reinterpret_cast<T *>(GetDataMut());
	}

private:
	// TODO convert to shared_ptr maybe?
	BufferPoolManager *bpm_ {nullptr};
	Page *page_ {nullptr};
	bool is_dirty_ {false};
};
class ReadPageGuard {
public:
	ReadPageGuard() = default;
	ReadPageGuard(BufferPoolManager &bpm, Page &page) : guard_(bpm, page) {
	}
	ReadPageGuard(const ReadPageGuard &) = delete;
	ReadPageGuard &operator=(const ReadPageGuard &) = delete;
	ReadPageGuard(ReadPageGuard &&that) noexcept;
	ReadPageGuard &operator=(ReadPageGuard &&that) noexcept;
	void Drop();
	~ReadPageGuard();
	page_id_t PageId() {
		return guard_.PageId();
	}
	const char *GetData() {
		return guard_.GetData();
	}
	template <class T>
	const T *As() {
		return guard_.As<T>();
	}

private:
	BasicPageGuard guard_;
};
class WritePageGuard {
public:
	WritePageGuard() = default;
	WritePageGuard(BufferPoolManager &bpm, Page &page) : guard_(bpm, page) {
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
	const T *As() {
		return guard_.As<T>();
	}
	char *GetDataMut() {
		return guard_.GetDataMut();
	}
	template <class T>
	T *AsMut() {
		return guard_.AsMut<T>();
	}

private:
	BasicPageGuard guard_;
};
} // namespace db
