#include <stdint.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define TOOL_BUILD
#include "initrd_build.h"

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <file-list>\n";
    return 1;
  }

  std::ifstream filelist(argv[1], std::ifstream::in);

  uint32_t num_files = 0;
  std::stringstream header_stream;
  std::stringstream content_stream;
  while (filelist.good()) {
    char line_buffer[1024];
    filelist.getline(line_buffer, sizeof(line_buffer));
    const std::string line(line_buffer);
    if (line.length() == 0)
      continue;
    std::cout << "Parsing " << line << "\n";
    const size_t split_pos = line.find_first_of(' ');
    if (split_pos == std::string::npos) {
      std::cerr << "Poorly formatted line in file list: " << line << "\n";
      return 1;
    }
    const std::string input = line.substr(0, split_pos);
    const std::string output = line.substr(split_pos + 1, std::string::npos);

    std::ifstream infile(input.c_str(), std::ifstream::binary);
    infile.seekg(0, std::ifstream::end);
    size_t insize = infile.tellg();
    infile.seekg(0);

    FileHeader header(output, insize, content_stream.tellp());
    header_stream.write((const char*) &header, sizeof(header));

    char* inbuffer = new char[insize];
    infile.read(inbuffer, insize);
    content_stream.write(inbuffer, insize);

    std::cout << "[" << num_files << "] " << input << " -> " << header.name_ <<
                 " @ " << header.offset_ << "\n";

    ++num_files;
  }

  Header header(num_files, sizeof(Header) + header_stream.tellg());
  std::cout << "File count: " << header.num_files << "\n";
  std::cout << "Header size: " << header.content_offset << "\n";

  // Write out initrd
  std::ofstream initrd("initrd", std::ifstream::binary);
  initrd.write((const char*) &header, sizeof(Header));
  header_stream.seekg(0);
  initrd << header_stream.rdbuf();
  content_stream.seekg(0);
  initrd << content_stream.rdbuf();

  std::cout << "initrd size: " << initrd.tellp() << "\n";

  return 0;
}
