#define _GNU_SOURCE 1
#include <Python.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdbool.h>
#include <XPLM/XPLMDefs.h>
#include <XPLM/XPLMPlanes.h>
#include "utils.h"
#include "plugin_dl.h"

static intptr_t availableCntr;
static PyObject *availableDict;

PyObject *XPLMSetUsersAircraftFun(PyObject *self, PyObject *args)
{
  (void)self;
  const char *inAircraftPath;
  if(!PyArg_ParseTuple(args, "s", &inAircraftPath)){
    return NULL;
  }
  XPLMSetUsersAircraft(inAircraftPath);
  Py_RETURN_NONE;
}

PyObject *XPLMPlaceUserAtAirportFun(PyObject *self, PyObject *args)
{
  (void)self;
  const char *inAirportCode;
  if(!PyArg_ParseTuple(args, "s", &inAirportCode)){
    return NULL;
  }
  XPLMPlaceUserAtAirport(inAirportCode);
  Py_RETURN_NONE;
}

PyObject *XPLMPlaceUserAtLocationFun(PyObject *self, PyObject *args)
{
  (void)self;
  double latitudeDegrees, longitudeDegrees;
  float elevationMetersMSL, headingDegreesTrue, speedMetersPerSecond;
  if(!XPLMPlaceUserAtLocation_ptr){
    PyErr_SetString(PyExc_RuntimeError , "XPLMPlaceUserAtLocation is available only in XPLM300 and up.");
    return NULL;
  }
  if(!PyArg_ParseTuple(args, "ddfff", &latitudeDegrees, &longitudeDegrees,
                       &elevationMetersMSL, &headingDegreesTrue, &speedMetersPerSecond)){
    return NULL;
  }
  XPLMPlaceUserAtLocation_ptr(latitudeDegrees, longitudeDegrees,
                              elevationMetersMSL, headingDegreesTrue, speedMetersPerSecond);
  Py_RETURN_NONE;
}

PyObject *XPLMCountAircraftFun(PyObject *self, PyObject *args)
{
  (void)self;
  (void)args;
  int outTotalAircraft, outActiveAircraft;
  XPLMPluginID outController;

  XPLMCountAircraft(&outTotalAircraft, &outActiveAircraft, &outController);
  return Py_BuildValue("(iii)", outTotalAircraft, outActiveAircraft, outController);
}

PyObject *XPLMGetNthAircraftModelFun(PyObject *self, PyObject *args)
{
  (void)self;
  int inIndex;
  char outFileName[512];
  char outPath[512];
  if(!PyArg_ParseTuple(args, "i", &inIndex)){
    return NULL;
  }

  XPLMGetNthAircraftModel(inIndex, outFileName, outPath);
  return Py_BuildValue("(ss)", outFileName, outPath);
}

void planesAvailable(void *inRefcon)
{
  PyObject *pID = PyLong_FromVoidPtr(inRefcon);
  PyObject *callback = PyDict_GetItem(availableDict, pID);
  Py_XDECREF(pID);
  if(callback == NULL){
    printf("Unknown callback (%p) requested in planesAvailable.", inRefcon);
    return;
  }
  PyObject *res = PyObject_CallFunctionObjArgs(PyTuple_GetItem(callback, 2), PyTuple_GetItem(callback, 3), NULL);
  PyObject *err = PyErr_Occurred();
  if(err){
    printf("Error occured during the planesAvailable callback(inRefcon = %p):\n", inRefcon);
    PyErr_Print();
  }
  Py_XDECREF(res);
}

