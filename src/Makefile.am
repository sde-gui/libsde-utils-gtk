
ABI_VERSION=1:0:0

NULL=

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include \
	-DPACKAGE_DATA_DIR=\""$(datadir)/$(PACKAGE)"\" \
	-DPACKAGE_LIB_DIR=\""$(libdir)/$(PACKAGE)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_INSTALLATION_PREFIX=\""$(prefix)"\" \
	$(NULL)

lib_LTLIBRARIES = libsde-utils-gtk-0.la
libsde_utils_gtk_0_la_SOURCES = config.c gdk-pixbuf.c gdkrgba.c icon-grid.c

libsde_utils_gtk_0_la_CFLAGS = \
	$(PACKAGE_CFLAGS) \
	$(NULL)

libsde_utils_gtk_0_la_LIBADD = \
	$(PACKAGE_LIBS) \
	$(NULL)

libsde_utils_gtk_0_la_LDFLAGS = \
	-no-undefined \
	-export-symbols-regex ^su \
	-version-info $(ABI_VERSION) \
	$(NULL)

EXTRA_DIST = config.h