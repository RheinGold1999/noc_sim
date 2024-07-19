#ifndef __SRC_LOC_H__
#define __SRC_LOC_H__

struct SrcLoc
{
  const char* file;
  int         line;
  const char* func;

  SrcLoc();
  SrcLoc(const char* file_, int line_, const char* func_);
};

#endif  // __SRC_LOC_H__
