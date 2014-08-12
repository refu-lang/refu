import subprocess
import shutil
import sys
import os


Import('env')

extra_opts = "REFU_MODULES=STRING,PARALLEL,INTRUSIVE_LIST,TEXTFILE,HASHMAP "
extra_opts += "HASHMAP=generic "

# edit this is if you have python2 as python in your system
python_exec = sys.executable
if not env.GetOption('clean'):
    dbg = "--debug=explain"

    sconsCall = "scons static COMPILER=gcc {} {}".format(extra_opts, dbg)
    p = subprocess.Popen(sconsCall, cwd=env['CLIB_DIRECTORY'],
                         shell=True)
    ret = p.poll()
    while ret is None:
        ret = p.poll()

    if ret != 0:
        print("Building Refu C library failed. Not proceeding to build "
              "the Refu compiler")
        Exit(1)

else:  # scons was called with -c  (so better clean the c library too
    subprocess.call(["scons", "-c", "static", "shared"],
                    cwd=env['CLIB_DIRECTORY'])
