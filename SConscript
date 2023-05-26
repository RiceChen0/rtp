import rtconfig
from building import *

cwd     = GetCurrentDir()
CPPPATH = [cwd, str(Dir('#'))]
src     = Split("""
src/rthread_pool.c
""")

CPPPATH = [cwd + '/include']

group = DefineGroup('rthread_pool', src, depend = [''], CPPPATH = CPPPATH)

Return('group')
