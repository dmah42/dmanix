#include <new>

#include "fs/fs.h"
#include "fs/node.h"
#include "memory/kalloc.h"
#include "memory/memory.h"
#include "multiboot.h"
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
    return NULL;
  string::copy(curr_dir.name, nodes[index].name); 
  curr_dir.inode = nodes[index].inode;
  return &curr_dir;
}

fs::Node* FindDir(fs::Node* node, const char* name) {
  if (node == root && string::compare(name, DEV))
    return dev;
  for (uint32_t i = 0; i < num_nodes; ++i) {
    if (string::compare(name, nodes[i].name))
      return &nodes[i];
  }
  return NULL;
}

}  // namespace

fs::Node* Initialize(const multiboot::Module& initrd_module) {
  const uint32_t location = initrd_module.start_address;
  //const uint32_t end = initrd_module.end_address;
  //screen::Printf("initrd: 0x%x -> 0x%x = %d bytes\n",
  //               location, end, end - location);

  void* root_mem = kalloc(sizeof(fs::Node));
  root = new (root_mem) fs::Node("initrd", fs::FLAG_DIRECTORY);
  root->readdir = ReadDir;
  root->finddir = FindDir;

  void* dev_mem = kalloc(sizeof(fs::Node));
  dev = new (dev_mem) fs::Node(DEV, fs::FLAG_DIRECTORY);
  dev->readdir = ReadDir;
  dev->finddir = FindDir;

  // Add the files
  header = (Header*) location;
  file_headers = (FileHeader*)(location + sizeof(Header));

  num_nodes = header->num_files;
  //screen::Printf("  num_files: %u\n", num_nodes);

  void* nodes_mem = (fs::Node*) kalloc(sizeof(fs::Node) * num_nodes);
  nodes = new (nodes_mem) fs::Node[num_nodes]();
  for (uint32_t i = 0; i < num_nodes; ++i) {
    file_headers[i].offset += location + header->content_offset;

    string::copy(nodes[i].name, file_headers[i].name);
    //screen::Printf("    %s @ 0x%x\n", file_headers[i].name, file_headers[i].offset);
    nodes[i].length = file_headers[i].size;
    nodes[i].inode = i;
    nodes[i].flags = fs::FLAG_FILE;
    nodes[i].read = Read;
  }
  return root;
}

void Shutdown() {
  for (uint32_t i = 0; i < num_nodes; ++i)
    nodes[i].~Node();
  kfree(nodes);

  dev->~Node();
  kfree(dev);

  root->~Node();
  kfree(root);
}

}  // namespace initrd
