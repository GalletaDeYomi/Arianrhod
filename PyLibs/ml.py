import sys

if sys.winver == '2.7':
    from immdbg import *
else:
    from MyPyLibrary import *
    from MyPyLibrary import WinTypes

sys.dont_write_bytecode = True
