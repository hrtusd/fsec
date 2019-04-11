#include "pch.h"

namespace fsec
{
	int* encoding_table;
	symbol_entry* symbol_table;
	decoding_entry* decoding_table;
	int* symbol_spread;

	unsigned long long countf(char* filename, int* &freqs)
	{
		int size = symbol_count;
		unsigned long long true_sum = 0ULL;

		printf_s("read\n");
		
		fsec::TimeMeasure* t = new fsec::TimeMeasure;
		t->Start();

		std::fstream ifs;
		ifs.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
		true_sum = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		int blockSize = 1 << 12;
		std::vector<unsigned char> buffer(blockSize);
		int pos = 0;

		while (pos < true_sum)
		{
			if (pos + blockSize > true_sum) {
				blockSize = true_sum - pos;
				buffer.resize(blockSize);
			}

			t->Start();

			ifs.read(reinterpret_cast<char *>(&buffer[0]), blockSize);

			t->End();
			t->Print();

			t->Start();

			for (int i = 0; i < blockSize; i++)
			{
				freqs[buffer[i]]++;
			}

			t->End();
			t->Print();

			pos += blockSize;
		}

		t->End();
		t->Print();

		ifs.close();		

		// reduce total size from 256
		while (freqs[size - 1] == 0 && size > 0)
		{
			size--;
		}

		symbol_count = size;
		printf_s("alphabet internal %d\n", symbol_count);
		return true_sum;
	}

	int* normalize(int* &freqs, unsigned long long true_sum)
	{
		int* normalized = new int[symbol_count];
		int sum = 0;

		// up/down scale
		double scale = L / (double)true_sum;

		for (int i = 0; i < symbol_count; i++)
		{
			if (freqs[i] == 0) normalized[i] = 0;
			else
			{
				// scale every symbol count
				int f = (int)round(scale * freqs[i]);
				normalized[i] = (f > 0) ? f : 1;
				sum += normalized[i];
			}
		}

		// count sum difference
		int error = L - sum;

		while (error != 0)
		{
			int sign = error > 0 ? 1 : -1;
			double min = ULONG_MAX;
			int correct_idx = 0;
			for (int i = 0; i < symbol_count; i++)
			{
				double correction = freqs[i] * log2l((double)normalized[i] / (normalized[i] + sign));
				if (correction < min)
				{
					correct_idx = i;
				}
			}
			normalized[correct_idx] += sign;
			error -= sign;
		}

		return normalized;
	}

	// Spread symbols across table
	int* spread(int* symbols)
	{
		int* symbol = new int[L];

		int idx = 0;
		int step = (int)(5.0 / 8 * L + 3);

		for (int i = 0; i < symbol_count; i++)
		{
			for (int j = 0; j < symbols[i]; j++)
			{
				symbol[idx] = i;
				idx = (idx + step) % L;
			}
		}

		return symbol;
	}

	decoding_entry* build_decoding_table(int* normalized)
	{
		decoding_table = new decoding_entry[L];

		symbol_spread = spread(normalized);

		int* next = new int[symbol_count];
		for (int i = 0; i < symbol_count; i++)
		{
			next[i] = normalized[i];
		}

		for (int X = 0; X < L; X++)
		{
			decoding_table[X].symbol = symbol_spread[X];
			int x = next[decoding_table[X].symbol]++;
			decoding_table[X].next_state = x;
			decoding_table[X].nb_bits = (int)(R - floor(log2(x)));
		}

		return decoding_table;
	}

	symbol_entry* build_symbol_table(int* freqs)
	{
		symbol_entry* symbol_table = new symbol_entry[symbol_count];

		for (int i = 0; i < symbol_count; i++)
		{
			int fl = (int)floor(log2l((double)L / freqs[i]));
			if (freqs[i] == 0) fl = 1;

			symbol_table[i].nb = fl;
			symbol_table[i].k = (freqs[i] + freqs[i]) << symbol_table[i].nb;
			symbol_table[i].start = -freqs[i];

			for (int j = 0; j < i; j++)
				symbol_table[i].start += freqs[j];

			symbol_table[i].next = freqs[i];
		}

		return symbol_table;
	}

	int* build_encoding_table(int* normalized)
	{
		encoding_table = new int[L];

		symbol_spread = spread(normalized);

		symbol_table = build_symbol_table(normalized);

		for (int x = L; x < 2 * L; x++)
		{
			int s = symbol_spread[x - L];
			int start = symbol_table[s].start;
			int next = symbol_table[s].next++;
			encoding_table[start + next] = x;
		}

		return encoding_table;
	}

	void print_tables()
	{
		printf_s("Alphabet: %d\n", symbol_count);
		printf_s("L: %d\n", L);
		printf_s("StateC: %d\n", L);

		printf_s("DTable entries: %d\n", L);
		for (int i = 0; i < L; i++)
		{
			printf_s(" index: %d\t symbol: %d\t nbBits: %d\t nextState: %d\n", i,
				decoding_table[i].symbol,
				decoding_table[i].nb_bits,
				decoding_table[i].next_state);
		}

		printf_s("ESTable entries: %d\n", symbol_count);
		for (int i = 0; i < symbol_count; i++)
		{
			printf_s(" index: %d\t maxState: %d\t nbBits: %d\t threshold: %d\t offset: %d\n", i,
				symbol_table[i].next,
				symbol_table[i].nb,
				symbol_table[i].k,
				symbol_table[i].start);

		}

		printf_s("CTable entries: %d\n", L);
		for (int i = 0; i < L; i++)
		{
			printf_s(" index: %d\t state: %d\n", i, encoding_table[i]);
		}
	}


	int decode(bitstream &input, int &state, decoding_entry* decoding_table)
	{
		decoding_entry de = decoding_table[state - L];

		int nb_bits = de.nb_bits;

		int shift = input.read(nb_bits);

		int new_state = de.next_state;
		new_state <<= nb_bits;
		new_state |= shift;

		//printf_s("symbol: %d\t state: %d -> %d\t bits: %d\t nb: %d\n", (int)de.symbol, state, new_state, shift, nb_bits);

		state = new_state;

		return de.symbol;
	}

	void encode(int symbol, int &state, bitstream &output)
	{
		int nb_bits = symbol_table[symbol].nb;
		if (state >= symbol_table[symbol].k) nb_bits++;

		//output.WriteBits(state & ((1 << nbBits) - 1), nbBits);
		output.write(state & ((1 << nb_bits) - 1), nb_bits);

		int tmp = state & ((1 << nb_bits) - 1);

		int start = symbol_table[symbol].start;
		int shift = (state >> nb_bits);
		int new_state = encoding_table[start + shift];

		//printf_s("symbol: %d\t state: %d -> %d\t bits: %d\t nb: %d\n", symbol, state, new_state, tmp, nb_bits);

		state = new_state;
	}
}
