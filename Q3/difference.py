

import sys
import numpy as np

y = list(map(lambda x: float(x.strip().split()[-1]), open(sys.argv[1]).readlines()))
x = list(map(lambda x: float(x.strip().split()[-1]), open(sys.argv[2]).readlines()))

l = min(len(x), len(y))

print('Difference: ', abs(np.array(x[:l]) - np.array(y[:l])).sum())
