

set python print-stack full
python

import re

def gdb_unescape(string):
    str_len = len(string)
    if str_len % 2: # check for unexpected string length
        return ""
    result = bytearray()
    try:
        pos = 0
        while pos < str_len:
            hex_char = string[pos:pos+2]
            result.append(int(hex_char, 16))
            pos += 2
    except: # check for unexpected string value
        return ""
    return result.decode('utf-8')

def gdb_escape(string):
    result = ""
    for curr_char in string.encode('utf-8'):
        result += format(curr_char, '02x')
    return result

class RRWhere(gdb.Command):
    """Helper to get the location for checkpoints/history. Used by auto-args"""
    def __init__(self):
        gdb.Command.__init__(self, 'rr-where',
                             gdb.COMMAND_USER, gdb.COMPLETE_NONE, False)

    def invoke(self, arg, from_tty):
#Get the symbol name from 'frame 0' in the format:
# '#0  0x00007f9d81a04c46 in _dl_start (arg=0x7ffee1f1c740) at rtld.c:356
# 356 in rtld.c'
        try:
            rv = gdb.execute('frame 0', to_string=True)
        except:
            rv = "???" # This may occurs if we're not running
        m = re.match("#0\w*(.*)", rv);
        if m:
            rv = m.group(1)
        else:
            rv = rv + "???"
        gdb.write(rv)

RRWhere()

class RRDenied(gdb.Command):
    """Helper to prevent use of breaking commands. Used by auto-args"""
    def __init__(self):
        gdb.Command.__init__(self, 'rr-denied',
                             gdb.COMMAND_USER, gdb.COMPLETE_NONE, False)

    def invoke(self, arg, from_tty):
        raise gdb.GdbError("Execution of '" + arg + "' is not possible in recorded executions.")

RRDenied()

class RRCmd(gdb.Command):
    def __init__(self, name, auto_args):
        gdb.Command.__init__(self, name,
                             gdb.COMMAND_USER, gdb.COMPLETE_NONE, False)
        self.cmd_name = name
        self.auto_args = auto_args

    def invoke(self, arg, from_tty):
        args = gdb.string_to_argv(arg)
        self.rr_cmd(args)

    def rr_cmd(self, args):
        cmd_prefix = "maint packet qRRCmd:" + gdb_escape(self.cmd_name)
        argStr = ""
        for auto_arg in self.auto_args:
            argStr += ":" + gdb_escape(gdb.execute(auto_arg, to_string=True))
        for arg in args:
            argStr += ":" + gdb_escape(arg)
        rv = gdb.execute(cmd_prefix + argStr, to_string=True);
        rv_match = re.search('received: "(.*)"', rv, re.MULTILINE);
        if not rv_match:
            gdb.write("Response error: " + rv)
            return
        response = gdb_unescape(rv_match.group(1))
        if response != '\n':
            gdb.write(response)

def history_push(p):
    # ensure any output (e.g. produced by breakpoint commands running during our
    # processing, that were triggered by the stop we've been notified for)
    # is echoed as normal.
    gdb.execute("rr-history-push")

rr_suppress_run_hook = False

class RRHookRun(gdb.Command):
    def __init__(self):
        gdb.Command.__init__(self, 'rr-hook-run',
                             gdb.COMMAND_USER, gdb.COMPLETE_NONE, False)
        
    def invoke(self, arg, from_tty):  
      thread = int(gdb.parse_and_eval("$_thread"))
      if thread != 0 and not rr_suppress_run_hook:
        gdb.execute("stepi")
     
class RRSetSuppressRunHook(gdb.Command):
    def __init__(self):
        gdb.Command.__init__(self, 'rr-set-suppress-run-hook',
                             gdb.COMMAND_USER, gdb.COMPLETE_NONE, False)
        
    def invoke(self, arg, from_tty):
      rr_suppress_run_hook = arg == '1'

RRHookRun()
RRSetSuppressRunHook()

#Automatically push an history entry when the program execution stops
#(signal, breakpoint).This is fired before an interactive prompt is shown.
#Disabled for now since it's not fully working.
gdb.events.stop.connect(history_push)

end
python RRCmd('elapsed-time', [])
document elapsed-time
Print elapsed time (in seconds) since the start of the trace, in the 'record' timeline.
end
python RRCmd('when', [])
document when
Print the current rr event number.
end
python RRCmd('when-ticks', [])
document when-ticks
Print the current rr tick count for the current thread.
end
python RRCmd('when-tid', [])
document when-tid
Print the real tid for the current thread.
end
python RRCmd('rr-history-push', [])
document rr-history-push
Push an entry into the rr history.
end
python RRCmd('back', [])
document back
Go back one entry in the rr history.
end
python RRCmd('forward', [])
document forward
Go forward one entry in the rr history.
end
python RRCmd('checkpoint', ['rr-where'])
document checkpoint
create a checkpoint representing a point in the execution
use the 'restart' command to return to the checkpoint
end
python RRCmd('delete checkpoint', [])
document delete checkpoint
remove a checkpoint created with the 'checkpoint' command
end
python RRCmd('info checkpoints', [])
document info checkpoints
list all checkpoints created with the 'checkpoint' command
end

define hookpost-back
maintenance flush register-cache
frame
end

define hookpost-forward
maintenance flush register-cache
frame
end
define restart
  run c$arg0
end
document restart
restart at checkpoint N
checkpoints are created with the 'checkpoint' command
end
define seek-ticks
  run t$arg0
end
document seek-ticks
restart at given ticks value
end
define jump
  rr-denied jump
end
define hook-run
  rr-hook-run
end
define hookpost-continue
  rr-set-suppress-run-hook 1
end
define hookpost-step
  rr-set-suppress-run-hook 1
end
define hookpost-stepi
  rr-set-suppress-run-hook 1
end
define hookpost-next
  rr-set-suppress-run-hook 1
end
define hookpost-nexti
  rr-set-suppress-run-hook 1
end
define hookpost-finish
  rr-set-suppress-run-hook 1
end
define hookpost-reverse-continue
  rr-set-suppress-run-hook 1
end
define hookpost-reverse-step
  rr-set-suppress-run-hook 1
end
define hookpost-reverse-stepi
  rr-set-suppress-run-hook 1
end
define hookpost-reverse-finish
  rr-set-suppress-run-hook 1
end
define hookpost-run
  rr-set-suppress-run-hook 0
end
set unwindonsignal on
handle SIGURG stop
set prompt (rr) 
python
import re
m = re.compile('[^0-9]*([0-9]+)\.([0-9]+)(\.([0-9]+))?').match(gdb.VERSION)
ver = int(m.group(1))*10000 + int(m.group(2))*100
if m.group(4):
    ver = ver + int(m.group(4))

if ver == 71100:
    gdb.write('This version of gdb (7.11.0) has known bugs that break rr. Install 7.11.1 or later.\n', gdb.STDERR)

if ver < 71101:
    gdb.execute('set target-async 0')
    gdb.execute('maint set target-async 0')
end
