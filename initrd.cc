#include "base.h"
#include "fs.h"
#include "kalloc.h"
#include "memory.h"
#include "new.h"
#include "screen.h"
#include "string.h"
#include "tools/initrd_build.h"

namespace initrd {
namespace {

Header *header;
FileHeader *file_headers;

fs::Node* root;
fs::Node* dev;

// TODO: array type
fs::Node* nodes;

fs::DirEntry curr_dir;

uint32_t Read(fs::Node *node, uint32_t offset, uint32_t size, uint8_t* buffer) {
  const FileHeader& header = file_headers[node->inode];
  if (offset > header.size_)
    return 0;
  if (offset + size > header.size_)
    size = header.size_ - offset;
  memory::copy(buffer, (uint8_t*) (header.offset_ + offset), size);
  return size;
}

fs::DirEntry* ReadDir(fs::Node* node, uint32_t index) {
  if (node == root && index == 0) {
    memory::copy(curr_dir.name, "dev", 3);
    curr_dir.name[3] = '\0';
    curr_dir.inode = 0;
    return &curr_dir;
  }

  index -= 1;
  if (index >= header->num_files)
    return 0;
  // TODO: strncpy
  uint32_t name_len = string::length(nodes[index].name);
  if (name_len > sizeof(curr_dir.name) - 1)
    PANIC("name too long");
  memory::copy(curr_dir.name, nodes[index].name, name_len); 
  curr_dir.name[name_len] = '\0';
  curr_dir.inode = nodes[index].inode;
  return &curr_dir;
}

fs::Node* FindDir(fs::Node* node, const char* name) {
  if (node == root && string::compare(name, "dev"))
    return dev;
  for (uint32_t i = 0; i < header->num_files; ++i) {
    if (string::compare(name, nodes[i].name))
      return &nodes[i];
  }
  return 0;
}

}  // namespace

fs::Node* Initialize(uint32_t location) {
  screen::puts("Initializing initrd @ ");
  screen::puth(location);
  screen::putc('\n');
  // TODO: Node ctor should take name/type
  root = new (kalloc(sizeof(fs::Node))) fs::Node();
  memory::copy(root->name, "initrd", 6);
  root->flags = fs::FLAG_DIRECTORY;
  root->readdir = ReadDir;
  root->finddir = FindDir;

  dev = new (kalloc(sizeof(fs::Node))) fs::Node();
  memory::copy(root->name, "dev", 3);
  dev->flags = fs::FLAG_DIRECTORY;
  dev->readdir = ReadDir;
  dev->finddir = FindDir;

  // Add the files
  header = (Header*) location;
  file_headers = (FileHeader*)(location + sizeof(Header));

  screen::puts("Dumping location...\n");
  for (int i = 0; i < 32; ++i) {
    screen::puth(*(((uint8_t*)location) + i));
    screen::putc(' ');
  }
  screen::putc('\n');

  uint32_t num_nodes = header->num_files;
  screen::putd(num_nodes);
  screen::puts(" files found\n");
  nodes = new (kalloc(sizeof(fs::Node) * num_nodes)) fs::Node[num_nodes]();
  for (uint32_t i = 0; i < num_nodes; ++i) {
    file_headers[i].offset_ += location + header->content_offset;

    // TODO: string::copy
    uint32_t name_len = string::length(file_headers[i].name_);
    if (name_len > sizeof(nodes[i].name) - 1)
      PANIC("name too long");
    memory::copy(nodes[i].name, file_headers[i].name_, name_len);
    screen::puts("  ");
    screen::puts(file_headers[i].name_);
    screen::puts(" @ ");
    screen::puth(file_headers[i].offset_);
    screen::putc('\n');
    nodes[i].length = file_headers[i].size_;
    nodes[i].inode = i;
    nodes[i].flags = fs::FLAG_FILE;
    nodes[i].read = Read;
  }
  screen::puts("done\n");
  return root;
}

}  // namespace initrd
