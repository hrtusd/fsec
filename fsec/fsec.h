#ifndef FSEC_H
#define FSEC_H

#define DEBUG_PRINT 0
#define INFO_PRINT 0
#define FILE_BLOCK_SIZE 1 << 12

namespace fsec
{
	/// <summary>
	/// Alphabet
	/// </summary>
	static int symbol_count = 256;

	/// <summary>
	/// Decoding entry
	/// </summary>
	struct decoding_entry
	{
		int symbol;
		int nb_bits;
		int next_state;
	};

	/// <summary>
	/// Symbol entry
	/// </summary>
	struct symbol_entry
	{
		int k;
		int nb;
		int start;
		int next;
	};

	/// <summary>
	/// Simple file bit I/O
	/// </summary>
	struct bitstream
	{
		std::fstream file;
		unsigned int buffer;
		int bitcount;


		/* v2 vars */
		unsigned long long bufferInternal;
        std::vector<unsigned char> bufferVec;
		int bufferSize = FILE_BLOCK_SIZE;
        int bufferRead;
		int position;

		void out(char* filename) {
			buffer = 0;
			bitcount = 0;
			file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
		}

		void out2(char* filename) {
			buffer = 0;
			bitcount = 0;
			file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
		}

		void write(unsigned int val, int nb)
		{
			buffer <<= nb;
			buffer |= val;
			bitcount += nb;
			while (bitcount > 7)
			{
				unsigned char tmp;
				tmp = (unsigned char)(buffer >> (bitcount - 8));

				#if DEBUG_PRINT
				printf_s("write %d\n", tmp);
				#endif // DEBUG_PRINT
				
				file.put(tmp);
				bitcount -= 8;
			}
		}

		void write2(unsigned int val, int nb)
		{
			buffer <<= nb;
			buffer |= val;
			bitcount += nb;
			while (bitcount > 7)
			{
				unsigned char tmp;
				tmp = (unsigned char)(buffer >> (bitcount - 8));

#if DEBUG_PRINT
				printf_s("write %d\n", tmp);
#endif // DEBUG_PRINT

				file.put(tmp);
				bitcount -= 8;
			}
		}

		void write(unsigned int val)
		{
			file.write(reinterpret_cast<const char *>(&val), sizeof(val));
		}

		int write_decoding_table(fsec::decoding_entry* decoding_table, int L) {
            int pos = file.tellg();

			for (int i = 0; i < L; i++)
			{
				const char* c1 = reinterpret_cast<const char *>(&(decoding_table[i].symbol));
				const char* c2 = reinterpret_cast<const char *>(&(decoding_table[i].nb_bits));
				const char* c3 = reinterpret_cast<const char *>(&(decoding_table[i].next_state));

				file.write(reinterpret_cast<const char *>(&(decoding_table[i].symbol)), sizeof((decoding_table[i].symbol)));
				file.write(reinterpret_cast<const char *>(&(decoding_table[i].nb_bits)), sizeof((decoding_table[i].nb_bits)));
				file.write(reinterpret_cast<const char *>(&(decoding_table[i].next_state)), sizeof((decoding_table[i].next_state)));
			}

            return pos;
		}

		int flush()
		{
			unsigned char tmp = (unsigned char)buffer;
			unsigned char bitchar = (unsigned char)bitcount;

			file.put(tmp);
			file.put(bitchar);

			#if DEBUG_PRINT
			printf_s("Flush %d\n", tmp);
			printf_s("Bitcount %d\n", bitcount);
			#endif // DEBUG_PRINT

			return bitcount;
		}

		int flush2()
		{
			unsigned char tmp = (unsigned char)buffer;
			unsigned char bitchar = (unsigned char)bitcount;

			file.put(tmp);
			file.put(bitchar);

#if DEBUG_PRINT
			printf_s("Flush %d\n", tmp);
			printf_s("Bitcount %d\n", bitcount);
#endif // DEBUG_PRINT

			return bitcount;
		}

		void in(char* filename, unsigned int &sum, int &state, fsec::decoding_entry* &decoding_table) {
			file.open(filename, std::ios::in | std::ios::binary);

			int len;
			file.seekg(-12, std::ios::end);
			file.read((char*)&state, sizeof(unsigned int));
			file.read((char*)&sum, sizeof(unsigned int));
			file.read((char*)&len, sizeof(unsigned int));

			int offset = 0 - 14 - len * 3 * sizeof(unsigned int);
			file.seekg(offset, std::ios::end);

			unsigned char buff = file.get();
			unsigned char bit = file.get();

			decoding_table = new fsec::decoding_entry[len];
			for (int i = 0; i < len; i++)
			{
				file.read(reinterpret_cast<char *>(&(decoding_table[i].symbol)), sizeof(unsigned int));
				file.read(reinterpret_cast<char *>(&(decoding_table[i].nb_bits)), sizeof(unsigned int));
				file.read(reinterpret_cast<char *>(&(decoding_table[i].next_state)), sizeof(unsigned int));
			}

			bitcount = bit;
			buffer = buff;

			#if DEBUG_PRINT
			printf_s("state %d\n", state);
			printf_s("sum %d\n", sum);
			printf_s("start %d\n", buffer);
			printf_s("bitc %d\n", bitcount);
			#endif // DEBUG_PRINT

			file.seekg(offset - 1, std::ios::end);
		}

