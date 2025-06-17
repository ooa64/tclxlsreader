#include "xlsreadercmd.hpp"
#include <string.h>
#include <xls.h>

using namespace xls;

#if defined(XLSREADERCMD_DEBUG)
#   include <iostream>
#   define DEBUGLOG(_x_) (std::cerr << "DEBUG: " << _x_ << "\n")
#else
#   define DEBUGLOG(_x_)
#endif

Tcl_Obj * Xls_NewNumberObj(xlsCell *cell) {
  // (abs(cell->d - floor(cell->d)) <= DBL_MIN)
  if (Tcl_WideInt(cell->d) == cell->d) 
    return Tcl_NewWideIntObj(Tcl_DoubleAsWide(cell->d));
  else 
    return Tcl_NewDoubleObj(cell->d);
  // {
  // char s[22];
  // snprintf(s, sizeof(s), "%.15g", cell->d);
  // return Tcl_NewStringObj(s, -1);
  // }
}            

Tcl_Obj * Xls_NewStringObj(xlsCell *cell, Tcl_Encoding encoding) {
  if (!encoding)
    return Tcl_NewStringObj((char *)cell->str, -1);
  Tcl_DString e;
  Tcl_DStringInit(&e);
  Tcl_ExternalToUtfDString(encoding, (char *)cell->str, -1, &e);
  Tcl_Obj * result = Tcl_NewStringObj(Tcl_DStringValue(&e), -1);
  Tcl_DStringFree(&e);
  return result;
}

Tcl_Obj * Xls_NewBoolerrObj(xlsCell *cell, Tcl_Encoding encoding) {
    if (strcmp((char *)cell->str, "bool") == 0) {
      return Tcl_NewBooleanObj((int)cell->d);
      // return Tcl_NewStringObj(((int)cell->d ? "true" : "false"), -1);
    } else if (strcmp((char *)cell->str, "error") == 0) {
      return Tcl_NewStringObj("*error*", -1);
    } else {
      return Xls_NewStringObj(cell, encoding);
    }
}

/*
  xlsreader read $xlsfile
    opens xls file, returns a list of lists (sheets/rows).
*/

int XlsreaderCmd::Command (int objc, Tcl_Obj * const objv[]) {
  if (objc != 3 && objc != 5) {
    Tcl_WrongNumArgs(tclInterp, 1, objv, "read <filename> ?-encoding <encoding>?");
    return TCL_ERROR;
  }
  Tcl_Encoding encoding = NULL;
  if (objc == 5) {
    if (strcmp(Tcl_GetString(objv[3]), "-encoding") != 0) {
      Tcl_AppendResult(tclInterp, "invalid arg \"", Tcl_GetString(objv[3]), "\": should be \"-encoding\"", NULL);
      return TCL_ERROR;
    }
    encoding = Tcl_GetEncoding(tclInterp, Tcl_GetString(objv[4]));
    if (encoding == NULL) {
      return TCL_ERROR;
    }
  }
  int result = TCL_ERROR;
  Tcl_DString s;
  Tcl_DString e;
  Tcl_DStringInit(&s);
  Tcl_DStringInit(&e);
  char * filename = Tcl_UtfToExternalDString(NULL,
      Tcl_TranslateFileName(tclInterp, Tcl_GetString(objv[2]), &s), -1, &e);
#ifdef XLS_DEBUG
  xls::xls(1);
#endif

  if (strcmp(Tcl_GetString(objv[1]), "read") == 0) {

    xls_error_t error = LIBXLS_OK;
    xlsWorkBook * pWB = xls_open_file(filename, "UTF-8", &error);
    if (!pWB) {
      Tcl_AppendResult(tclInterp, "error reading ", filename, ": ", xls_getError(error), NULL);
      goto exit;
    }
    if (!encoding) {
      if ((pWB->codepage >= 874 /*437*/ && pWB->codepage <= 950) || (pWB->codepage >= 1250 && pWB->codepage <= 1258)) {
        char s[7];
        snprintf(s, sizeof(s), "cp%d", pWB->codepage);
        encoding = Tcl_GetEncoding(NULL, s);
      }
    }
    Tcl_Obj * resultObj = Tcl_GetObjResult(tclInterp);

    for (unsigned sheet = 0; sheet < pWB->sheets.count; sheet++) {

      Tcl_Obj * sheetObj = Tcl_NewObj();
      xlsWorkSheet * pWS = xls_getWorkSheet(pWB, sheet);
      error = xls_parseWorkSheet(pWS);
      if (error) {
        Tcl_AppendResult(tclInterp, "error parsing sheet ", pWB->sheets.sheet[sheet].name, ": ", xls_getError(error), NULL);
        xls_close_WS(pWS);
        goto exit;
      }
      for (unsigned row = 0; row <= pWS->rows.lastrow; row++) {

        Tcl_Obj * rowObj = Tcl_NewObj();

        for (unsigned col = 0; col <= pWS->rows.lastcol; col++) {

          xlsCell *cell = xls_cell(pWS, row, col);
          if ((!cell) || (cell->isHidden)) {
            continue;
          }
          if (cell->id == XLS_RECORD_RK || cell->id == XLS_RECORD_MULRK || cell->id == XLS_RECORD_NUMBER) {
            Tcl_ListObjAppendElement(tclInterp, rowObj, Xls_NewNumberObj(cell));
          } else if (cell->id == XLS_RECORD_FORMULA || cell->id == XLS_RECORD_FORMULA_ALT) {
            if (cell->l == 0) {
              Tcl_ListObjAppendElement(tclInterp, rowObj, Xls_NewNumberObj(cell));
            } else if (cell->str) {
              Tcl_ListObjAppendElement(tclInterp, rowObj, Xls_NewBoolerrObj(cell, encoding));
            }
          } else if (cell->id == XLS_RECORD_BOOLERR) {
            Tcl_ListObjAppendElement(tclInterp, rowObj, Xls_NewBoolerrObj(cell, encoding));
          } else if (cell->str) {
            Tcl_ListObjAppendElement(tclInterp, rowObj, Xls_NewStringObj(cell, encoding));
          } else {
            Tcl_ListObjAppendElement(tclInterp, rowObj, Tcl_NewStringObj("", -1));
          }
          if (cell->colspan > 1) {
            for (unsigned i = 2; i <= cell->colspan; i++)
              Tcl_ListObjAppendElement(tclInterp, rowObj, Tcl_NewStringObj("", -1));
          }
          if (cell->rowspan > 1) {
            // FIXME ?
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
    Tcl_AppendResult(tclInterp, "not implemented", NULL);
  } else {
    Tcl_AppendResult(tclInterp, "invalid subcommand \"", Tcl_GetString(objv[1]), "\": should be read or open", NULL);
  }

exit:
#ifdef XLS_DEBUG
  xls::xls(0);
#endif
  if (encoding)
    Tcl_FreeEncoding(encoding);
  Tcl_DStringFree(&e);
  Tcl_DStringFree(&s);
  return result;
};

