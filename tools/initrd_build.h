#ifndef INITRD_BUILD_H
#define INITRD_BUILD_H

struct Header {
#ifdef TOOL_BUILD
  Header(uint32_t num_files, uint32_t content_offset)
      : num_files(num_files), content_offset(content_offset) { }
#endif
  uint32_t num_files;
  uint32_t content_offset;
};

struct FileHeader {
#ifdef TOOL_BUILD
  FileHeader(const std::string& name, uint32_t size, uint32_t offset)
      : size_(size),
        offset_(offset) {
    if (name.length() > sizeof(name_))
      std::cout << "WARNING: " << name << " is being truncated.\n";
    strncpy(name_, name.c_str(), sizeof(name_));
  }
#endif  // TOOL_BUILD

  uint32_t size_;
  uint32_t offset_;
  char name_[64];
};

#endif  // INITRD_BUILD_H