PyObject *XPLMAcquirePlanesFun(PyObject *self, PyObject *args)
{
  (void)self;
  PyObject *pluginSelf, *aircraft, *inCallback, *inRefcon;
  if(!PyArg_ParseTuple(args, "OOOO", &pluginSelf, &aircraft, &inCallback, &inRefcon)){
    PyErr_Clear();
    if(!PyArg_ParseTuple(args, "OOO", &aircraft, &inCallback, &inRefcon)){
      return NULL;
    }
    pluginSelf = get_pluginSelf(/*PyThreadState_GET()*/);
  }
  int res;
  void *refcon = (void*)++availableCntr;
  PyObject *refObj = PyLong_FromVoidPtr(refcon);
  PyObject *argsObj = Py_BuildValue( "(OOOO)", pluginSelf, aircraft, inCallback, inRefcon);
  PyDict_SetItem(availableDict, refObj, argsObj);
  Py_DECREF(argsObj);
  Py_DECREF(refObj);
  if(aircraft == Py_None){
    res = XPLMAcquirePlanes(NULL, planesAvailable, refcon);
  }else{
    Py_ssize_t len = PySequence_Length(aircraft);
    char **inAircraft = (char **)malloc((len + 1) * sizeof(char *));
    Py_ssize_t i;
    for(i = 0; i < len; ++i){
      PyObject *tmpItem = PySequence_GetItem(aircraft, i);
      PyObject *tmpStr = PyObject_Str(tmpItem);
      PyObject *tmpObj = PyUnicode_AsUTF8String(tmpStr);
      char *tmp = PyBytes_AsString(tmpObj);

      Py_DECREF(tmpItem);
      Py_DECREF(tmpStr);

      if (PyErr_Occurred()) return NULL;
      if(tmp[0] == '\0'){
        Py_DECREF(tmpObj);
        break;
      }else{
        inAircraft[i] = strdup(tmp);
      }
      Py_DECREF(tmpObj);
    }
    inAircraft[i] = NULL;
    res = XPLMAcquirePlanes(inAircraft, planesAvailable, refcon);
    i = 0;
    while(inAircraft[i]){
      free(inAircraft[i]);
      ++i;
    }
    free(inAircraft);
  }
  return PyLong_FromLong(res);
}





PyObject *XPLMReleasePlanesFun(PyObject *self, PyObject *args)
{
  (void)self;
  (void)args;
  XPLMReleasePlanes();
  Py_RETURN_NONE;
}

PyObject *XPLMSetActiveAircraftCountFun(PyObject *self, PyObject *args)
{
  (void)self;
  int inCount;
  if(!PyArg_ParseTuple(args, "i", &inCount)){
    return NULL;
  }

  XPLMSetActiveAircraftCount(inCount);
  Py_RETURN_NONE;
}

PyObject *XPLMSetAircraftModelFun(PyObject *self, PyObject *args)
{
  (void)self;
  int inIndex;
  const char *inAircraftPath;
  if(!PyArg_ParseTuple(args, "is", &inIndex, &inAircraftPath)){
    return NULL;
  }
  XPLMSetAircraftModel(inIndex, inAircraftPath);
  Py_RETURN_NONE;
}

PyObject *XPLMDisableAIForPlaneFun(PyObject *self, PyObject *args)
{
  (void)self;
  int inPlaneIndex;
  if(!PyArg_ParseTuple(args, "i", &inPlaneIndex)){
    return NULL;
  }

  XPLMDisableAIForPlane(inPlaneIndex);
  Py_RETURN_NONE;
}

