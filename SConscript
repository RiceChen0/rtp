import rtconfig
from building import *

cwd     = GetCurrentDir()
CPPPATH = [cwd, str(Dir('#'))]
CPPPATH = [cwd + '/include']

src     = Split("""
src/rtp.c
adapter/rtthread/rtp_mutex.c
adapter/rtthread/rtp_sem.c
adapter/rtthread/rtp_task.c
""")

group = DefineGroup('rtp', src, depend = [''], CPPPATH = CPPPATH)

Return('group')
