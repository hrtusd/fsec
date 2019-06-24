#include "pch.h"

namespace fsec
{
	int* encoding_table;
	symbol_entry* symbol_table;
	decoding_entry* decoding_table;
	int* symbol_spread;

	unsigned long long countf(char* filename, int* &freqs)
	{
		#if INFO_PRINT
		printf_s("Counting symbol frequencies - start\n");
		TimePoint start = timer_timepoint();
		#endif

		int size = symbol_count;
		unsigned long long true_sum = 0ULL;

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

			ifs.read(reinterpret_cast<char *>(&buffer[0]), blockSize);

			for (int i = 0; i < blockSize; i++)
			{
				freqs[buffer[i]]++;
			}

			pos += blockSize;
		}

		ifs.close();		

		// reduce total size from 256
		while (freqs[size - 1] == 0 && size > 0)
		{
			size--;
		}

		symbol_count = size;

		#if INFO_PRINT
		printf_s("Counting symbol frequencies - ");
        TimePoint end = timer_timepoint();
        timer_print(start, end);

		printf_s("Alphabet internal %d\n", symbol_count);
#endif
		return true_sum;
	}

	int* normalize(int* &freqs, unsigned long long true_sum, int L)
	{
#if INFO_PRINT
		printf_s("Normalizing symbol frequencies - start\n");
        TimePoint start = timer_timepoint();
#endif

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

#if INFO_PRINT
		printf_s("Normalizing symbol frequencies - ");
        TimePoint end = timer_timepoint();
        timer_print(start, end);
#endif

		return normalized;
	}

    double entropy(int* &freqs, unsigned long long true_sum)
    {
#if INFO_PRINT
        printf_s("Calculating entropy - start\n");
        TimePoint start = timer_timepoint();
#endif

        double e = 0.0;

        for (int i = 0; i < symbol_count; i++)
        {
            double prob = (double)freqs[i] / true_sum;
            if (prob == 0) continue;
            double log = log2(prob);
            double mul = prob * log;
            e += mul;
        }

#if INFO_PRINT
		printf_s("Calculating entropy - ");
        TimePoint end = timer_timepoint();
        timer_print(start, end);
#endif

        return -e;
    }

	// Spread symbols across table
	int* spread(int* symbols, int L)
	{
#if INFO_PRINT
		printf_s("Spreading symbol occurencies - start\n");
        TimePoint start = timer_timepoint();
#endif

		int* symbol = new int[L];

		int idx = 0;
		int step = (int)(5.0 / 8 * L + 3);

		//int cnt = 0;
		for (int i = 0; i < symbol_count; i++)
		{
			for (int j = 0; j < symbols[i]; j++)
			{
				symbol[idx] = i;
				idx = (idx + step) % L;
				//symbol[cnt++] = i;
			}
		}

#if INFO_PRINT
		printf_s("Spreading symbol occurencies - ");
		TimePoint end = timer_timepoint();
        timer_print(start, end);
#endif

		return symbol;
	}

	decoding_entry* build_decoding_table(int* normalized, int L, int R)
	{
#if INFO_PRINT
		printf_s("Building decoding table - start\n");
        TimePoint start = timer_timepoint();
#endif

		decoding_table = new decoding_entry[L];

		symbol_spread = spread(normalized, L);

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

#if INFO_PRINT
		printf_s("Building decoding table - ");
		TimePoint end = timer_timepoint();
        timer_print(start, end);
#endif

		return decoding_table;
	}

	symbol_entry* build_symbol_table(int* freqs, int L)
	{
#if INFO_PRINT
		printf_s("Building symbol table - start\n");
        TimePoint start = timer_timepoint();
#endif

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

#if INFO_PRINT
		printf_s("Building symbol table - ");
        TimePoint end = timer_timepoint();
        timer_print(start, end);
#endif

		return symbol_table;
	}

	int* build_encoding_table(int* normalized, int L)
	{
#if INFO_PRINT
		printf_s("Building encoding table - start\n");
        TimePoint start = timer_timepoint();
#endif

		encoding_table = new int[L];

		symbol_spread = spread(normalized, L);

		symbol_table = build_symbol_table(normalized, L);

		for (int x = L; x < 2 * L; x++)
		{
			int s = symbol_spread[x - L];
			int start = symbol_table[s].start;
			int next = symbol_table[s].next++;
			encoding_table[start + next] = x;
		}

#if INFO_PRINT
		printf_s("Building encoding table - ");
        TimePoint end = timer_timepoint();
        timer_print(start, end);
#endif

		return encoding_table;
	}

	void print_tables(int L)
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

	unsigned char decode(bitstream &input, int &state, decoding_entry* decoding_table, int L)
	{
		decoding_entry de = decoding_table[state - L];

		int nb_bits = de.nb_bits;

		int shift = input.read2(nb_bits);

		int new_state = de.next_state;
		new_state <<= nb_bits;
		new_state |= shift;

		#if DEBUG_PRINT
		printf_s("symbol: %d\t state: %d -> %d\t bits: %d\t nb: %d\n", (int)de.symbol, state, new_state, shift, nb_bits);
		#endif

		state = new_state;

		return (unsigned char)de.symbol;
	}

	void encode(int symbol, int &state, bitstream &output)
	{
		int nb_bits = symbol_table[symbol].nb;
		if (state >= symbol_table[symbol].k) nb_bits++;

		int tmp = state & ((1 << nb_bits) - 1);

		int start = symbol_table[symbol].start;
		int shift = (state >> nb_bits);
		int new_state = encoding_table[start + shift];

		#if DEBUG_PRINT
		printf_s("symbol: %d\t state: %d -> %d\t bits: %d\t nb: %d\n", symbol, state, new_state, tmp, nb_bits);
		#endif

		output.write2(state & ((1 << nb_bits) - 1), nb_bits);

		state = new_state;
	}

    void fsec_encode(std::string filename, int L, int R) {
        int* freqs = new int[fsec::symbol_count];
        for (int i = 0; i < fsec::symbol_count; i++)
        {
            freqs[i] = 0;
        }

        unsigned long long sum = fsec::countf(&filename[0u], freqs);
        printf_s("File lenght (bytes): %d\n", sum);

#if DEBUG_PRINT
        printf_s("Standard frequencies\n");
        for (int i = 0; i < fsec::symbol_count; i++)
        {
            printf_s("%d: %d\n", i, freqs[i]);
        }
#endif

        double entropy = fsec::entropy(freqs, sum);

#if INFO_PRINT
        printf_s("Entropy: %f\n", entropy);
#endif

        int* norm = fsec::normalize(freqs, sum, L);

#if DEBUG_PRINT
        printf_s("Normalized frequencies\n");
        for (int i = 0; i < fsec::symbol_count; i++)
        {
            printf_s("%d: %d\n", i, norm[i]);
        }
#endif

        //int* spread = fsec::spread(norm);

        int* encoding_table = fsec::build_encoding_table(norm, L);
        fsec::decoding_entry* decoding_table = fsec::build_decoding_table(norm, L, R);

#if DEBUG_PRINT
        fsec::print_tables(L);
#endif

        int state = L;

        fsec::bitstream bs;
        std::string f = std::string(filename) + ".fsec";
        bs.out2(&f[0u]);


        std::ifstream ifs;
        ifs.open(filename, std::ios::binary | std::ios::ate);

        int blockSize = FILE_BLOCK_SIZE;
        std::vector<unsigned char> buffer;
        int pos = (int)(ifs.tellg());

        while (pos > 0)
        {
            if (pos - blockSize < 0) {
                blockSize = pos;
                pos = 0;
            }
            else {
                pos -= blockSize;
            }

            ifs.seekg(pos, std::ios::beg);
            buffer.resize(blockSize);

            ifs.read(reinterpret_cast<char*>(&buffer[0]), blockSize);

            for (int i = blockSize - 1; i >= 0; i--)
            {
                fsec::encode(buffer[i], state, bs);
            }
        }

        int bitcount = bs.flush();

        int bytesWritten = bs.write_decoding_table(decoding_table, L);

#if INFO_PRINT
        printf_s("Bytes source: %d\n", sum);
        printf_s("Bytes written: %d\n", bytesWritten);
        printf_s("Bytes min: %f\n", entropy * sum / 8);
        printf_s("Ratio: %f\n", (double)bytesWritten / sum);
#endif
        bs.write(state);
        bs.write(sum);
        bs.write(L);

        bs.end();

#if DEBUG_PRINT
        printf_s("End state %d\n", state);
        printf_s("End sum %d\n", sum);
#endif
    }

    void fsec_decode(std::string filename, int L) {
        int state;
        unsigned int sum;

        fsec::decoding_entry* decoding_table;

#if _DEBUG
        filename = filename + ".fsec";
#endif // _DEBUG

#if INFO_PRINT
        printf_s("Reading state and decoding table - start\n");
        fsec::TimePoint start = fsec::timer_timepoint();
#endif

        fsec::bitstream bs;
        bs.in2(&filename[0u], sum, state, decoding_table);

#if INFO_PRINT
		printf_s("Reading state and decoding table - ");
        fsec::TimePoint end = fsec::timer_timepoint();
        fsec::timer_print(start, end);
#endif

        std::ofstream ofs;
        std::string outputFileName = filename + ".decoded";
        ofs.open(outputFileName, std::ios::binary | std::ios::trunc);

        int blockSize = FILE_BLOCK_SIZE;
        std::vector<unsigned char> buffer;
        int cnt = 0;
        buffer.resize(blockSize);

#if INFO_PRINT
		printf_s("Decoding - start\n");
		fsec::TimePoint start2 = fsec::timer_timepoint();
#endif

        for (int i = 0; i < sum; i++)
        {
            unsigned char symbol = fsec::decode(bs, state, decoding_table, L);
            buffer[cnt++] = symbol;

            if (cnt == blockSize) {
                ofs.write(reinterpret_cast<char*>(&buffer[0]), cnt);
                cnt = 0;
            }
        }

        ofs.write(reinterpret_cast<char*>(&buffer[0]), cnt);
        ofs.close();

#if INFO_PRINT
		printf_s("Decoding - ");
		fsec::TimePoint end2 = fsec::timer_timepoint();
		fsec::timer_print(start2, end2);

		printf_s("File decoded to: %s\n", &outputFileName[0]);
#endif
    }
}
