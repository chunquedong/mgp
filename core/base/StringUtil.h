/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */
#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_


#include <string>
#include <vector>

namespace mgp
{

class StringUtil {
public:
    typedef std::string String;

    static size_t hashCode(const String& self);
    static bool iequals(const String& self, const String &other);
    static bool contains(const String& self, const String& s);
    static bool startsWith(const String& self, const String& s);
    static bool endsWith(const String& self, const String& s);

    static void replace(String& self, const String& src, const String& dst);
    static std::vector<String> split(const String& self, const String &sep);
    static String substr(const String& self, size_t pos, size_t len = -1);

    static void trimEnd(String& self);
    static void trimStart(String& self);
    static void trim(String& self) { trimStart(self); trimEnd(self); }
    static void removeLastChar(String& self);

    static String toLower(const String& self);
    static String toUpper(const String& self);

    static int toInt(const String& self);
    static int64_t toLong(const String& self);
    static float toFloat(const String& self);
    static double toDouble(const String& self);

    static String fromInt(int i);
    static String fromLong(int64_t i);
    static String fromDouble(double f);
    static String fromFloat(float f);

    /**
    * 'printf' style format
    */
    static String format(const char* fmt, ...);
};

}

#endif