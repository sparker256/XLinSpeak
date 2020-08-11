#!/usr/bin/env python3
#
from check_helper import *

from XPLMDefs import *
from XPLMDataAccess import *
from XPLMPlugin import *

class PythonInterface(checkBase):
   def __init__(self):
      checkBase.__init__(self, 'Plugin');
   
   def XPluginStart(self):
      self.Name = "Plugin regression test"
      self.Sig = "PluginRT"
      self.Desc = "Regression test for XPLMPlugin module"

      self.id = 4
      # to set the id (XPLMGetMyID, ... in chkPlugin.cpp)
      XPLMIsPluginEnabled(self.id)
      self.msgCntr = 43
      return self.Name, self.Sig, self.Desc
   
   def XPluginStop(self):
      self.check()
   
   def XPluginEnable(self):
      current = self.msgCntr
      XPLMSendMessageToPlugin(XPLMGetMyID(), 104, current)
      self.checkVal('XPLMSendMessageToPlugin didn\'t pass the inParam', self.msgCntr, current + 42)
      return 1
   
   def XPluginDisable(self):
      return

   def XPluginReceiveMessage(self, inFromWho, inMessage, inParam):
      self.id = 4
      XPLMIsPluginEnabled(self.id)
      self.checkVal('XPlugin Receive message passed incorrect inFromWho', inFromWho, 5)
      if inMessage == 104:
        self.checkVal('XPlugin Receive message passed incorrect inParam', inParam, self.msgCntr)
      else:
        self.checkVal('XPlugin Receive message passed incorrect inParam', inParam, 333)
      self.msgCntr += 42

      self.checkVal('XPLMGetMyID returned wrong value', XPLMGetMyID(), self.id + 1)
      self.checkVal('XPLMCountPlugins returned wrong value', XPLMCountPlugins(), self.id + 2)
      tmp = 1023
      self.checkVal('XPLMGetNthPlugin returned wrong value', XPLMGetNthPlugin(tmp), tmp + 1)
      path = '/path/to/plugin'
      signature = '+++'
      self.checkVal('XPLMFindPluginByPath returned wrong value', XPLMFindPluginByPath(path), self.id + 3)
      self.checkVal('XPLMFindPluginBySignature returned wrong value', XPLMFindPluginBySignature(signature), self.id + 4)
      outName = []
      outFilePath = []
      outSignature = []
      outDescription = []
      self.id = 2047
      XPLMGetPluginInfo(self.id, outName, outFilePath, outSignature, outDescription)
      self.checkVal('XPLMGetPluginInfo didn\'t pass the inPlugin correctly', XPLMGetMyID(), self.id + 1)
      self.checkVal('XPLMGetPluginInfo didn\'t get the outName correctly', outName[0], "plugin name")
      self.checkVal('XPLMGetPluginInfo didn\'t get the outFilePath correctly', outFilePath[0], path)
      self.checkVal('XPLMGetPluginInfo didn\'t get the outSignature correctly', outSignature[0], signature)
      self.checkVal('XPLMGetPluginInfo didn\'t get the outDescription correctly', outDescription[0], "plugin description")

      self.id = 4095
      self.checkVal('XPLMIsPluginEnabled returned wrong value', XPLMIsPluginEnabled(self.id), self.id + 100)
      self.checkVal('XPLMEnablePlugin returned wrong value', XPLMEnablePlugin(8192), 8192 + 101)
      XPLMDisablePlugin(tmp)
      self.checkVal('XPLMDisablePlugin returned wrong value', XPLMGetMyID(), self.id + tmp + 1)
      XPLMReloadPlugins()
      self.checkVal('XPLMReload plugins problem', XPLMGetMyID(), 0)

      features = dict(A = 44, B = 55)
      for feature,val in features.items():
         XPLMEnableFeature(feature, val)
      self.checkVal('XPLMHasFeature returned wrong value', XPLMHasFeature('C'), 0)
      for feature,val in features.items():
         self.checkVal('XPLMHasFeature returned wrong value', XPLMHasFeature(feature), 1)
         self.checkVal('XPLMIsFeatureEnabled returned wrong value', XPLMIsFeatureEnabled(feature), val)
      self.fEnum = self.featureEnumerator
      self.keys = []
      XPLMEnumerateFeatures(self, self.fEnum, self.keys)
      exp = [x for x in features.keys()]
      exp.sort()
      self.checkVal('XPLMEnumerateFeatures not OK', self.keys, exp)
      self.id = 4
      XPLMIsPluginEnabled(self.id)
      return

   def featureEnumerator(self, name, ref):
      ref.append(name)
      


