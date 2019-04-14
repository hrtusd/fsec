#include "pch.h"

void fsec_encode(std::string filename) {
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
	
	
	int* norm = fsec::normalize(freqs, sum);
	
	#if DEBUG_PRINT
	printf_s("Normalized frequencies\n");
	for (int i = 0; i < fsec::symbol_count; i++)
	{
		printf_s("%d: %d\n", i, norm[i]);
	}
	#endif

	//int* spread = fsec::spread(norm);

	int* encoding_table = fsec::build_encoding_table(norm);
	fsec::decoding_entry* decoding_table = fsec::build_decoding_table(norm);

	#if DEBUG_PRINT
	fsec::print_tables();
	#endif
	
	int state = fsec::L;

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

	bs.write_decoding_table(decoding_table);
	bs.write(state);
	bs.write(sum);
	bs.write(fsec::L);

	bs.end();

	#if DEBUG_PRINT
	printf_s("End state %d\n", state);
	printf_s("End sum %d\n", sum);
	#endif
}

void fsec_decode(std::string filename) {
	int state;
	unsigned int sum;

	fsec::decoding_entry* decoding_table;

	// temp
	filename = filename + ".anst";

	printf_s("Rading state and decoding table - ");
	fsec::TimeMeasure* t = new fsec::TimeMeasure;
	t->Start();

	fsec::bitstream bs;
	bs.in2(&filename[0u], sum, state, decoding_table);

	t->End();
	t->Print();

	std::ofstream ofs;
	ofs.open(filename + "decode.txt", std::ios::binary | std::ios::trunc);

	for (int i = 0; i < sum; i++)
	{
		unsigned char symbol = fsec::decode(bs, state, decoding_table);
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
	if (argc == 2) {
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
	}
	else
	{
		printf_s("No file input.\n");
		return 0;
	}

	fsec::TimeMeasure* t = new fsec::TimeMeasure;

	#if _DEBUG
	
	t->Start();
	printf_s("\nEncoding ...\n");
	fsec_encode(filename);
	t->End();
	t->Print();

	t->Start();
	printf_s("\nDecoding ...\n");
	fsec_decode(filename);
	t->End();
	t->Print();
	

	printf_s("\nDone ...\n");

	return 0;
	#endif // DEBUG

	switch (mode)
	{
	case -1:
		return 0;
		break;
	case 0:
		printf_s("\nDecoding ...\n");

		t->Start();

		fsec_decode(filename);

		t->End();
		t->Print();

		printf_s("\nDone ...\n");
		break;
	case 1:
		printf_s("\nEncoding ...\n");

		t->Start();

		fsec_encode(filename);

		t->End();
		t->Print();

		printf_s("\nDone ...\n");
		break;
	default:
		break;
	}

	delete t;
	return 0;
}