		void in2(char* filename, unsigned int &sum, int &state, fsec::decoding_entry* &decoding_table) {			
			file.open(filename, std::ios::in | std::ios::binary);

			int len;
			file.seekg(-12, std::ios::end);
			file.read((char*)&state, sizeof(unsigned int));
			file.read((char*)&sum, sizeof(unsigned int));
			file.read((char*)&len, sizeof(unsigned int));

			int offset = 0 - 14 - len * 3 * sizeof(unsigned int);
			file.seekg(offset, std::ios::end);

			unsigned char buff = file.get();
			unsigned char bit = file.get();

			decoding_table = new fsec::decoding_entry[len];
			for (int i = 0; i < len; i++)
			{
				file.read(reinterpret_cast<char *>(&(decoding_table[i].symbol)), sizeof(unsigned int));
				file.read(reinterpret_cast<char *>(&(decoding_table[i].nb_bits)), sizeof(unsigned int));
				file.read(reinterpret_cast<char *>(&(decoding_table[i].next_state)), sizeof(unsigned int));
			}

			bitcount = bit;
			bufferInternal = buff;
            bufferRead = 0;
            bufferVec.resize(bufferSize);

			#if DEBUG_PRINT
			printf_s("state %d\n", state);
			printf_s("sum %d\n", sum);
			printf_s("start %d\n", bufferInternal);
			printf_s("bitc %d\n", bitcount);
			#endif // DEBUG_PRINT

			file.seekg(offset, std::ios::end);
			position = file.tellg();
		}

        unsigned int read(int nb_bits)
		{
			while (bitcount < nb_bits)
			{
				unsigned char r = file.get();

				#if DEBUG_PRINT
				printf_s("read %d\n", r);
				#endif // DEBUG_PRINT				

				std::streampos pos = file.tellg();
				pos -= 2;
				if (pos < 0)
				{
					pos = 0;
				}
				file.seekg(pos);

				buffer |= r << bitcount;
				bitcount += 8;
			}

			unsigned int ret = buffer & ((1 << nb_bits) - 1);

			buffer >>= nb_bits;
			bitcount -= nb_bits;

			return ret;
		}

        unsigned int read2(int nb_bits)
        {
            while (bitcount < nb_bits/* && position > 0*/)
            {
                //v2
                if (bufferRead == 0) {
                    if (position < bufferSize) {
                        bufferSize = position;
                        position = 0;
                    }
                    else {
                        position -= bufferSize;
                    }
                    file.seekg(position, std::ios::beg);


                    bufferVec.resize(bufferSize);
                    file.read(reinterpret_cast<char *>(&bufferVec[0]), bufferSize);
                    bufferRead = bufferSize;
                }

                int refreshRate = 2;
                int refreshed = 0;
                while (refreshed < refreshRate && bufferRead > 0)
                {
                    int rotate = bitcount + refreshed++ * 8;
                    unsigned char newByte = bufferVec[--bufferRead];

                    bufferInternal |= newByte << rotate;

                    #if DEBUG_PRINT
                    printf_s("read %d\n", newByte);
                    #endif // DEBUG_PRINT
                }

                bitcount += 8 * refreshed;
			}

			unsigned int ret = bufferInternal & ((1 << nb_bits) - 1);

			bufferInternal >>= nb_bits;
			bitcount -= nb_bits;

			return ret;
		}

		void end() {
			file.close();
		}
	};

	unsigned long long countf(char* filename, int* &freqs);
	int* normalize(int* &freqs, unsigned long long true_sum, int L);
    double entropy(int* &freqs, unsigned long long true_sum);
	int* spread(int* symbols, int L);
	decoding_entry* build_decoding_table(int* normalized, int L, int R);
	symbol_entry* build_symbol_table(int* freqs, int L);
	int* build_encoding_table(int* normalized, int L);

	unsigned char decode(bitstream &input, int &state, decoding_entry* decoding_table, int L);
	void encode(int symbol, int &state, bitstream &output);

	void print_tables(int L);

    void fsec_encode(std::string filename, int L, int R);
    void fsec_decode(std::string filename, int L);
}

#endif //FSEC_H