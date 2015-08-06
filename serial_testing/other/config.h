/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

#ifndef SP_CONFIG_H
#define SP_CONFIG_H

/* BOTHER macro is available */
#define HAVE_BOTHER 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* struct serial is available. */
#define HAVE_SERIAL_STRUCT 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if `c_ispeed' is a member of `struct termios2'. */
#define HAVE_STRUCT_TERMIOS2_C_ISPEED 1

/* Define to 1 if `c_ospeed' is a member of `struct termios2'. */
#define HAVE_STRUCT_TERMIOS2_C_OSPEED 1

/* Define to 1 if `c_ispeed' is a member of `struct termios'. */
/* #undef HAVE_STRUCT_TERMIOS_C_ISPEED */

/* Define to 1 if `c_ospeed' is a member of `struct termios'. */
/* #undef HAVE_STRUCT_TERMIOS_C_OSPEED */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* struct termios2 is available. */
#define HAVE_TERMIOS2 1

/* struct termios2 has c_ispeed/c_ospeed. */
#define HAVE_TERMIOS2_SPEED 1

/* struct termios has c_ispeed/c_ospeed. */
/* #undef HAVE_TERMIOS_SPEED */

/* struct termiox is available. */
#define HAVE_TERMIOX 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Enumeration is unsupported */
/* #undef NO_ENUMERATION */

/* Port metadata is unavailable */
/* #undef NO_PORT_METADATA */

/* Name of package */
#define PACKAGE "libserialport"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "martin-libserialport@earth.li"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libserialport"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libserialport 0.1.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libserialport"

/* Define to the home page for this package. */
#define PACKAGE_URL "http://sigrok.org/wiki/Libserialport"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.1.1"

/* Macro preceding public API functions */
#define SP_API __attribute__((visibility("default")))

/* Macro preceding private functions */
#define SP_PRIV __attribute__((visibility("hidden")))

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.1.1"

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

#endif
