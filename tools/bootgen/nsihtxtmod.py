import os
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove

BL1_FILE_NAME = "bl1.bin"
BBL_FILE_NAME = "bbl.bin"
NSIH_BL1_TXT_FILE_NAME = "nsih-bl1.txt"
NSIH_BBL_TXT_FILE_NAME = "nsih-bbl.txt"

ZERO_PAD_FILE_NAME = "zeropad.bin"


def binpadAppend(filename):
    tempBl1Size = os.stat(filename).st_size
    temp = (tempBl1Size + 512 - 1)/512
    padSize = (temp * 512) - tempBl1Size

    genFile = open(filename, 'ab')
    # print(filename + " padSize = %d"%padSize)

    with open(ZERO_PAD_FILE_NAME, 'rb') as data:
        genFile.write(data.read(padSize))  # 0~0x1f0 + 16byte

    genFile.close()


def getNSIHSize(binFileName):
    temp = os.stat(binFileName).st_size
    # tempInt = int(temp)
    # tempAlign = 512*int((tempInt + 512 -1)/512)
    temp = "%08x" % temp
    return temp


def modNSIHTXT(txtFileName, binFileName):
    tempFile1, tempFile2 = mkstemp()
    with fdopen(tempFile1, 'wt') as new_file:
        with open(txtFileName) as org_file:
            for line in org_file:
                if "0x040" in line and "Load Size" in line:
                    new_file.write(str(getNSIHSize(binFileName)) + "   " +
                                   "".join(line.split('   ')[1:]))
                else:
                    new_file.write(line)

    remove(txtFileName)
    move(tempFile2, txtFileName)


def main():
    binpadAppend(BL1_FILE_NAME)
    binpadAppend(BBL_FILE_NAME)
    modNSIHTXT(NSIH_BL1_TXT_FILE_NAME, BL1_FILE_NAME)
    modNSIHTXT(NSIH_BBL_TXT_FILE_NAME, BBL_FILE_NAME)


if __name__ == "__main__":
    try:
        main()
    finally:
        pass
