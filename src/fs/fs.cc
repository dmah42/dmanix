#include "fs/fs.h"

#include "fs/node.h"
#include "string.h"

#include <stdlib.h>

namespace fs {

Node* root = NULL;

Node* Traverse(const char* filename) {
  fs::DirEntry* node = NULL;
  uint32_t i = 0;
  while ((node = fs::root->ReadDir(i)) != NULL) {
    fs::Node* fsnode = fs::root->FindDir(node->name);
    if ((fsnode->flags & fs::FLAG_DIRECTORY) == fs::FLAG_DIRECTORY) {
      // TODO(dominic): recurse.
    } else if (string::compare(node->name, filename)) {
      return fsnode;
    }
    ++i;
  }
  return NULL;
}

}  // namespace fs
