#!/bin/sh
# Example for use of GNU gettext.
# This file is in the public domain.
#
# Script for cleaning all autogenerated files.

test ! -f Makefile || make distclean

# Brought in by explicit copy.
rm -f gettext.h

# Brought in by autopoint.
rm -f ABOUT-NLS
rm -f config.rpath
rm -f m4/codeset.m4
rm -f m4/gettext.m4
rm -f m4/glibc2.m4
rm -f m4/glibc21.m4
rm -f m4/iconv.m4
rm -f m4/intdiv0.m4
rm -f m4/intl.m4
rm -f m4/intldir.m4
rm -f m4/intlmacosx.m4
rm -f m4/intmax.m4
rm -f m4/inttypes-pri.m4
rm -f m4/inttypes_h.m4
rm -f m4/lcmessage.m4
rm -f m4/lib-ld.m4
rm -f m4/lib-link.m4
rm -f m4/lib-prefix.m4
rm -f m4/lock.m4
rm -f m4/longlong.m4
rm -f m4/nls.m4
rm -f m4/po.m4
rm -f m4/printf-posix.m4
rm -f m4/progtest.m4
rm -f m4/size_max.m4
rm -f m4/stdint_h.m4
rm -f m4/uintmax_t.m4
rm -f m4/visibility.m4
rm -f m4/wchar_t.m4
rm -f m4/wint_t.m4
rm -f m4/xsize.m4
rm -f po/Makefile.in.in
rm -f po/remove-potcdate.sin

# Generated by aclocal.
rm -f aclocal.m4

# Generated by autoconf.
rm -f configure

# Generated or brought in by automake.
rm -f Makefile.in
rm -f m4/Makefile.in
rm -f install-sh
rm -f missing
rm -f config.guess
rm -f config.sub
rm -f po/*.pot
rm -f po/stamp-po
rm -f po/*.gmo
