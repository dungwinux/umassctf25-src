#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstring>
// #include <type_traits>

// class Node {
//   Node *left;
//   Node *right;
// };
//
// class Tree {
//   Node *root;
// };

extern "C" int special(char *__restrict__ dst, const char *__restrict__ src,
                       unsigned length);

#define PACKED __attribute__((__packed__))

class PACKED ZdSpan {
public:
  uint64_t addr : 57;
  uint8_t length : 7;
  // Tried to limit copying. At least we tried
  ZdSpan(const ZdSpan &) = delete;
  ZdSpan &operator=(const ZdSpan &) = delete;
  ZdSpan(uint64_t _addr, uint8_t _length) noexcept
      : addr{_addr}, length{_length} {};
  ZdSpan(const char *_addr, uint8_t _length) noexcept
      : addr{std::bit_cast<uint64_t>(_addr)}, length{_length} {};
  template <size_t N> inline constexpr bool operator==(const char (&other)[N]) {
    return (length == N - 1) &&
           (0 == memcmp(std::bit_cast<char *>(addr), other, length));
  }
  struct Start {};
  struct End {};
};
[[maybe_unused]] constexpr const auto ZdSpanStart = ZdSpan::Start{};
[[maybe_unused]] constexpr const auto ZdSpanEnd = ZdSpan::End{};

class PACKED ZdStr {
public:
  static constexpr const size_t SMALL_CAPACITY = 14;

private:
  union {
    struct Normal {
      uint64_t addr : 57;
      uint8_t capacity : 7;
    } PACKED normal;
    char small[SMALL_CAPACITY];
  } PACKED s;
  uint16_t length;
  inline auto _alloc_memory(uint8_t amt) const { return new char[1 << amt]; }
  bool inline _is_small() const { return length < SMALL_CAPACITY; }

public:
  ZdStr() noexcept {
    length = 0;
    s.small[0] = '\0';
  }
  template <size_t N> ZdStr(const char (&cstr)[N]) noexcept {
    length = N - 1;
    if constexpr (N < 1 + SMALL_CAPACITY) {
      memcpy(s.small, cstr, N);
    } else {
      s.normal.capacity = std::bit_width(N - 1);
      auto storage = _alloc_memory(s.normal.capacity);
      s.normal.addr = std::bit_cast<uint64_t>(storage);
      memcpy(storage, cstr, N);
    }
  }
  ZdStr(const char *&cstrp) noexcept {
    // C-string is NULL-terminated, and the logic follows that fact
    auto k = strlen(cstrp);
    length = k;
    if (k < SMALL_CAPACITY) {
      memcpy(s.small, cstrp, k + 1);
    } else {
      s.normal.capacity = std::bit_width(k);
      auto storage = _alloc_memory(s.normal.capacity);
      s.normal.addr = std::bit_cast<uint64_t>(storage);
      memcpy(storage, cstrp, k + 1);
    }
  }
  ZdStr(const ZdStr &other) noexcept {
    if (other._is_small()) {
      memcpy(s.small, other.s.small, other.length + 1);
    } else {
      s.normal.capacity = other.s.normal.capacity;
      auto clone_memory = _alloc_memory(s.normal.capacity);
      s.normal.addr = std::bit_cast<uint64_t>(clone_memory);
      memcpy(clone_memory, std::bit_cast<char *>(other.s.normal.addr),
             other.length + 1);
    }
    length = other.length;
  }
  ~ZdStr() noexcept {
    if (!_is_small()) {
      delete[] std::bit_cast<char *>(s.normal.addr);
    }
  }

  constexpr auto get_length() const noexcept { return length; }
  constexpr auto get_capacity() const noexcept {
    return _is_small() ? SMALL_CAPACITY : (1UZ << s.normal.capacity);
  }
  constexpr auto get_cstr() noexcept {
    return _is_small() ? s.small : std::bit_cast<char *>(s.normal.addr);
  }
  constexpr auto get_ccstr() const noexcept {
    return _is_small() ? s.small : std::bit_cast<const char *>(s.normal.addr);
  }
  template <class... Args> constexpr void concat(const Args &...args) noexcept {
    uint16_t new_length = length + (get_strlen(args) + ...);
    if (new_length < get_capacity()) {
      auto write_ptr = get_cstr() + length;
      {
        [[maybe_unused]] int dummy[sizeof...(Args)] = {
            (memcpy(write_ptr, get_addr(args), get_strlen(args)),
             write_ptr += get_strlen(args), 0)...};
      }
      *write_ptr = '\0';
      length = new_length;
    } else {
      uint8_t new_capacity = std::bit_width(new_length);
      auto new_memory = _alloc_memory(new_capacity);
      auto write_ptr = new_memory;
      memcpy(write_ptr, get_ccstr(), length);
      write_ptr += length;
      {
        [[maybe_unused]] int dummy[sizeof...(Args)] = {
            (memcpy(write_ptr, get_addr(args), get_strlen(args)),
             write_ptr += get_strlen(args), 0)...};
      }
      *write_ptr = '\0';
      if (!_is_small()) {
        delete[] std::bit_cast<char *>(s.normal.addr);
      }
      length = new_length;
      s.normal.addr = std::bit_cast<uint64_t>(new_memory);
      s.normal.capacity = new_capacity;
    }
  }

