#include "version.hh"

#include <sstream>
#include <string>

#include <samplerate.h>

#include <pjsua2.hpp>

namespace sipxsua {

std::string version;

const std::string &getVersionString() {
  if (!version.empty()) {
    return version;
  }
  std::ostringstream oss;
  /* clang-format off */
  oss << "git-"
#ifdef __PROJECT_VERSION__
      << __PROJECT_VERSION__
#else
      << "unknown "
#endif

#if defined(linux) || defined(__linux__) || defined(__linux)
      << "linux"
#elif defined(__APPLE__) || defined(__MACH__)
      << "macos"
#elif defined(__FreeBSD__)
      << "freebsd"
#elif defined(__ANDROID__)
      << "android"
#elif defined(unix) || defined(__unix__) || defined(__unix)
      << "unix"
#elif defined(_WIN32) || defined(_WIN64)
      << "windows"
#else
      << "unknown"
#endif
      << "/"
#if defined(__x86_64__) || defined(__x86_64)
      << "x86_64"
#elif defined(__amd64__) || defined(__amd64)
      << "amd64"
#elif defined(i386) || defined(__i386) || defined(__i386__)
      << "i386"
#elif defined(__aarch64__)
      << "aarch64"
#elif defined(__arm__)
      << "arm"
#else
      << "unknown"
#endif
      << "\n\n"
      << "  compiler:"
#ifdef __GNUC__
      << "\n"
      << "    gcc " << __VERSION__
#else
      << " unknown"
#endif
      << "\n\n"
      << "  with:"
      << "\n"
      << "    pjsip " << PJ_VERSION
      << "\n"
      << "    " << src_get_version()
      << "\n\n"
      << "  build: " << __TIME__ << " " << __DATE__
      << "\n"
      ;
  /* clang-format on */
  version = oss.str();
  return version;
}

}; // namespace sipxsua
