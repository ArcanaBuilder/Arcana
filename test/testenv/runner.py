#!/bin/python3

import os
from sys import argv
import dataclasses
import subprocess
import re
import shutil

@dataclasses.dataclass
class Test:
    path:            str = ''
    script:          str = ''
    expected_result: int = 0


ANSI_ESCAPE = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')


def scan_arc_tests(root):
    tests = []

    for dirpath, _, filenames in os.walk(root):
        for f in filenames:
            if f.endswith(".arc") or f == 'arcfile':
                path = os.path.join(dirpath, f)

                try:
                    with open(path, "r", encoding="utf-8") as fh:
                        content = fh.read()
                except Exception as e:
                    print(f"Cannot read {path}: {e}")
                    continue

                expected_result = 0
                if "negative" in content.lower():
                    expected_result = 255
                    
                tests.append( Test(path, os.path.splitext(os.path.basename(path))[0], expected_result) )

    arcfile_tests = [t for t in tests if t.script == "arcfile"]

    if arcfile_tests:
        tests = [arcfile_tests[0]]

    return tests


def run_test(test: Test):
    type: str = "\x1b[2m\033[32mPOSITIVE\033[0m" if test.expected_result == 0 else "\x1b[2m\033[31mNEGATIVE\033[0m"
    print(f'Running [{type}] test: {test.script:40s}', end='')

    cmd = ["arcana", "-s", test.path, "--debug"]

    res = subprocess.run(
        cmd,
        capture_output=True,
        text=True
    )

    return ANSI_ESCAPE.sub('', res.stdout), ANSI_ESCAPE.sub('', res.stderr), res.returncode


def exec_tests(root: str, tests: list[Test]) -> tuple:
    passed = 0
    failed = 0
    test_pwd  = f'{root}'

    if os.path.exists(test_pwd):
        shutil.rmtree(test_pwd)

    os.makedirs(test_pwd, exist_ok=True)

    tests.sort(key=lambda t: (t.expected_result, t.path))

    for test in tests:
        test_name = f'{test.script}.log'

        out, err, code = run_test(test)

        if code == test.expected_result:
            print("[\033[92mPASSED\033[0m]")
            passed += 1
        else:
            print("[\033[91mFAILED\033[0m]")
            failed += 1

        with open(f'{test_pwd}/{test_name}', "w", encoding="utf-8") as f:
            f.write(out)
            f.write(err)

    return (passed, failed)


campaigns = [ 'statement', 'variable', 'task', 'project' ]

for campagn in campaigns:
    print(f'''###############################################################################
# Running Test Campaign '{campagn}'
''')
    root:  str        = f"../{campagn}"
    tests: list[Test] = scan_arc_tests(root)

    if len(tests):
        passed, failed = exec_tests(campagn, tests)

        print(f'''
# Test Campaign '{campagn}'
- Passed:    {passed}
- Failed:    {failed}
- P/F Rateo: {(100*passed)/(passed+failed):.1f}%
###############################################################################



''')