  template <typename T>
  constexpr static inline auto get_strlen(const T &s) noexcept -> uint16_t {
    if constexpr (std::is_same_v<T, ZdStr>) {
      return s.get_length();
    } else if constexpr (std::is_same_v<
                             std::remove_cvref_t<
                                 std::remove_pointer_t<std::decay_t<T>>>,
                             char>) {
      return strlen(s);
    } else if constexpr (std::is_same_v<T, ZdSpan>) {
      return s.length;
    }
    // This is intentionally left here to catch bugs.
#ifdef _DEBUG
    __debugbreak();
#endif
    return -1;
  };
  template <typename T>
  constexpr static inline auto get_addr(const T &s) noexcept -> const char * {
    if constexpr (std::is_same_v<T, ZdStr>) {
      return s.get_ccstr();
    } else if constexpr (std::is_same_v<
                             std::remove_cvref_t<
                                 std::remove_pointer_t<std::decay_t<T>>>,
                             char>) {
      return s;
    } else if constexpr (std::is_same_v<T, ZdSpan>) {
      return std::bit_cast<const char *>(s.addr);
    }
    // Same as above, to intentionally catch bugs.
#ifdef _DEBUG
    __debugbreak();
#endif
    return nullptr;
  };
  inline constexpr char &operator[](uint16_t index) noexcept {
    return _is_small() ? s.small[index]
                       : std::bit_cast<char *>(s.normal.addr)[index];
  }
  inline constexpr operator const char *() const { return get_ccstr(); }

  template <size_t N> inline constexpr bool operator==(const char (&other)[N]) {
    return (length == N - 1) && (0 == memcmp(get_ccstr(), other, length));
  }
  // inline constexpr bool operator==(const char *&otherp) const {
  //   return (length == strlen(otherp)) &&
  //          0 == strncmp(get_ccstr(), otherp, length);
  // }

  inline constexpr ZdSpan operator[](const uint16_t index_start,
                                     const uint16_t index_end) noexcept {
    return ZdSpan{std::bit_cast<uint64_t>(get_cstr()) + index_start,
                  static_cast<uint8_t>(index_end - index_start)};
  }
  inline constexpr ZdSpan operator[](const uint16_t index_start,
                                     const ZdSpan::End) noexcept {
    return ZdSpan{std::bit_cast<uint64_t>(get_cstr()) + index_start,
                  static_cast<uint8_t>(length - index_start)};
  }
  inline constexpr ZdSpan operator[](const ZdSpan::Start,
                                     const uint16_t index_end) noexcept {
    return ZdSpan{std::bit_cast<uint64_t>(get_cstr()),
                  static_cast<uint8_t>(index_end)};
  }
  // void multiply(uint32_t times) {
  //   // Force 16bit
  //   times &= 0xffff;
  //   auto next_length = times * (uint32_t)length;
  //   if (next_length < (uint32_t)length) {
  //     // Do nothing if overflow
  //     return;
  //   }
  // }
};

int main() {
  // printf("%llu", sizeof(ZdStr));
  // {
  //   ZdStr a1{"small"};
  // }
  // {
  //   ZdStr a2{"this is a normal string"};
  // }
  // {
  //   ZdStr a{"1"};
  //   ZdStr b{"2"};
  //   a.concat(b);
  // }
  // {
  //   ZdStr a{"1"};
  //   ZdStr b{"this is a normal string"};
  //   const char *w = "Hello, World!";
  //   a.concat(b, "\n", b, "\n", b, "\n", w, "\n");
  //   puts(a.get_cstr());
  // }
  // {
  //   ZdStr a{"Hello, World!"};
  //   ZdStr b{"look what we have here!"};
  //   a.concat({"\nAh, ", b});
  //   std::cout << a.get_length() << '\t' << a.get_cstr() << '\n'
  //             << b.get_length() << '\t' << b.get_cstr() << '\n';
  // }
  // {
  //   const char b[] = "Hello, World!";
  //   auto s0 = ZdStr{b};
  //   auto a = "Hello there. \nPlease enter the following code:";
  //   auto s1 = ZdStr{a};
  //   char c[] = {'A', 'B', 'C', '\0'};
  //   auto s2 = ZdStr{c};
  //   auto s3 = ZdStr{""};
  //   puts(s0.get_cstr());
  //   puts(s1.get_cstr());
  //   puts(s2.get_cstr());
  //   printf("[%d], %s", s3.get_length(), s3.get_cstr());
  // }

  auto welcome = ZdStr{"Welcome to The Void."};
  welcome.concat(
      "\n", "To activate our wonderful challenge, please input your flag:");
  puts(welcome);
  char write_buffer[0x100];
  const char *read_ptr = write_buffer;
  auto fmt = ZdStr{"UMASS{%255[^}"};
  fmt.concat("]s", fmt[12, ZdSpanEnd]);
  // puts(fmt);
  const size_t recv = scanf(fmt, write_buffer);
  const auto error_message =
      ZdStr{"Invalid input! Make sure you got the right flag :)"};
  if (1 != recv) {
    puts(error_message);
    return 1;
  }
  ZdStr input{read_ptr};
  int error_code = special(write_buffer, input.get_ccstr(), input.get_length());
  // puts(input);
  if (0 > error_code) {
    puts(ZdStr{"Unsupported CPU! Please upgrade your machine!"});
    return 1;
  }
  // write_buffer[input.get_length()] = '\0';
  // for (auto i = 0U; i < input.get_length(); ++i) {
  //   printf("\\x%02x", write_buffer[i]);
  // }
  putchar('\n');
  if (!(ZdSpan{write_buffer, static_cast<uint8_t>(input.get_length())} ==
        "\x00\x21\x22\x04\x77\x52\x30\x11\x70\x29\x47\x01\x66\x71\x61\x34\x67"
        "\x13\x65\x33\x61\x55\x24\x23\x24\x23\x57\x75\x78\x13\x69\x81\x24\x50"
        "\x69\x73\x97\x72\x87\x30\x75\x40\x34\x68\x57\x86\x97\x29\x88\x54\x64"
        "\x57\x00\x38\x91")) {
    puts(error_message);
    return 1;
  }
  puts(ZdStr{"You got it! Please submit the result online to get points."});
  return 0;
}
