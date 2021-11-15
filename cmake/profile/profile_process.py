with open("fp_profiling", "r") as fh:
    lines = [line.strip().split() for line in fh.readlines()]

totals = {}
lasts = {}
for stage, token, module, stamp in lines:
    stamp = float(stamp)
    if token not in totals:
        totals[token] = (0, 0, 9999999999, 0, [])
    if stage == "START":
        lasts[token] = stamp
    elif stage == "STOP":
        accum, count, mind, maxd, lsts = totals[token]
        delta = stamp - lasts[token]
        totals[token] = (accum + delta, count + 1, min(delta, mind), max(delta, maxd), lsts + [(delta, module)])

for token, pair in totals.items():
    accum, count, mind, maxd, lsts = totals[token]
    if count == 0:
        continue
    print("{} {:.6f} ({:.6f}, {:.6f}, {:.6f}) {}".format(token, accum, mind, accum/count, maxd, lsts[int(count/2)][0]))
    hist = {index: (0, []) for index in range(0, 1000)}
    for item, module in lsts:
        index = int(int(item * 1000)/10) * 10
        hist[index] = (hist[index][0] + 1, hist[index][1] + [module])
    for index, pair in hist.items():
        count, members = pair
        if count != 0:
            print("    {} ms: {}        -- {}".format(index, count, " ".join(members[:5])))



