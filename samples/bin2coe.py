import sys
import struct

template = '''memory_initialization_radix=16;
memory_initialization_vector={};
radix=16;'''

def bin2coe(fname):
   with open(fname, 'rb') as f:
      content = f.read()
   unpacked_content = ["{:08x}".format(struct.unpack("<I", content[i:i+4])[0]) for i in range(0,len(content),4)]
   print(template.format(','.join(unpacked_content)))

if __name__ == '__main__':
   if len(sys.argv) <= 1:
      print("python bin2coe.py <bin file>")

   bin2coe(sys.argv[1])
      
