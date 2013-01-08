#ifndef INITRD_H
#define INITRD_H

// TODO: reimagine this

namespace fs {
struct Node;
}  // namespace fs

namespace initrd {
  fs::Node* Initialize(uint32_t location);
}  // namespace initrd

#endif  // INITRD_H
