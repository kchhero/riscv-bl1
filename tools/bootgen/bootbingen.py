import binascii
import sys
import os

ORG_GPT_FILE_NAME = "sdboot_gpt.bin"
ORG_DOS_FILE_NAME = "sdboot_dos.bin"
ZERO_PAD_FILE_NAME = "zeropad.bin"
GEN_FILE_NAME = "sdboot.bin"
NSIH_BL1_TXT_FILE_NAME = "nsih-bl1.txt"
NSIH_BBL_TXT_FILE_NAME = "nsih-bbl.txt"
NSIH_BL1_BIN_FILE_NAME = "nsih-bl1.bin"
NSIH_BBL_BIN_FILE_NAME = "nsih-bbl.bin"
VECTOR_BIN_FILE_NAME = "vector.bin"


def nsihgen(txtFileName, binFileName):
    nsihFilePath = txtFileName

    genFile = open(binFileName, 'wb')
    temp = []
    with open(nsihFilePath, 'rt') as data:
        for line in data:
            if len(line) <= 7 or (line[0] == '/' and line[1] == '/'):
                continue
            temp.append((line.split(' '))[0].strip().lower())

    for i in temp:
        if (len(i) != 8):
            print("ERROR")
            break

        temp2 = []
        # i : 01 23 45 67
        temp2.append(i[6])
        temp2.append(i[7])
        temp2.append(i[4])
        temp2.append(i[5])
        temp2.append(i[2])
        temp2.append(i[3])
        temp2.append(i[0])
        temp2.append(i[1])

        # print "".join(temp2)

        genFile.write(binascii.unhexlify("".join(temp2)))

    genFile.close()


def sdboot_gpt_headercut(filename):
    genFile = open(filename, 'wb')
    with open(ORG_GPT_FILE_NAME, 'rb') as data:
        genFile.write(data.read(0x43f))  # 1088)) #0~0x43f

    genFile.close()
    padAppend(0x43ff - 0x43f + 1, filename)  # 512byte * 34sector = 17408

    # with open('sdboot.bin', 'rb+') as filehandle:
    #     filehandle.seek(-1, os.SEEK_END)
    #     filehandle.truncate()


def sdboot_dos_headercut(filename):
    genFile = open(filename, 'wb')
    with open(ORG_DOS_FILE_NAME, 'rb') as data:
        genFile.write(data.read(512))  # 0~0x1ff
    genFile.close()


def padAppend(padSize, filename):
    genFile = open(filename, 'ab')

    with open(ZERO_PAD_FILE_NAME, 'rb') as data:
        genFile.write(data.read(padSize))  # 0~0x1f0 + 16byte

    genFile.close()


def vector_bin_size_fitting(filename):
    tempBinSize = os.stat(filename).st_size
    padSize = 1024*4 - tempBinSize

    genFile = open(filename, 'ab')
    print("vector bin padSize = %d" % padSize)

    with open(ZERO_PAD_FILE_NAME, 'rb') as data:
        genFile.write(data.read(padSize))  # 0~0x1f0 + 16byte

    genFile.close()


def main(binType, binPath):
    if binType == "gpt":
        sdboot_gpt_headercut(binPath+GEN_FILE_NAME)
    elif binType == "dos":
        sdboot_dos_headercut(binPath+GEN_FILE_NAME)
    else:
        print('Usage: python bootbingen.py \"gpt\"')
        print('Usage: python bootbingen.py \"dos\"')

    nsihgen(NSIH_BL1_TXT_FILE_NAME, binPath+NSIH_BL1_BIN_FILE_NAME)
    nsihgen(NSIH_BBL_TXT_FILE_NAME, binPath+NSIH_BBL_BIN_FILE_NAME)

    vector_bin_size_fitting(binPath+VECTOR_BIN_FILE_NAME)


if __name__ == "__main__":
    try:
        # profile.run('main()')
        main(sys.argv[1], sys.argv[2]+'/')
    finally:
        pass
