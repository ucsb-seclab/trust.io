import sys
import numpy

f = open(sys.argv[1])
logs = {'on': {
    'data_abort': [],
    'callback': [],
    'verify_return': [],
},
    'off': {
        'data_abort': [],
        'callback': [],
        'verify_return': [],
    },
    'read': {
        'data_abort': [],
        'callback': [],
        'verify_return': [],
    }
}
data_abort = []
callback = []
verify_return = []
for line in f:
    '''
    xil_printf("--- off ---\r\n");
    print_time(START_TIME);
    print_time(CRYPTO_CALL);
    print_time(CRYPTO_RESPONSE);
    print_time(TIO_RETURN);
    '''
    if line.startswith("---"):
        cmd = line.split(" ")[1]
        if cmd == "off":
            cmd = "on"
        start = f.next()
        start = int(start.strip(), 16)
        crypto_call = int(f.next().strip(), 16)
        crypto_response = int(f.next().strip(), 16)
        tio_return = int(f.next().strip(), 16)

        logs[cmd]['data_abort'].append(crypto_call - start)
        logs[cmd]['callback'].append(crypto_response - crypto_call)
        logs[cmd]['verify_return'].append(tio_return - crypto_response)

for cmd in logs:
    print "Timing for ", cmd
    print len(logs[cmd]['data_abort'])
    print numpy.mean(logs[cmd]['data_abort'][:2000]) * 1000.0 / (
        333.33333333 * 1000000)
    print numpy.std(logs[cmd]['data_abort'][:2000]) * 1000.0 / (
        333.33333333 * 1000000)
    print ""
    print numpy.mean(logs[cmd]['callback'][:2000]) * 1000.0 / (
        333.33333333 * 1000000)
    print numpy.std(logs[cmd]['callback'][:2000]) * 1000.0 / (
        333.33333333 * 1000000)
    print ""
    print numpy.mean(logs[cmd]['verify_return'][:2000]) * 1000.0 / (
        333.33333333 * 1000000)
    print numpy.std(logs[cmd]['verify_return'][:2000]) * 1000.0 / (
        333.33333333 * 1000000)
    print ""
