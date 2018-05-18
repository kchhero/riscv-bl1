import binascii
import os
import sys

ORG_GPT_FILE_NAME = "sdboot_gpt.bin"
ORG_DOS_FILE_NAME = "sdboot_dos.bin"
ZERO_PAD_FILE_NAME = "zeropad.bin"
GEN_FILE_NAME = "sdboot.bin"
NSIH_BL1_TXT_FILE_NAME = "nsih-bl1.txt"
NSIH_BBL_TXT_FILE_NAME = "nsih-bbl.txt"
NSIH_BL1_BIN_FILE_NAME = "nsih-bl1.bin"
NSIH_BBL_BIN_FILE_NAME = "nsih-bbl.bin"

def nsihgen(txtFileName, binFileName):
    nsihFilePath = txtFileName

    genFile = file(binFileName,'wb')
    temp = []
    
    with open(nsihFilePath, 'rt') as data :
        for line in data :
            if len(line) <= 7 or (line[0]=='/' and line[1]=='/') :
                continue
            
            temp.append((line.split(' '))[0].strip().lower())

    for i in temp :
        if (len(i) != 8) :
            print "ERROR"
            break

        temp2 = []
        #i : 01 23 45 67
        temp2.append(i[6])
        temp2.append(i[7])
        temp2.append(i[4])
        temp2.append(i[5])
        temp2.append(i[2])
        temp2.append(i[3])
        temp2.append(i[0])
        temp2.append(i[1])

        #print "".join(temp2)

        genFile.write(binascii.unhexlify("".join(temp2)))

    genFile.close()


def sdboot_gpt_headercut():
    genFile = file("sdboot.bin",'wb')
    
    with open(ORG_GPT_FILE_NAME, 'rb') as data :        
        genFile.write(data.read(0x43f))#1088)) #0~0x43f

    genFile.close()
    
    padAppend(0x43ff - 0x43f + 1)  #512byte * 34sector = 17408

    # with open('sdboot.bin', 'rb+') as filehandle:
    #     filehandle.seek(-1, os.SEEK_END)
    #     filehandle.truncate()
    
def sdboot_dos_headercut():
    genFile = file("sdboot.bin",'wb')
    
    with open(ORG_DOS_FILE_NAME, 'rb') as data :        
        genFile.write(data.read(512)) #0~0x1ff
                                    
    genFile.close()

def padAppend(padSize):
    genFile = file("sdboot.bin",'ab')

    with open(ZERO_PAD_FILE_NAME, 'rb') as data :
        genFile.write(data.read(padSize)) #0~0x1f0 + 16byte

    genFile.close()
        
def main(binType):
    if binType == "gpt":
        sdboot_gpt_headercut()
    elif binType == "dos":
        sdboot_dos_headercut()
    else :
        print "Usage: python bootbingen.py \"gpt\""
        print "Usage: python bootbingen.py \"dos\""

    nsihgen(NSIH_BL1_TXT_FILE_NAME, NSIH_BL1_BIN_FILE_NAME)
    nsihgen(NSIH_BBL_TXT_FILE_NAME, NSIH_BBL_BIN_FILE_NAME)

if __name__ == "__main__":
    try : 
        #profile.run('main()')
        main(sys.argv[1])
    finally : 
        pass