# XPPython3 Installation Notes

## Requirements
* requires 64-bit
* requires python3, tested with
  + python37 and python38 on windows,
  + python37 on Mac,
  + ubuntu (python38)
* XP 11.50+. Plugin is built with SDK 303 and is NOT backward compatible to 11.41 or earlier.

Single plugin should work with any (post 3.3?) version of python3,
unlike the python2-based plugin which required minor version match.

It's called XPPython3:
* "xpython" already exists (it's an all python version of python)
* "3" to emphasize it is python 3, which is not fully backward compatible with python2

## Installation
This plugin XPPython3 folder should be placed in <XP>/Resources/plugins.
On startup, it will create the PythonPlugins folder, if you don't have
one.
```
  <X-Plane>/
  └─── Resources/
       └─── plugins/
            ├─── XPPython3/
            │    ├─── mac_x64/
            │    |    └─── xppython3.xpl
            │    ├─── lin_x64/
            │    |    └─── xppython3.xpl
            │    └─── win_x64/
            │         └─── xppython3.xpl
            └─── PythonPlugins/
                 │
                 ├─── PI_<plugin1>.py
                 ├─── PI_<plugin2>.py
                 └─── ....
```
    
Python plugins themselves go to:
* Resources/plugins/PythonPlugins/ folder
  + Single file named PI_<i>anything</i>.py for each separate python plugin. This is the starting point for each python plugin
    (For Python2, we used "PythonScripts" folder -- same idea, but we need a different folder to allow
    python2 and python3 to coexist.)
  + Plugins are loaded in order as returned by the OS: that is, do not assume
    alphabetically!
  + Python files can then import other python files
* Resources/plugins/XPPython3/ folder
  + "internal" plugins. This is intended for internal use, and are additional python plugins loaded
    prior to the user directory "PythonPlugins". Note this is (usually) the same folder as holding
    the binary *.xpl plugin files. To be read, files need to be named I_PI_<i>anything</i>.py.

## Plugin Menus
XPPython3 installs a single menu item, "XPPython3", on the plugin menu. From there, you can:
  * Start - only useful if you've stopped the scripts
  * Stop - Stop all scripts (kindof -- actually, we just stop sending messages to scripts,
           but they are still "running" as far as I can tell).  XPluginReceiveMessage() is disabled.
  * Reload - Stops and restarts python, reloading all scripts. While this works, note that some
    plugins do not clean up after themselves, so loading all python plugins may result in duplications.
    Best example is where a plugin doesn't remove it's menu on stop. On Reload, it will add _another_ menu
    of the same name: The best fix is to have the plugin author fix their code, otherwise restart X-Plane or
    just don't use Reload.

## Logging
### `Log.txt`

* Some messages go to Log.txt. Specifically, when python plugin itself is loaded:

        Loaded: <XP>/Resources/plugins/XPPython3/mac_x64/xppython3.xpl (XPPython3.0.0).
   
   If XPPython3 cannot load, you'll see an error in this log file.

* Common error on windows:

        <XP>/Resources/plugins/XPpython3/win.xpl: Error Code = 126 : The specified module could not be found.
     
   __Cause__: X-Plane cannot load all DLLs required by plugin. In this case, the python plugin is looking for python itself.
   Usually, python is installed in `C:\Program Files\Python3X folder`, where you'll find a file `python3.dll`.
   
   __Solution__:
   1. Python needs to be installed "for all users" -- that places the folder under \Program Files, if not for all
      users, it's stored somewhere else & X-Plane may not be able to find it.
   2. Python needs to be installed with "Set Environment Path" (*** need correct wording)
      This helps X-Plane find it -- perhaps, it could be stored "for single user" but PATH needs to set?

### `XPPython3.txt`

Python messages go to `<XP>/XPPython3.txt` (for python2 it was a couple files in the
<XP>/Resources/plugins/PythonScripts folder.) You can change location of this logfile
by setting environment variable `XPPYTHON3_LOG`. Log is re-written each time (Python2,
we appended to the file rather than clearing it out.) If you want to preserve
the contents of the logfile, set environment variable `XPPYTHON3_PRESERVE`.

Log always contains:

    XPPython3 Version x.x.x Started.

Then the script folder(s) are scanned. If the folder cannot be found (not an error really, but just to
let you know):

    Can't open <folder> to scan for plugins.

On _each_ python plugin startup, we print:

    PI_<plugin> initialized.
        Name: <plugin name>
        Sig:  <plugin signature>
        Desc: <plugin description>


Successful shutdown will included:

    XPPython Stopped.

## Errors
If you have errors running python plugins,
1. Check `Log.txt`. Make sure python and the python plugin are installed correctly. If not,
   there will be a message in Log.txt, and XPPython3.txt will not be created. Verify it's Python3, not Python2
   getting loaded.
2. Check XPPython3.txt. Most python coding errors / exceptions will be reported in this
   log.

You should provide both Log.txt and XPPythonLog.txt when looking for assistance with a python plugin.
