#ifndef FS_H
#define FS_H

#include <stdint.h>

namespace fs {

struct DirEntry;
struct Node;

typedef uint32_t (*read_func)(Node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_func)(Node*, uint32_t, uint32_t, uint8_t*);
typedef void (*open_func)(Node*);
typedef void (*close_func)(Node*);
typedef DirEntry* (*readdir_func)(Node*, uint32_t);
typedef Node* (*finddir_func)(Node*, const char* name);

enum Flags {
  FLAG_FILE = 0x1,
  FLAG_DIRECTORY = 0x2,
  FLAG_CHAR_DEVICE = 0x3,
  FLAG_BLOCK_DEVICE = 0x4,
  FLAG_PIPE = 0x5,
  FLAG_SYMLINK = 0x6,
  FLAG_MOUNTPOINT = 0x8,
};

class Node {
 public:
  Node();
  explicit Node(const char* node_name, uint32_t flags);

  char name[128];
  uint32_t perm;
  uint32_t uid;
  uint32_t gid;
  uint32_t flags;
  uint32_t inode;
  uint32_t length;
  uint32_t impl;

  read_func read;
  write_func write;
  open_func open;
  close_func close;
  readdir_func readdir;
  finddir_func finddir;

  // For mountpoints and symlinks
  Node* link;

  uint32_t Read(uint32_t offset, uint32_t size, uint8_t* buffer);
  uint32_t Write(uint32_t offset, uint32_t size, uint8_t* buffer);
  void Open();
  void Close();
  DirEntry* ReadDir(uint32_t index);
  Node* FindDir(const char* name);

 private:
  // these methods intentionally left unimplemented.
  Node(const Node&);
};

struct DirEntry {
  char name[128];
  uint32_t inode;
};

}  // namespace fs

#endif  // FS_H
