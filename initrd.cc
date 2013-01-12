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

const char* DEV = "dev";

Header *header;
FileHeader *file_headers;

fs::Node* root;
fs::Node* dev;

// TODO: dynamic array type
fs::Node* nodes;
uint32_t num_nodes;

fs::DirEntry curr_dir;

uint32_t Read(fs::Node *node, uint32_t offset, uint32_t size, uint8_t* buffer) {
  const FileHeader& header = file_headers[node->inode];
  if (offset > header.size)
    return 0;
  if (offset + size > header.size)
    size = header.size - offset;
  memory::copy(buffer, (uint8_t*) (header.offset + offset), size);
  return size;
}

fs::DirEntry* ReadDir(fs::Node* node, uint32_t index) {
  if (node == root && index == 0) {
    string::copy(curr_dir.name, DEV);
    curr_dir.inode = 0;
    return &curr_dir;
  }

  index -= 1;
  if (index >= num_nodes)
    return 0;
  string::copy(curr_dir.name, nodes[index].name); 
  curr_dir.inode = nodes[index].inode;
  return &curr_dir;
}

fs::Node* FindDir(fs::Node* node, const char* name) {
  if (node == root && string::compare(name, "dev"))
    return dev;
  for (uint32_t i = 0; i < num_nodes; ++i) {
    if (string::compare(name, nodes[i].name))
      return &nodes[i];
  }
  return 0;
}

}  // namespace

fs::Node* Initialize(uint32_t location) {
  screen::Printf("  initrd @ %x\n", location);
  // TODO: Node ctor should take name/type
  //root = new (kalloc(sizeof(fs::Node))) fs::Node();
  root = (fs::Node*) kalloc(sizeof(fs::Node));
  string::copy(root->name, "initrd");
  root->perm = root->uid = root->gid = root->inode = root->length = 0;
  root->flags = fs::FLAG_DIRECTORY;
  root->read = 0; root->write = 0; root->open = 0; root->close = 0;
  root->readdir = ReadDir;
  root->finddir = FindDir;
  root->link = 0;
  root->impl = 0;

  //dev = new (kalloc(sizeof(fs::Node))) fs::Node();
  dev = (fs::Node*) kalloc(sizeof(fs::Node));
  string::copy(dev->name, DEV);
  dev->perm = dev->uid = dev->gid = dev->inode = dev->length = 0;
  dev->flags = fs::FLAG_DIRECTORY;
  dev->read = 0; dev->write = 0; dev->open = 0; dev->close = 0;
  dev->readdir = ReadDir;
  dev->finddir = FindDir;
  dev->link = 0;
  dev->impl = 0;

  // Add the files
  header = (Header*) location;
  file_headers = (FileHeader*)(location + sizeof(Header));

  num_nodes = header->num_files;
  screen::Printf("  num_files: %u\n", num_nodes);
  //nodes = new (kalloc(sizeof(fs::Node) * num_nodes)) fs::Node[num_nodes]();
  nodes = (fs::Node*) kalloc(sizeof(fs::Node) * num_nodes);
  for (uint32_t i = 0; i < num_nodes; ++i) {
    file_headers[i].offset += location + header->content_offset;

    string::copy(nodes[i].name, file_headers[i].name);
    screen::Printf("    %s @ %x\n", file_headers[i].name, file_headers[i].offset);
    nodes[i].length = file_headers[i].size;
    nodes[i].inode = i;
    nodes[i].flags = fs::FLAG_FILE;
    nodes[i].read = Read;
  }
  return root;
}

}  // namespace initrd
