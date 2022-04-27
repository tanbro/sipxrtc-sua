#include "version.hh"

#include <sstream>
#include <string>

#include <samplerate.h>

#include <pjsua2.hpp>

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM "windows" // Windows
#elif defined(__CYGWIN__) && !defined(_WIN32)
#define PLATFORM "windows" // Windows (Cygwin POSIX under Microsoft Window)
#elif defined(__ANDROID__)
#define PLATFORM "android" // Android (implies Linux, so it must come first)
#elif defined(__linux__)
#define PLATFORM "linux"
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
#include <sys/param.h>
#if defined(BSD)
#define PLATFORM "bsd" // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
#endif
#elif defined(__hpux)
#define PLATFORM "hp-ux" // HP-UX
#elif defined(_AIX)
#define PLATFORM "aix"                        // IBM AIX
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
#define PLATFORM "ios" // Apple iOS
#elif TARGET_OS_IPHONE == 1
#define PLATFORM "ios" // Apple iOS
#elif TARGET_OS_MAC == 1
#define PLATFORM "osx" // Apple OSX
#endif
#elif defined(__sun) && defined(__SVR4)
#define PLATFORM "solaris" // Oracle Solaris, Open Indiana
#else
#define PLATFORM "unknown"
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define ARCHITECTURE "x86_64"
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define ARCHITECTURE "x86_32"
#elif defined(__ARM_ARCH_2__)
#define ARCHITECTURE "ARM2"
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
#define ARCHITECTURE "ARM3"
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
#define ARCHITECTURE "ARM4T"
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
#define ARCHITECTURE "ARM5"
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
#define ARCHITECTURE "ARM6T2"
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) ||                   \
    defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) ||                    \
    defined(__ARM_ARCH_6ZK__)
#define ARCHITECTURE "ARM6"
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) ||                   \
    defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) ||                    \
    defined(__ARM_ARCH_7S__)
#define ARCHITECTURE "ARM7"
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) ||                  \
    defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#define ARCHITECTURE "ARM7A"
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) ||                  \
    defined(__ARM_ARCH_7S__)
#define ARCHITECTURE "ARM7R"
#elif defined(__ARM_ARCH_7M__)
#define ARCHITECTURE "ARM7M"
#elif defined(__ARM_ARCH_7S__)
#define ARCHITECTURE "ARM7S"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCHITECTURE "ARM64"
#elif defined(__arm__) || defined(_M_ARM)
#define ARCHITECTURE "ARM"
#elif defined(mips) || defined(__mips__) || defined(__mips)
#define ARCHITECTURE "MIPS"
#elif defined(__sh__)
#define ARCHITECTURE "SUPERH"
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) ||  \
    defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) ||            \
    defined(_ARCH_PPC)
#define ARCHITECTURE "POWERPC"
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
#define ARCHITECTURE "POWERPC64"
#elif defined(__sparc__) || defined(__sparc)
#define ARCHITECTURE "SPARC"
#elif defined(__m68k__)
#define ARCHITECTURE "M68K"
#else
#define ARCHITECTURE "UNKNOWN"
#endif

using namespace std;

namespace sipxsua {

string versionString;

const string &getVersionString() {
  if (!versionString.empty()) {
    return versionString;
  }

  ostringstream oss;

  oss << ": "
#ifdef __GIT_DESC__
      << "git " << __GIT_DESC__
#else
#endif
      << " "
#ifdef __GIT_REV__
      << "rev " << __GIT_REV__ << ""
#endif
      << " " << PLATFORM << "/" << ARCHITECTURE << endl

      << endl

      << "  compiler:" << endl
#ifdef __GNUC__
      << "    gcc " << __VERSION__
#else
      << "    unknown"
#endif
      << endl

      << endl

      << "  with:" << endl
      << "    pjsip " << PJ_VERSION << endl
      << "    " << src_get_version() << endl

      << endl

      << "  build: " << __TIME__ << ", " << __DATE__ << endl;

  versionString = oss.str();
  return versionString;
}

}; // namespace sipxsua
