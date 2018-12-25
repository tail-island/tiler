
import subprocess
import random

from datetime import datetime
from pathlib  import Path


def evaluate(a, b):
    with open('configuration.hpp', 'w') as f:
        f.write('#pragma once\n')
        f.write('\n')
        f.write('namespace tiler {\n')
        f.write('  constexpr auto A = {0};\n'.format(a))
        f.write('  constexpr auto B = {0};\n'.format(b))
        f.write('}\n')

    subprocess.run('MSBuild /p:configuration=release', stdout=subprocess.DEVNULL)

    total_score = 0

    for path in sorted(Path('./questions').glob('*.txt')):
        with open(path) as f:
            result = subprocess.run('x64/release/tiler.exe', stdin=f, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)

        score = int(result.stderr.decode('utf-8') or '0')

        total_score += score

    return total_score


a = 100
b = 400

score = evaluate(a, b)

for _ in range(1000):
    a_ = abs(a + random.randrange(-3, 3) * 50) or 50
    b_ = abs(b + random.randrange(-3, 3) * 50) or 50

    score_ = evaluate(a_, b_)
    print('{}-{}: {} vs {}-{}: {}, {}'.format(a_, b_, score_, a, b, score, datetime.now()))

    if score_ >= score:
        a = a_
        b = b_

        score = score_
