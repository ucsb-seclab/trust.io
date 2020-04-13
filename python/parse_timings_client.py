import sys
import numpy

f = open(sys.argv[1])
logs = {'on': [],
    'off': [],
    'read':[]
}
for line in f:
    split_line = line.split()
    if split_line[0] == "'on\\r\\n'":
        logs['on'].append(float(split_line[1]))
    elif split_line[0] == "'off\\r\\n'":
        logs['off'].append(float(split_line[1]))
    elif split_line[0] == "read":
        logs['read'].append(float(split_line[1]))

# print logs

for cmd in logs:
    print "Timing for (ms)", cmd
    print len(logs[cmd])
    print numpy.mean(logs[cmd][:2000])*1000
    print numpy.std(logs[cmd][:2000])*1000
    print ""
#     print numpy.mean(logs[cmd]['callback'][:2000]) * 1000.0 / (
#         333.33333333 * 1000000)
#     print numpy.std(logs[cmd]['callback'][:2000]) * 1000.0 / (
#         333.33333333 * 1000000)
#     print ""
#     print numpy.mean(logs[cmd]['verify_return'][:2000]) * 1000.0 / (
#         333.33333333 * 1000000)
#     print numpy.std(logs[cmd]['verify_return'][:2000]) * 1000.0 / (
#         333.33333333 * 1000000)
#     print ""
