#!/usr/bin/env python3
#
from check_helper import *

from XPLMDefs import *
from XPLMDataAccess import *
from XPLMScenery import *
from XPLMUtilities import *

import random

class PythonInterface(checkBase):
   def __init__(self):
      checkBase.__init__(self, 'Scenery');
   
   def XPluginStart(self):
      self.Name = "Scenery regression test"
      self.Sig = "SceneryRT"
      self.Desc = "Regression test for XPLMScenery module"
      self.versions = XPLMGetVersions()

      return self.Name, self.Sig, self.Desc
   
   def XPluginStop(self):
      self.check()
   
   def XPluginEnable(self):
      return 1
   
   def XPluginDisable(self):
      return

   def XPluginReceiveMessage(self, inFromWho, inMessage, inParam):
      structSize = XPLMFindDataRef("probe.structSize")
      locationX = XPLMFindDataRef("probe.locationX")
      locationY = XPLMFindDataRef("probe.locationY")
      locationZ = XPLMFindDataRef("probe.locationX")
      normalX = XPLMFindDataRef("probe.normalX")
      normalY = XPLMFindDataRef("probe.normalY")
      normalZ = XPLMFindDataRef("probe.normalZ")
      velocityX = XPLMFindDataRef("probe.velocityX")
      velocityY = XPLMFindDataRef("probe.velocityY")
      velocityZ = XPLMFindDataRef("probe.velocityZ")
      is_wet = XPLMFindDataRef("probe.is_wet")

      path = XPLMFindDataRef("obj.path")
      lighting = XPLMFindDataRef("obj.lighting")
      earth_relative = XPLMFindDataRef("obj.earth_relative")
      locations = XPLMFindDataRef("obj.locations")

      
      self.checkVal('Probe structure size mismatch.', XPLMGetDatai(structSize), 0)
      probe = XPLMCreateProbe(xplm_ProbeY)
      self.checkVal('Initialized probe structure size mismatch.', XPLMGetDatai(structSize), 44)
      info = []
      (x, y, z) = (123, 456, 789)
      res = XPLMProbeTerrainXYZ(probe, x, y, z, info)
      self.checkVal('XPLMProbeTerrain: structSize mismatch.', info[0], 44)
      self.checkVal('XPLMProbeTerrain: locationX mismatch.', info[1], x)
      self.checkVal('XPLMProbeTerrain: locationY mismatch.', info[2], y)
      self.checkVal('XPLMProbeTerrain: locationZ mismatch.', info[3], z)

      self.checkVal('XPLMProbeTerrain: normalX mismatch.', info[4], XPLMGetDataf(normalX))
      self.checkVal('XPLMProbeTerrain: normalY mismatch.', info[5], XPLMGetDataf(normalY))
      self.checkVal('XPLMProbeTerrain: normalZ mismatch.', info[6], XPLMGetDataf(normalZ))
      self.checkVal('XPLMProbeTerrain: velocityX mismatch.', info[7], XPLMGetDataf(velocityX))
      self.checkVal('XPLMProbeTerrain: velocityY mismatch.', info[8], XPLMGetDataf(velocityY))
      self.checkVal('XPLMProbeTerrain: velocityZ mismatch.', info[9], XPLMGetDataf(velocityZ))
      self.checkVal('XPLMProbeTerrain: is_wet mismatch.', info[10], XPLMGetDatai(is_wet))

      XPLMDestroyProbe(probe)
      self.checkVal('Probe structure size mismatch.', XPLMGetDatai(structSize), 0)

      self.lat, self.lon = 4321, 6789
      try:
        tmp = XPLMGetMagneticVariation(self.lat, self.lon);
      except RuntimeError as re:
         if (self.versions[1] >= 300) or (str(re) != 'XPLMGetMagneticVariation is available only in XPLM300 and up.'):
            raise
      else:
        self.checkVal('XPLMGetMagneticVariation', tmp, self.lat + self.lon**2)

      self.hdg = 49152
      try:
        tmp = XPLMDegTrueToDegMagnetic(self.hdg);
      except RuntimeError as re:
         if (self.versions[1] >= 300) or (str(re) != 'XPLMDegTrueToDegMagnetic is available only in XPLM300 and up.'):
            raise
      else:
        self.checkVal('XPLMDegTrueToDegMagnetic', tmp, self.hdg**3)

      self.hdg = 49153
      try:
        tmp = XPLMDegMagneticToDegTrue(self.hdg);
      except RuntimeError as re:
         if (self.versions[1] >= 300) or (str(re) != 'XPLMDegMagneticToDegTrue is available only in XPLM300 and up.'):
            raise
      else:
        self.checkVal('XPLMDegMagneticToDegTrue', tmp, 1 / self.hdg)

      tmp = 'path/to/object'
      self.objectDict = {}
      self.objectDict[tmp] = XPLMLoadObject(tmp)
      self.checkVal('XPLMLoadObject passed wrong path', self.getString(path), tmp)

      tmp = 'path/to/another/object'
      self.loadCbk = self.loadCallback
      self.refcon = [tmp]
      self.checkVal('XPLMLoadObjectAsync hash already contains a reference', self.objectDict.get(tmp), None)
      try:
         XPLMLoadObjectAsync(self, tmp, self.loadCbk, self.refcon)
      except RuntimeError as re:
         if (self.versions[1] >= 210) or (str(re) != 'XPLMLoadObjectAsync is available only in XPLM210 and up.'):
            raise
      else:
         self.checkVal('XPLMLoadObjectAsync didn\'t add a new object', len(self.objectDict), 2)
         self.checkVal('XPLMLoadObjectAsync passed wrong path', self.getString(path), tmp)

      tmp = 7
      loc = [[1000 * random.random() for i in range(0, 6)] for j in range(0, tmp)]


      for k, v in self.objectDict.items():
         XPLMDrawObjects(v, tmp, loc, 365, 485)
         self.checkVal('XPLMDrawObjects lighting', XPLMGetDatai(lighting), 365)
         self.checkVal('XPLMDrawObjects earth_relative', XPLMGetDatai(earth_relative), 485)
         for i in range(0, tmp):
            locData = []
            self.checkVal('XPLMDrawObjects locations 1', XPLMGetDatavf(locations, locData, i, 7), 7)
            self.checkVal('XPLMDrawObjects locations 2', locData[0], 28)
            self.checkVal('XPLMDrawObjects locations 3', locData[1:], loc[i])
         XPLMUnloadObject(v)
         self.checkVal('XPLMLoadObject passed wrong path',  self.getString(path), k + " deleted")

      self.objPath = '/test/path/'
      self.objEnumerator = self.enumeratorCallback
      self.objects = []
      self.checkVal('XPLMLookupObjects returned unexpected number', 
         XPLMLookupObjects(self, self.objPath, 3.1415926, 2.7182818, self.objEnumerator, self.objects), 2)
      self.checkVal('XPLMLookupData inLatitude problem', XPLMGetDataf(locationX), 3.1415926)
      self.checkVal('XPLMLookupData inLongitude problem', XPLMGetDataf(locationY), 2.71827818)
      self.checkVal('XPLMLookupData inPath problem', self.getString(path), self.objPath)

      for idx, path in enumerate(self.objects):
         self.checkVal('XPLMLookupData return path wrong', path, "{0}{1}".format(self.objPath, idx + 1))
      return

   def loadCallback(self, objRef, refcon):
      self.objectDict[refcon[0]] = objRef;

   def enumeratorCallback(self, inFilePath, inRef):
      inRef.append(inFilePath)
      self.checkVal('EnumeratorCallback path inconsistent', self.objects[-1], inRef[-1])


