#ifndef XLSREADERCMD_H
#define XLSREADERCMD_H

#include <tcl.h>
#include <xls.h>
#include "tclcmd.hpp"

class XlsreaderCmd : public TclCmd {

public:

  XlsreaderCmd (Tcl_Interp * interp, const char * name): TclCmd(interp, name) {}

private:

  virtual int Command (int objc, Tcl_Obj * const objv[]);
};

#endif