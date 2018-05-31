#!/usr/bin/env python

import os.path
from optparse import OptionParser

# global configs
lineMaxSize = 12

# menu and options
optionParser = OptionParser("usage: %prog --in <input_file> --out <output_file>")
optionParser.add_option("--in",
                        "--input",
                        action="store",
                        type="string",
                        dest="inputFile",
                        help="Input file to convert to C format")
optionParser.add_option("--out",
                        "--output",
                        action="store",
                        type="string",
                        dest="outputFile",
                        help="Output file to save C format in")
(options, args) = optionParser.parse_args()

# error checking for input and output files
if (not options.inputFile):
  optionParser.error("input file argument missing")
if (not options.outputFile):
  optionParser.error("output file argument missing")
if (not os.path.exists(options.inputFile)):
  optionParser.error("input file not found")

# open input and output files
inputFileContent = open(options.inputFile, 'r')
outputFileContent = open(options.outputFile, 'w')

# initialize array that will contain all hex values
hexArray = []

# parse and add all hex values to hexArray
for line in inputFileContent:
  if not line:
    continue
  hexLine = ["{:02x}".format(ord(c)) for c in line]
  hexArray.extend(hexLine)

# iterate through hexArray and print out lineMaxSize items on each line
for i in xrange(0, len(hexArray)):
  if i != 0:
    if i % lineMaxSize == 0:
      outputFileContent.write('\n')
    else:
      outputFileContent.write(' ')
  outputFileContent.write("0x%s" % hex(int(hexArray[i], 16)).upper()[2:].zfill(2))
  if i+1 != len(hexArray):
    outputFileContent.write(',')

# close input and output files
inputFileContent.close()
outputFileContent.close()
