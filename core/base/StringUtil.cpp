/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */
#include <stdlib.h>
#include "StringUtil.h"
#include <functional>
#include <stdarg.h>
//#include <varargs.h>
#include <string.h>
#include <cstdlib>

using namespace mgp;
#define BUF_SIZE 1024
typedef std::string String;

size_t StringUtil::hashCode(const String& self) {
    std::hash<std::string> hash_fn;
    return hash_fn(self);
}

bool StringUtil::iequals(const String& self, const String &other) {
  size_t sz = self.size();
  if (other.size() != sz)
    return false;
  for (unsigned int i = 0; i < sz; ++i)
    if (tolower(self[i]) != tolower(other[i]))
      return false;
  return true;
}

bool StringUtil::contains(const String& self, const String& s) {
    std::string::size_type pos = self.find(s);
    return pos != std::string::npos;
}
bool StringUtil::startsWith(const String& self, const String& s) {
    return self.find(s) == 0;
}
bool StringUtil::endsWith(const String& self, const String& s) {
    return self.rfind(s) == (self.size() - s.size());
}

void StringUtil::replace(String& self, const String& src, const String& dst) {
  if (src.compare(dst) == 0) {
    return;
  }
  size_t srcLen = src.size();
  size_t desLen = dst.size();
  std::string::size_type pos = self.find(src);
  
  while ((pos != std::string::npos))
  {
    self.replace(pos, srcLen, dst);
    pos = self.find(src, (pos+desLen));
  }
}

std::vector<String> StringUtil::split(const String& self, const String &sep) {
  std::vector<String> tokens;
  if (self.size() == 0)
      return tokens;
  std::size_t start = 0, end = 0;
  while ((end = self.find(sep, start)) != std::string::npos) {
    tokens.push_back(self.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(self.substr(start));
  return tokens;
}

String StringUtil::substr(const String& self, size_t pos, size_t len) {
    return self.substr(pos, len);
}

static bool isSpace(char ch) {
    return (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t');
}

void StringUtil::trimEnd(String& str) {
  int i = str.size()-1;
  for (; i >=0; --i) {
    if (!isSpace(str[i])) {
      break;
    }
  }

  if (i < str.size()-1) {
    str.erase(str.begin()+i+1, str.end());
  }
}
void StringUtil::trimStart(String& str) {
  int i = 0;
  for (; i < str.size(); ++i) {
    if (!isSpace(str[i])) {
      break;
    }
  }

  if (i > 0) {
    str.erase(str.begin(), str.begin()+i);
  }
}

void StringUtil::removeLastChar(String& str) {
  if (str.length() == 0) return;
  str.erase(str.length() - 1);
}

String StringUtil::toLower(const String& str) {
  std::string ret;
  char chrTemp;
  size_t i;
  for (i = 0; i < str.length(); ++i)
  {
    chrTemp = str[i];
    chrTemp = tolower(chrTemp);
    ret.push_back(chrTemp);
  }
  
  return ret;
}
String StringUtil::toUpper(const String& str) {
  std::string ret;
  char chrTemp;
  size_t i;
  for (i = 0; i < str.length(); ++i)
  {
    chrTemp = str[i];
    chrTemp = toupper(chrTemp);
    ret.push_back(chrTemp);
  }
  
  return ret;
}

int64_t StringUtil::toLong(const String& str) {
  if (str.empty()) return 0;
  int64_t nValue=0;
  sscanf(str.c_str(),"%lld",&nValue);
  return nValue;
}
int StringUtil::toInt(const String& self) { return (int)std::stol(self.c_str(), NULL, 10); }
float StringUtil::toFloat(const String& self) { return std::stof(self.c_str(), NULL); }
double StringUtil::toDouble(const String& self) { return std::stod(self.c_str(), NULL); }

String StringUtil::fromInt(int i) {
  char buf[BUF_SIZE];
  snprintf(buf, sizeof(buf), "%d", i);
  
  return buf;
}
String StringUtil::fromLong(int64_t i) {
  char buf[BUF_SIZE];
  snprintf(buf, sizeof(buf), "%lld", i);
  
  return buf;
}
String StringUtil::fromDouble(double f) {
  char buf[BUF_SIZE];
  snprintf(buf, sizeof(buf), "%f", f);
  
  return buf;
}
String StringUtil::fromFloat(float f) {
  char buf[BUF_SIZE];
  snprintf(buf, sizeof(buf), "%f", f);
  
  return buf;
}

/**
* 'printf' style format
*/
String StringUtil::format(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  
  char buf[BUF_SIZE];
  char *abuf = NULL;
  int i = vsnprintf(buf, sizeof(buf), fmt, args);
  
  if (i < 0) {
    va_end(args);
    return "";
  }
  if (i >= BUF_SIZE) {
    abuf = (char*)malloc(i+1);
    i = vsnprintf(abuf, i, fmt, args);
    if (i < 0) {
      va_end(args);
      return "";
    }
    if (i>0) {
      String str(abuf);
      free(abuf);
      va_end(args);
      return str;
    }
  }

  va_end(args);
  return String(buf);
}
