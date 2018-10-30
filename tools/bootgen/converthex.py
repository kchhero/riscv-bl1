import binascii
import sys


def converthex(binPath):
    srcFilePath = binPath+"sdboot.bin"
    hexFileName = binPath+"sdboot.hex"

    genFile = open(hexFileName, 'wb')
    # temp = []

    with open(srcFilePath, 'rb') as data:
        while 1:
            byte_s = data.read(1)
            if not byte_s:
                break

            # print binascii.hexlify(byte_s) + ' ',
            genFile.write(binascii.hexlify(byte_s)+"\n")

    genFile.close()


def main(binPath):
    converthex(binPath)


if __name__ == "__main__":
    try:
        main(sys.argv[1]+'/')
    finally:
        pass
