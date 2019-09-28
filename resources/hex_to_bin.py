#!/usr/bin/python3

import sys


def main():
    if len(sys.argv) < 2:
        print("No input file")
        return

    filename = sys.argv[1]

    hex_string = ""
    with open(filename) as f:
        while True:
            c = f.read(1)
            if not c:
                print("End of file")
                break

            if c not in [" ", "\n"]:
                hex_string = hex_string + c

    b = bytes.fromhex(hex_string)
    binary_file = filename[:-3] + "bin"
    open(binary_file, 'wb').write(b)


if __name__ == "__main__":
    # execute only if run as a script
    main()
