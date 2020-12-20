#include <cstring>
#include <cstdarg>
#include <cstdio>
#include "../../../bitvisor/bitvisor.hpp"

extern "C" {

void __init_serial1() {}

void __serial_print1(const char* cstr)
{
  bv_console_write(cstr, strlen(cstr));
}
void __serial_print(const char* str, size_t len)
{
  bv_console_write(str, len);
}

void kprint(const char* c){
  __serial_print1(c);
}

void kprintf(const char* format, ...)
{
  char buf[8192];
  va_list aptr;
  va_start(aptr, format);
  vsnprintf(buf, sizeof(buf), format, aptr);
  __serial_print1(buf);
  va_end(aptr);
}

}
