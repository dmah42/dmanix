#ifndef INITRD_H
#define INITRD_H

namespace fs { class Node; }
namespace multiboot { struct Module; }

namespace initrd {
  fs::Node* Initialize(const multiboot::Module& initrd_module);
}  // namespace initrd

#endif  // INITRD_H
