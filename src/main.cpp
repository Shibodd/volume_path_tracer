#include <nanovdb/io/IO.h>

int main() {
  nanovdb::GridHandle<nanovdb::HostBuffer> handle;
  handle = nanovdb::io::readGrid<decltype(handle)::BufferType>("scenes/fire.nvdb");
}
