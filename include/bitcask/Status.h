#ifndef BITCASK_STATUS_H_
#define BITCASK_STATUS_H_

#include "bitcask/Base.h"

namespace bitcask {

template <typename T>
class StatusOr;

class Status final {
 public:
  Status() = default;

  ~Status() = default;

  Status(const Status &rhs) {
    state_ = (rhs.state_ == nullptr ? nullptr : copyState(rhs.state_.get()));
  }

  Status &operator=(const Status &rhs) {
    // `state_ == rhs.state_' means either `this == &rhs',
    // or both `*this' and `rhs' are OK
    if (state_ != rhs.state_) {
      state_ = rhs.state_ == nullptr ? nullptr : copyState(rhs.state_.get());
    }
    return *this;
  }

  Status(Status &&rhs) noexcept {
    state_ = std::move(rhs.state_);
  }

  Status &operator=(Status &&rhs) noexcept {
    // `state_ == rhs.state_' means either `this == &rhs',
    // or both `*this' and `rhs' are OK
    if (state_ != rhs.state_) {
      state_ = std::move(rhs.state_);
    }
    return *this;
  }

  static Status from(const Status &s) {
    return s;
  }

  template <typename T>
  static Status from(StatusOr<T> &&s) {
    return std::move(s).status();
  }

  template <typename T>
  static Status from(const StatusOr<T> &s) {
    return s.status();
  }

  bool operator==(const Status &rhs) const {
    // `state_ == rhs.state_' means either `this == &rhs',
    // or both `*this' and `rhs' are OK
    if (state_ == rhs.state_) {
      return true;
    }
    return code() == rhs.code();
  }

  bool operator!=(const Status &rhs) const {
    return !(*this == rhs);
  }

  bool ok() const {
    return state_ == nullptr;
  }

  static Status OK() {
    return Status();
  }

  std::string toString() const {
    Code code = this->code();
    if (code == kOk) {
      return "OK";
    }
    std::string result(toString(code));
    result.append(&state_[kHeaderSize], size());
    return result;
  }

  friend std::ostream &operator<<(std::ostream &os, const Status &status);

  // If some kind of error really needs to be distinguished with others using a
  // specific code, other than a general code and specific msg, you could add a
  // new code below, e.g. kSomeError, and add the corresponding
  // STATUS_GENERATOR(SomeError)
  enum Code : uint16_t {
    // OK
    kOk = 0,
    // 1xx, for data file errors
    kOpenFileError = 101,
    kNoSuchFile = 102,
    // kNotSupported = 103,

    kError = 999,
  };

  Code code() const {
    if (state_ == nullptr) {
      return kOk;
    }
    return reinterpret_cast<const Header *>(state_.get())->code_;
  }

  std::string message() const {
    if (state_ == nullptr) return "";
    return std::string(&state_[kHeaderSize], size());
  };

  static Status ERROR(Code code, std::string msg) {
    return Status(code, msg);
  }

 private:
  // REQUIRES: stat_ != nullptr
  uint16_t size() const {
    return reinterpret_cast<const Header *>(state_.get())->size_;
  }

  Status(Code code, std::string msg) {
    const uint16_t size = msg.size();
    auto state = std::unique_ptr<char[]>(new char[size + kHeaderSize]);
    auto *header = reinterpret_cast<Header *>(state.get());
    header->size_ = size;
    header->code_ = code;
    ::memcpy(&state[kHeaderSize], msg.c_str(), size);
    state_ = std::move(state);
  }

  static std::unique_ptr<const char[]> copyState(const char *state) {
    const auto size = *reinterpret_cast<const uint16_t *>(state);
    const auto total = size + kHeaderSize;
    auto result = std::unique_ptr<char[]>(new char[total]);
    ::memcpy(&result[0], state, total);
    return result;
  }

  static std::string format(const char *fmt, va_list args) {
    char result[256];
    vsnprintf(result, sizeof(result), fmt, args);
    return result;
  }

  static const char *toString(Code code) {
    switch (code) {
      case kOk:
        return "OK";
      case kOpenFileError:
        return "Cannot open file: ";
      case kNoSuchFile:
        return "No such file: ";
    }
    DLOG(FATAL) << "Invalid status code: " << static_cast<uint16_t>(code);
    return "";
  }

 private:
  struct Header {
    uint16_t size_;
    Code code_;
  };
  static constexpr auto kHeaderSize = sizeof(Header);
  // state_ == nullptr indicates OK
  // otherwise, the buffer layout is:
  //  state_[0..1] length of the error msg, i.e. size() - kHeaderSize
  //  state_[2..3] code
  //  state_[4...] verbose error message
  std::unique_ptr<const char[]> state_;
};

inline std::ostream &operator<<(std::ostream &os, const Status &status) {
  return os << status.toString();
}

}  // namespace bitcask

#endif  // BITCASK_STATUS_H_
