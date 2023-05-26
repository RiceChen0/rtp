import rtconfig
from building import *

cwd     = GetCurrentDir()
CPPPATH = [cwd, str(Dir('#'))]
src     = Split("""
src/rthread_pool.c
platform/rtthread/pf_task_adapter.c
platform/rtthread/pf_mutex_adapter.c
platform/rtthread/pf_sem_adapter.c
platform/rtthread/pf_event_adapter.c
""")

CPPPATH = [cwd + '/include']
CPPPATH += [cwd + '/platform']

group = DefineGroup('rthread_pool', src, depend = [''], CPPPATH = CPPPATH)

Return('group')
