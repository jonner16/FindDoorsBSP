#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
import glob
import difflib
import math
from subprocess import Popen, PIPE
from threading import Timer
from util import *
from itertools import zip_longest

MAX_TEST_RUN_TIME = 10
def timeoutProcess(process, verbose = False):
    if verbose:
        print("Test timed out. Killing...")
    process.kill()

def runTest(program, roomInFile, doorInFile, outFile, verbose = False):
    exit_code, err, output = run_program([program, roomInFile, doorInFile], verbose=verbose, timeout=MAX_TEST_RUN_TIME)
    if exit_code == 0:
        if((output is None) or (len(output) == 0)):
            print("Error: No output obtained from stdout!")
            return -1
        with open(outFile, "r") as f:
            expected = [l for l in f.readlines()]
        # Trim new lines at the end
        while expected[len(expected) - 1] == "\n":
            del expected[len(expected) - 1]
        while output[len(output) - 1] == "\n":
            del output[len(output) - 1]
        diffs = list(difflib.ndiff(expected, output))
        failedDiffs = [f"\t{i + 1:5d}|{l}" for i, l in enumerate(diffs) if l[0] in {"+", "-"}]
        if(len(expected) != len(output)):
            print(f"Error: Output length ({len(output)}) does not match expected length ({len(expected)})")
            return -1
        testResults = [exp.lower().strip() == out.lower().strip() for (exp, out) in zip_longest(expected, output)]
        if len(failedDiffs) > 0:
            if verbose:
                print("Test produced incorrect output:")
                print("".join(failedDiffs[:30]))
                if len(failedDiffs) > 30:
                    print(">> Diff truncated as too long.\n")
                
            return (len(testResults) - (sum(testResults)))
        else:
            if verbose:
                print("Test passed")
            return 0
    else:
        if verbose:
            if exit_code != 0:
                print(f"Program exited with code: {exit_code} ({get_code_reason(exit_code)})")
            if err is not None:
                print("Error occured during execution:", err)
        return -1


def testProgram(program, tests, verbose = False):
    if verbose > 1:
        print(f"Testing `{program}` with tests from: `{tests}`")
    if not os.path.isfile(program):
        if verbose > 0:
            print(f"Program `{program}` does not exist")
        return 0
    if not os.path.isdir(tests):
        if verbose > 0:
            print(f"Tests directory: `{tests}` does not exist")
        return None
    totalTests = 0
    testsPassed = 0
    testErrors = []
    for roomInFile in glob.glob(f"{tests}/room*.in"):
        # Find the corresponding door file
        testNumber = roomInFile.split("/")[-1][4:-3]
        doorInFile = tests + "door" + testNumber + ".in"
        # Find the corresponding out file
        outFile = tests + "test" + testNumber + ".out"
        if os.path.isfile(outFile):
            if verbose > 1:
                print(f"Running test: {testNumber}")
            totalTests += 1
            result = runTest(program, roomInFile, doorInFile, outFile, verbose = verbose > 1)
            if (result == 0):
                testErrors.append((outFile, result))
                testsPassed += 1
                if verbose == 1:
                    print("+", end="", flush=True)
            elif(result == -1):
                testErrors.append((outFile, result))
                if verbose == 1:
                    print("-", end="", flush=True)
            else:
                testErrors.append((outFile, result))
                if verbose == 1:
                    print("+-", end="", flush=True)
    pointsAchieved = 0
    if verbose == 1:
        print("\n", end="")
    for test in testErrors:
        if test[1] == -1:
            if verbose > 0:
                print(f"{test[0]} failed with error, resulting in 0 points.")
        elif test[1] >= 4:
            if verbose > 0:
                print(f"{test[0]} completed with >= 5 errors ({test[1]} errors), resulting in 0 points.")
        else:
            pointsEarned = 10 - round(2 * test[1])
            pointsAchieved += pointsEarned
            if verbose > 0:
                print(f"{test[0]} completed with {test[1]} errors, resulting in {pointsEarned} points.")
    if verbose == 1:
        print("\n", end="")
    if verbose > 0:
        totalPossible = totalTests * 10
        print(f"Given {totalTests} tests, each worth 10 points, you achieved {pointsAchieved} out of {totalPossible} points.")
    return pointsAchieved
                
    

if __name__ == '__main__':
    prog = sys.argv[1]
    testFolder = sys.argv[2]
    verbose = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    finalScore = testProgram(prog, testFolder, verbose=verbose)
    print(finalScore)
