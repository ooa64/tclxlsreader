#include "xlsreadercmd.hpp"

extern "C" {
  DLLEXPORT int Xlsreader_Init (Tcl_Interp *interp);
}

static void Xlsreader_Done(void *);

XlsreaderCmd * cmd = NULL;

int Xlsreader_Init (Tcl_Interp *interp) {
  if (!cmd) {
#ifdef USE_TCL_STUBS    
    if (Tcl_InitStubs(interp, STUB_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }
#endif
    cmd = new XlsreaderCmd(interp, "xlsreader");
    Tcl_CreateExitHandler(Xlsreader_Done, NULL);
    Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION);
  }

  return TCL_OK;
}

static void Xlsreader_Done(void *) {
  if (cmd) {
    delete cmd;
    cmd = NULL;
  }
}
