import rtconfig
from building import *

cwd     = GetCurrentDir()
CPPPATH = [cwd, str(Dir('#'))]
src     = Split("""
core/source/tp_manage.c
adapter/source/cmsis/tp_thread_adapter.c
adapter/source/cmsis/tp_mutex_adapter.c
adapter/source/cmsis/tp_sem_adapter.c
""")

CPPPATH += ['core/include']
CPPPATH += ['adapter/include']

group = DefineGroup('tp', src, depend = [''], CPPPATH = CPPPATH)

Return('group')
