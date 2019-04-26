#include "pch.h"

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
    printf_s("Entropy: %f\n", entropy);
	
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
	fsec::print_tables();
	#endif
	
	int state = L;

	fsec::bitstream bs;
	std::string f = std::string(filename) + ".anst";
	bs.out2(&f[0u]);


	std::ifstream ifs;
	ifs.open(filename, std::ios::binary | std::ios::ate);

	int blockSize = 1 << 14;
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

    printf_s("Bytes source: %d\n", sum);
    printf_s("Bytes written: %d\n", bytesWritten);
    printf_s("Bytes min: %f\n", entropy * sum / 8);
    printf_s("Ratio: %f\n", sum / (double)bytesWritten);
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
    filename = filename + ".anst";
    #endif // _DEBUG


	printf_s("Rading state and decoding table - ");
    fsec::TimePoint start = fsec::timer_timepoint();

	fsec::bitstream bs;
	bs.in2(&filename[0u], sum, state, decoding_table);

    fsec::TimePoint end = fsec::timer_timepoint();
    fsec::timer_print(start, end);

	std::ofstream ofs;
    // Double name of original file
    std::string outputFileName = filename.substr(0, filename.length() - 5) + filename.substr(0, filename.length() - 5);
	ofs.open(outputFileName, std::ios::binary | std::ios::trunc);

	for (int i = 0; i < sum; i++)
	{
		unsigned char symbol = fsec::decode(bs, state, decoding_table, L);
		ofs.write(reinterpret_cast<char *>(&symbol), 1);
	}
}

int main(int argc, char** argv)
{
	#if _DEBUG
		argc = 2;
	#endif // DEBUG

	const char* filename;
	int mode = -1;

    int R = 10;
    int L = 1 << R;

    switch (argc)
    {
    case 3:
        R = std::atoi(argv[2]);
        L = 1 << R;
    case 2:
        #if _DEBUG
        filename = "plrabn12.txt";// "plrabn12.txt";
        #else
        filename = argv[1];
        #endif // DEBUG

        printf_s("File: %s\n", filename);
        if (std::string(filename).find(".anst") != std::string::npos) {
            mode = 0; // Decoding
        }
        else {
            mode = 1; // Encoding
        }
        break;
    default:
        printf_s("No file input.\n");
        return 0;
        break;
    }

	#if _DEBUG
	
    fsec::TimePoint startEnc = fsec::timer_timepoint();
	printf_s("\nEncoding ...\n");
	fsec_encode(filename);
    fsec::TimePoint endEnc = fsec::timer_timepoint();
    fsec::timer_print(startEnc, endEnc);

    fsec::TimePoint startDec = fsec::timer_timepoint();
	printf_s("\nDecoding ...\n");
	fsec_decode(filename);
    fsec::TimePoint endDec = fsec::timer_timepoint();
    fsec::timer_print(startDec, endDec);
	

	printf_s("\nDone ...\n");

	return 0;
	#endif // DEBUG

    if (mode != 0 && mode != 1) return 0;

    fsec::TimePoint startR = fsec::timer_timepoint();

	switch (mode)
	{
	case 0:
		printf_s("\nDecoding ...\n");
		fsec_decode(filename, L);
		break;
	case 1:
		printf_s("\nEncoding ...\n");
		fsec_encode(filename, L, R);
		break;
	}

    fsec::TimePoint endR = fsec::timer_timepoint();
    fsec::timer_print(startR, endR);
    printf_s("\nDone ...\n");


	return 0;
}

