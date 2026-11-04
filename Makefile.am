SUBDIRS		= doc po src examples conf
EXTRA_DIST	= ABOUT-NLS AUTHORS AX25-NOTES COPYING ChangeLog \
		  FAQ INSTALL NEWS README TODO \
		  tnt.spec BUILD.md tests/modern-c.sh tests/test-inp.sh tests/io.sh \
		  tests/terminal.sh packaging/README.packages \
		  packaging/build-deb.sh packaging/build-rpm.sh \
		  packaging/test-package.py

TESTS = tests/modern-c.sh tests/test-inp.sh tests/io.sh tests/terminal.sh
LOG_COMPILER = $(SHELL)
TESTS_ENVIRONMENT = top_srcdir='$(abs_top_srcdir)' \
		   top_builddir='$(abs_top_builddir)' CC='$(CC)'
