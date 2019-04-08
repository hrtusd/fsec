#pragma once

namespace fsec
{
	/// <summary>
	/// Bit range
	/// </summary>
	static int R = 5;
	/// <summary>
	/// Max state (2^R)
	/// </summary>
	static int L = 1 << R;
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

		void out(char* filename) {
			buffer = 0;
			bitcount = 0;
			file.open(filename, std::ios::out | std::ios::trunc);
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
				printf_s("write %d\n ", tmp);
				file.put(tmp);
				bitcount -= 8;
			}
		}

		void write(unsigned int val)
		{
			file.write(reinterpret_cast<const char *>(&val), sizeof(val));
		}

		void write_decoding_table(fsec::decoding_entry* decoding_table) {
			/*for (int i = 0; i < ANST::L; i++)
			{
				printf_s(" index: %d\t symbol: %d\t nbBits: %d\t nextState: %d\n", i,
					dtable[i].Symbol,
					dtable[i].nbBits,
					dtable[i].NextState);
			}*/

			for (int i = 0; i < fsec::L; i++)
			{
				const char* c1 = reinterpret_cast<const char *>(&(decoding_table[i].symbol));
				auto s1 = sizeof((decoding_table[i].symbol));
				const char* c2 = reinterpret_cast<const char *>(&(decoding_table[i].nb_bits));
				auto s2 = sizeof((decoding_table[i].nb_bits));
				const char* c3 = reinterpret_cast<const char *>(&(decoding_table[i].next_state));
				auto s3 = sizeof((decoding_table[i].next_state));


				file.write(reinterpret_cast<const char *>(&(decoding_table[i].symbol)), sizeof((decoding_table[i].symbol)));
				file.write(reinterpret_cast<const char *>(&(decoding_table[i].nb_bits)), sizeof((decoding_table[i].nb_bits)));
				file.write(reinterpret_cast<const char *>(&(decoding_table[i].next_state)), sizeof((decoding_table[i].next_state)));
			}
		}

		int flush()
		{
			unsigned char tmp = (unsigned char)(buffer);
			file.put(tmp);
			file.put(bitcount);

			printf_s("flush %d\n ", tmp);
			printf_s("bitc %d\n ", bitcount);

			return bitcount;
		}

		void in2(char* filename, unsigned int &sum, int &state, fsec::decoding_entry* &decoding_table) {
			file.open(filename, std::ios::in);


			int len;
			file.seekg(-12, std::ios::end);
			file.read((char*)&state, sizeof(unsigned int));
			file.read((char*)&sum, sizeof(unsigned int));
			file.read((char*)&len, sizeof(unsigned int));

			int offset = 0 - 15 - len * 3 * sizeof(unsigned int);
			file.seekg(offset, std::ios::end);

			//file.seekg(-11, std::ios::end);
			unsigned char buff = file.get();
			//file.seekg(-10, std::ios::end);
			unsigned char bit = file.get();

			decoding_table = new fsec::decoding_entry[len];
			for (int i = 0; i < len; i++)
			{
				file.read((char*)&(decoding_table[i].symbol), sizeof(unsigned int));
				file.read((char*)&(decoding_table[i].nb_bits), sizeof(unsigned int));
				file.read((char*)&(decoding_table[i].next_state), sizeof(unsigned int));
			}

			/*for (int i = 0; i < len; i++)
			{
				printf_s(" index: %d\t symbol: %d\t nbBits: %d\t nextState: %d\n", i,
					dtable[i].Symbol,
					dtable[i].nbBits,
					dtable[i].NextState);
			}*/

			bitcount = bit;
			buffer = buff;

			printf_s("state %d\n", state);
			printf_s("sum %d\n", sum);
			printf_s("start %d\n", buffer);
			printf_s("bitc %d\n", bitcount);

			file.seekg(0 - 16 - len, std::ios::end);
		}

		unsigned int read(int nb_bits)
		{
			while (bitcount < nb_bits)
			{
				unsigned char r = file.get();
				printf_s("read %d\n", r);

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

		void end() {
			file.close();
		}
	};

	int count(int* input, int inputLen, int* &freqs);
	unsigned long long countf(char* filename, int* &freqs);
	int* normalize(int* &freqs, unsigned long long true_sum);
	int* spread(int* symbols);
	decoding_entry* build_decoding_table(int* normalized);
	symbol_entry* build_symbol_table(int* freqs);
	int* build_encoding_table(int* normalized);

	int decode(bitstream &input, int &state, decoding_entry* decoding_table);
	void encode(int symbol, int &state, bitstream &output);

	void print_tables();
}