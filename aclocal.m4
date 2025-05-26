#
# Include the TEA standard macro set
#

builtin(include,tclconfig/tcl.m4)

#
# Add here whatever m4 macros you want to define for your package
#

AC_DEFUN([XLSREADER_SETUP_DEBUGLOG], [
    AC_MSG_CHECKING([for build with debuglog])
    AC_ARG_ENABLE(debuglog,
	AS_HELP_STRING([--enable-debuglog],
	    [build with debug printing (default: off)]),
	[tcl_ok=$enableval], [tcl_ok=no])
    if test "$tcl_ok" = "yes"; then
	CFLAGS_DEBUGLOG="-DTCLCMD_DEBUG -DXLSREADER_DEBUG -DXLSREADERCMD_DEBUG"
	AC_MSG_RESULT([yes])
    else
	CFLAGS_DEBUGLOG=""
	AC_MSG_RESULT([no])
    fi
    AC_SUBST(CFLAGS_DEBUGLOG)
])
