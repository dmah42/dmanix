#include "fs/node.h"

#include <stdlib.h>

#include "fs/fs.h"
#include "string.h"

namespace fs {

Node::Node()
    : perm(0), uid(0), gid(0), flags(0), inode(0), length(0), impl(0),
      read(NULL), write(NULL), open(NULL), close(NULL),
      readdir(NULL), finddir(NULL),
      link(NULL) {
  name[0] = '\0';
}

Node::Node(const char* node_name, uint32_t flags)
    : perm(0), uid(0), gid(0), flags(flags), inode(0), length(0), impl(0),
      read(NULL), write(NULL), open(NULL), close(NULL),
      readdir(NULL), finddir(NULL),
      link(NULL) {
  string::copy(name, node_name);
}

uint32_t Node::Read(uint32_t offset, uint32_t size, uint8_t* buffer) {
  return read ? read(this, offset, size, buffer) : 0;
}

uint32_t Node::Write(uint32_t offset, uint32_t size, uint8_t* buffer) {
  return write ? write(this, offset, size, buffer) : 0;
}

void Node::Open() {
  if (open) open(this);
}

void Node::Close() {
  if (close) close(this);
}

DirEntry* Node::ReadDir(uint32_t index) {
  if ((flags & FLAG_DIRECTORY) == FLAG_DIRECTORY && readdir)
    return readdir(this, index);
  return NULL;
}

Node* Node::FindDir(const char* name) {
  if ((flags & FLAG_DIRECTORY) == FLAG_DIRECTORY && finddir)
    return finddir(this, name);
  return NULL;
}

}  // namespace fs