#if defined(XPLM_DEPRECATED)
PyObject *XPLMDrawAircraftFun(PyObject *self, PyObject *args)
{
  (void)self;
  int inPlaneIndex, inFullDraw;
  float inX, inY, inZ, inPitch, inRoll, inYaw;
  PyObject *drawStateInfoSeq, *tmp;
  XPLMPlaneDrawState_t inDrawStateInfo;
  if(!PyArg_ParseTuple(args, "iffffffiO", &inPlaneIndex, &inX, &inY, &inZ, &inPitch,
                                          &inRoll, &inYaw, &inFullDraw, &drawStateInfoSeq)){
    return NULL;
  }
  PyObject *drawStateInfo = PySequence_Tuple(drawStateInfoSeq);
  inDrawStateInfo.structSize = sizeof(XPLMPlaneDrawState_t);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 1));
  inDrawStateInfo.gearPosition = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 2));
  inDrawStateInfo.flapRatio = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 3));
  inDrawStateInfo.spoilerRatio = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 4));
  inDrawStateInfo.speedBrakeRatio = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 5));
  inDrawStateInfo.slatRatio = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 6));
  inDrawStateInfo.wingSweep = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 7));
  inDrawStateInfo.thrust = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 8));
  inDrawStateInfo.yokePitch = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 9));
  inDrawStateInfo.yokeHeading = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);
  tmp = PyNumber_Float(PyTuple_GetItem(drawStateInfo, 10));
  inDrawStateInfo.yokeRoll = PyFloat_AsDouble(tmp);
  Py_DECREF(tmp);

  XPLMDrawAircraft(inPlaneIndex, inX, inY, inZ, inPitch, inRoll, inYaw, inFullDraw, &inDrawStateInfo);
  Py_RETURN_NONE;
}
#endif

static PyObject *cleanup(PyObject *self, PyObject *args)
{
  (void) self;
  (void) args;
  PyDict_Clear(availableDict);
  Py_DECREF(availableDict);
  Py_RETURN_NONE;
}



#if defined(XPLM_DEPRECATED)
/* WARNING: DO NOT USE.  Use XPLMPlaceUserAtAirport or XPLMPlaceUserAtLocation.*/
PyObject *XPLMReinitUsersPlaneFun(PyObject *self, PyObject *args)
{
  (void)self;
  (void)args;
  XPLMReinitUsersPlane();
  Py_RETURN_NONE;
}
#endif

static PyMethodDef XPLMPlanesMethods[] = {
  {"XPLMSetUsersAircraft", XPLMSetUsersAircraftFun, METH_VARARGS, ""},
  {"XPLMPlaceUserAtAirport", XPLMPlaceUserAtAirportFun, METH_VARARGS, ""},
  {"XPLMPlaceUserAtLocation", XPLMPlaceUserAtLocationFun, METH_VARARGS, ""},
  {"XPLMCountAircraft", XPLMCountAircraftFun, METH_VARARGS, ""},
  {"XPLMGetNthAircraftModel", XPLMGetNthAircraftModelFun, METH_VARARGS, ""},
  {"XPLMReleasePlanes", XPLMReleasePlanesFun, METH_VARARGS, ""},
  {"XPLMSetActiveAircraftCount", XPLMSetActiveAircraftCountFun, METH_VARARGS, ""},
  {"XPLMAcquirePlanes", XPLMAcquirePlanesFun, METH_VARARGS, ""},
  {"XPLMSetAircraftModel", XPLMSetAircraftModelFun, METH_VARARGS, ""},
  {"XPLMDisableAIForPlane", XPLMDisableAIForPlaneFun, METH_VARARGS, ""},
#if defined(XPLM_DEPRECATED)
  {"XPLMDrawAircraft", XPLMDrawAircraftFun, METH_VARARGS, ""},
#endif
#if defined(XPLM_DEPRECATED)
  {"XPLMReinitUsersPlane", XPLMReinitUsersPlaneFun, METH_VARARGS, ""},
#endif
  {"cleanup", cleanup, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL}
};

static struct PyModuleDef XPLMPlanesModule = {
  PyModuleDef_HEAD_INIT,
  "XPLMPlanes",
  NULL,
  -1,
  XPLMPlanesMethods,
  NULL,
  NULL,
  NULL,
  NULL
};

PyMODINIT_FUNC
PyInit_XPLMPlanes(void)
{
  if(!(availableDict = PyDict_New())){
    return NULL;
  }
  PyObject *mod = PyModule_Create(&XPLMPlanesModule);
  if(mod){
    PyModule_AddIntConstant(mod, "XPLM_USER_AIRCRAFT", XPLM_USER_AIRCRAFT);

  }

  return mod;
}



