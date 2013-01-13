#include "fs.h"
#include "string.h"

namespace fs {

Node* root = 0;

Node::Node()
    : perm(0), uid(0), gid(0), flags(0), inode(0), length(0), impl(0), 
      read(0), write(0), open(0), close(0), readdir(0), finddir(0),
      link(0) {
  name[0] = '\0';
}

Node::Node(const char* node_name, uint32_t flags)
    : perm(0), uid(0), gid(0), flags(flags), inode(0), length(0), impl(0), 
      read(0), write(0), open(0), close(0), readdir(0), finddir(0),
      link(0) {
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
  return 0;
}

Node* Node::FindDir(const char* name) {
  if ((flags & FLAG_DIRECTORY) == FLAG_DIRECTORY && finddir)
    return finddir(this, name);
  return 0;
}

}  // namespace fs
