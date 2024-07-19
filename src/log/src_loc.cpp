#include "log/src_loc.h"

SrcLoc::SrcLoc()
  : file("unknown"),
    line(0),
    func("unknown")
{}

SrcLoc::SrcLoc(
  const char* file_,
  int line_,
  const char* func_
)
  : file(file_),
    line(line_),
    func(func_)
{}
