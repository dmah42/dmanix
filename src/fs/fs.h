#ifndef FS_FS_H
#define FS_FS_H

#include <stdint.h>

namespace fs {

struct DirEntry;
class Node;

enum Flags {
  FLAG_FILE = 0x1,
  FLAG_DIRECTORY = 0x2,
  FLAG_CHAR_DEVICE = 0x3,
  FLAG_BLOCK_DEVICE = 0x4,
  FLAG_PIPE = 0x5,
  FLAG_SYMLINK = 0x6,
  FLAG_MOUNTPOINT = 0x8,
};

struct DirEntry {
  char name[128];
  uint32_t inode;
};

Node* Traverse(const char* filename);

}  // namespace fs

#endif  // FS_H
