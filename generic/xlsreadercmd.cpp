#include <string.h>
#include <xls.h>
#include "xlsreadercmd.hpp"

using namespace xls;

#if defined(XLSREADERCMD_DEBUG)
#   include <iostream>
#   define DEBUGLOG(_x_) (std::cerr << "DEBUG: " << _x_ << "\n")
#else
#   define DEBUGLOG(_x_)
#endif

/*
  xlsreader read $xlsfile
    opens xls file, returns a list of lists (sheets/rows).
*/

int XlsreaderCmd::Command (int objc, Tcl_Obj * const objv[]) {
  if (objc != 3) {
    Tcl_WrongNumArgs(tclInterp, 1, objv, "read <filename>");
    return TCL_ERROR;
  }
  int result = TCL_ERROR;
  Tcl_DString s;
  Tcl_DString e;
  Tcl_DStringInit(&s);
  Tcl_DStringInit(&e);
  char * filename = Tcl_UtfToExternalDString(NULL,
      Tcl_TranslateFileName(tclInterp, Tcl_GetString(objv[2]), &s), -1, &e);
  DEBUGLOG(objv[2] << " | " << filename);
#ifdef DEBUG
  xls::xls(1);
#endif

  if (strcmp(Tcl_GetString(objv[1]), "read") == 0) {

    xls_error_t error = LIBXLS_OK;
    xlsWorkBook * pWB = xls_open_file(filename, "UTF-8", &error);
    if (!pWB) {
      Tcl_AppendResult(tclInterp, "Error reading ", filename, ": ", xls_getError(error), NULL);
      goto exit;
    }
    Tcl_Obj * resultObj = Tcl_GetObjResult(tclInterp);

    for (unsigned int sheet = 0; sheet < pWB->sheets.count; sheet++) {

      Tcl_Obj * sheetObj = Tcl_NewObj();
      xlsWorkSheet * pWS = xls_getWorkSheet(pWB, sheet);
      xls_parseWorkSheet(pWS);

      for (unsigned int row = 0; row <= (unsigned int)pWS->rows.lastrow; row++) {

        Tcl_Obj * rowObj = Tcl_NewObj();

        for (unsigned int col = 0; col <= (unsigned int)pWS->rows.lastcol; col++) {

          xlsCell *cell = xls_cell(pWS, row, col);
          if ((!cell) || (cell->isHidden)) {
            continue;
          }
          if (cell->id == XLS_RECORD_RK || cell->id == XLS_RECORD_MULRK || cell->id == XLS_RECORD_NUMBER) {
            Tcl_ListObjAppendElement(tclInterp, rowObj, Tcl_NewDoubleObj(cell->d));
          } else if (cell->id == XLS_RECORD_FORMULA || cell->id == XLS_RECORD_FORMULA_ALT) {
            // formula
            if (cell->l == 0) // its a number
            {
              Tcl_ListObjAppendElement(tclInterp, rowObj, Tcl_NewDoubleObj(cell->d));
            } else if (cell->str) {
              if (!strcmp((char *)cell->str, "bool")) // its boolean, and test cell->d
              {
                Tcl_ListObjAppendElement(tclInterp, rowObj, Tcl_NewBooleanObj((int)cell->d));
              } else if (!strcmp((char *)cell->str, "error")) // formula is in error
              {
                Tcl_ListObjAppendElement(tclInterp, rowObj, Tcl_NewStringObj("*error*", -1));
              } else // ... cell->str is valid as the result of a string formula.
              {
                Tcl_ListObjAppendElement(tclInterp, rowObj, Tcl_NewStringObj((char *)cell->str, -1));
              }
            }
          } else if (cell->str) {
            Tcl_ListObjAppendElement(tclInterp, rowObj, Tcl_NewStringObj((char *)cell->str, -1));
          } else {
            Tcl_ListObjAppendElement(tclInterp, rowObj, Tcl_NewStringObj("", -1));
          }
          if (cell->rowspan > 1) {
            // FIXME: add empty rows?
          }

        }

        Tcl_ListObjAppendElement(tclInterp, sheetObj, rowObj);
      }

      xls_close_WS(pWS);
      Tcl_ListObjAppendElement(tclInterp, resultObj, sheetObj);
    }

    xls_close(pWB);
    result = TCL_OK;

  } else if (strcmp(Tcl_GetString(objv[1]), "open") == 0) {
    Tcl_AppendResult(tclInterp, "Not implemented", NULL);
  } else {
    Tcl_AppendResult(tclInterp, "Invalid subcommand ", Tcl_GetString(objv[1]), ", should be read or open", NULL);
  }

exit:
  Tcl_DStringFree(&e);
  Tcl_DStringFree(&s);
  return result;
};

