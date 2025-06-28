def convert_to_header(bin_filename, out_header):
    with open(bin_filename, 'rb') as f:
        data = f.read()

    with open(out_header, 'w') as h:
        h.write('#pragma once\n\n')
        h.write('const unsigned char NNUE_DATA[] = {\n')

        for i in range(0, len(data)):
            if i % 12 == 0:
                h.write('    ')
            h.write(f'0x{data[i]:02x}')
            if i != len(data) - 1:
                h.write(', ')
            if (i + 1) % 12 == 0:
                h.write('\n')

        if len(data) % 12 != 0:
            h.write('\n')

        h.write('};\n')
        h.write(f'const size_t NNUE_SIZE = {len(data)};\n')

# Example usage
if __name__ == "__main__":
    convert_to_header("nnue.bin", "nnue_data.h")