import os
import sys
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove

BL1_FILE_NAME = "bl1.bin"
BBL_FILE_NAME = "bbl.bin"
DTB_FILE_NAME = ""
NSIH_BL1_TXT_FILE_NAME = "nsih-bl1.txt"
NSIH_BBL_TXT_FILE_NAME = "nsih-bbl.txt"

ZERO_PAD_FILE_NAME = "zeropad.bin"


def binpadAppend(filename):
    tempBl1Size = os.stat(filename).st_size
    temp = (tempBl1Size + 512 - 1) / 512
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


def modNSIHTXT(txtFileName, binFileName, key1, key2):
    tempFile1, tempFile2 = mkstemp()
    with fdopen(tempFile1, 'wt') as new_file:
        with open(txtFileName) as org_file:
            for line in org_file:
                if key1 in line and key2 in line:
                    new_file.write(str(getNSIHSize(binFileName)) + "   " +
                                   "".join(line.split('   ')[1:]))
                else:
                    new_file.write(line)

    remove(txtFileName)
    move(tempFile2, txtFileName)


def main(boardName, binPath):
    binpadAppend(binPath + BL1_FILE_NAME)
    binpadAppend(binPath + BBL_FILE_NAME)
    DTB_FILE_NAME = "swallow-" + boardName + ".dtb"
    binpadAppend(binPath + "swallow-" + boardName + ".dtb")
    modNSIHTXT(NSIH_BL1_TXT_FILE_NAME, binPath + BL1_FILE_NAME,
               "0x040", "Load Size")
    modNSIHTXT(NSIH_BBL_TXT_FILE_NAME, binPath + BBL_FILE_NAME,
               "0x040", "Load Size")
    modNSIHTXT(NSIH_BL1_TXT_FILE_NAME, binPath + DTB_FILE_NAME,
               "0x100", "DTB Binary Size")


if __name__ == "__main__":
    try:
        print(sys.argv[1])
        print(sys.argv[2])
        main(sys.argv[1], sys.argv[2] + '/')
    finally:
        pass
