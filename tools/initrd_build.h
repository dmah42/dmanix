#ifndef INITRD_BUILD_H
#define INITRD_BUILD_H

namespace initrd {

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
      : size(size),
        offset(offset) {
    if (name.length() > sizeof(this->name))
      std::cout << "WARNING: " << name << " is being truncated.\n";
    strncpy(this->name, name.c_str(), sizeof(this->name));
  }
#endif  // TOOL_BUILD

  uint32_t size;
  uint32_t offset;
  char name[64];
};

}  // namespace initrd

#endif  // INITRD_BUILD_H
