#include "pch.h"

void fsec_encode(std::string filename) {
	int* freqs = new int[fsec::symbol_count];
	for (int i = 0; i < fsec::symbol_count; i++)
	{
		freqs[i] = 0;
	}

	unsigned long long sum = fsec::countf(&filename[0u], freqs);
	printf_s("sum %d\n", sum);
	printf_s("normalized\n");
	printf_s("alphabet ext %d\n", fsec::symbol_count);
	//for (int i = 0; i < fsec::symbol_count; i++)
	//{
	//	printf_s("%d: %d\n", i, freqs[i]);
	//}

	int* norm = fsec::normalize(freqs, sum);
	/*printf_s("normalized\n");
	for (int i = 0; i < fsec::symbol_count; i++)
	{
		printf_s("%d: %d\n", i, norm[i]);
	}*/

	int* spread = fsec::spread(norm);

	int* encoding_table = fsec::build_encoding_table(norm);
	fsec::decoding_entry* decoding_table = fsec::build_decoding_table(norm);

	fsec::print_tables();

	int state = fsec::L;


	fsec::bitstream bs;
	std::string f = std::string(filename) + ".anst";
	bs.out(&f[0u]);


	std::ifstream ifs;
	ifs.open(filename, std::ios::in);
	ifs.seekg(-1, std::ios::end);
	for (int i = sum; i > 0; i--)
	{
		fsec::encode(ifs.get(), state, bs);

		std::streampos pos = ifs.tellg();
		pos -= 2;
		if (pos < 0)
		{
			pos = 0;
		}
		ifs.seekg(pos);
	}

	int bitcount = bs.flush();

	bs.write_decoding_table(decoding_table);
	bs.write(state);
	bs.write(sum);
	bs.write(fsec::L);

	bs.end();
	printf_s("state %d\n", state);
	printf_s("sum %d\n", sum);
}

void fsec_decode(std::string filename) {
	int state;
	unsigned int sum;

	fsec::decoding_entry* decoding_table;


	// temp
	filename = filename + ".anst";

	fsec::bitstream bs;
	bs.in2(&filename[0u], sum, state, decoding_table);

	for (int i = 0; i < fsec::L; i++)
	{
		printf_s(" index: %d\t symbol: %d\t nbBits: %d\t nextState: %d\n", i,
			decoding_table[i].symbol,
			decoding_table[i].nb_bits,
			decoding_table[i].next_state);
	}

	std::ofstream ofs;
	ofs.open(filename + "decode.txt", std::ios::out | std::ios::trunc);

	for (int i = 0; i < sum; i++)
	{
		ofs.put(fsec::decode(bs, state, decoding_table));
	}
}

void fsec_encode_mem(std::string filename) {
	int* freqs = new int[256];
	for (int i = 0; i < 256; i++)
	{
		freqs[i] = 0;
	}

	unsigned long long sum = fsec::countf(&filename[0u], freqs);
	int* norm = fsec::normalize(freqs, sum);


	int* spread = fsec::spread(norm);

	int* encoding_table = fsec::build_encoding_table(norm);
	fsec::decoding_entry* decoding_table = fsec::build_decoding_table(norm);

	int state = fsec::L;


	fsec::bitstream bs;
	std::string f = std::string(filename) + ".anst";
	bs.out(&f[0u]);


	std::ifstream ifs;
	ifs.open(filename, std::ios::in);
	ifs.seekg(-1, std::ios::end);
	for (int i = sum; i > 0; i--)
	{
		fsec::encode(ifs.get(), state, bs);

		std::streampos pos = ifs.tellg();
		pos -= 2;
		if (pos < 0)
		{
			pos = 0;
		}
		ifs.seekg(pos);
	}

	int bitcount = bs.flush();

	bs.write_decoding_table(decoding_table);
	bs.write(state);
	bs.write(sum);
	bs.write(fsec::L);

	bs.end();
	printf_s("state %d\n", state);
	printf_s("sum %d\n", sum);
}

void fsec_decode_mem(std::string filename) {
	int state;
	unsigned int sum;

	fsec::decoding_entry* decoding_table;

	fsec::bitstream bs;
	bs.in2(&filename[0u], sum, state, decoding_table);

	std::ofstream ofs;
	ofs.open(filename + "decode.txt", std::ios::out | std::ios::trunc);

	for (int i = 0; i < sum; i++)
	{
		ofs.put(fsec::decode(bs, state, decoding_table));
	}
}

void test() {
	int inpSize = 5;
	int* input = new int[inpSize];

	for (int i = 0; i < inpSize; i++)
	{
		input[i] = rand() % 255;
	}

	int* freqs = new int[256];
	memset(freqs, 0, 256);

	int sum = fsec::count(input, inpSize, freqs);
	int* norm = fsec::normalize(freqs, sum);


	int* spread = fsec::spread(norm);

	int* encoding_table = fsec::build_encoding_table(norm);
	fsec::decoding_entry* decoding_table = fsec::build_decoding_table(norm);

	fsec::print_tables();

	return;

	int state = fsec::L;
	unsigned char* comp = new unsigned char[50];
	unsigned char* comp_start = comp;

	/*bit_output bout;
	bout.start(comp_start);

	int bout_final_bits;
	unsigned char* bout_final_ptr;


	int state_bits = 11;

	for (int i = inpSize - 1; i > -1; i--)
	{
		fsec::encode(input[i], state, bout);
	}

	bout.put(state, state_bits);

	bout_final_bits = bout.flush();
	bout_final_ptr = bout.ptr;


	bit_input bin;
	bin.start(bout_final_ptr, bout_final_bits);

	printf_s("\n");

	state = bin.get(state_bits);

	int* decoded = new int[inpSize];

	for (int i = 0; i < inpSize; i++)
	{
		decoded[i] = fsec::decode(bin, state);
		if (decoded[i] != input[i])
		{
			printf_s("err\n");
			return;
		}
	}*/

	printf_s("done..\n");
}

int main(int argc, char** argv)
{
	//test();

	//return 0;

	#if _DEBUG
		argc = 2;
	#endif // DEBUG

	const char* filename;
	int mode = -1;
	if (argc == 2) {
		#if _DEBUG
			filename = "tt.txt";
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

	printf_s("Encoding....\n");
	fsec_encode(filename);

	printf_s("Decoding....\n");
	fsec_decode(filename);

	return 0;

	switch (mode)
	{
	case -1:
		return 0;
		break;
	case 0:
		printf_s("Decoding....\n");
		fsec_decode(filename);
		break;
	case 1:
		printf_s("Encoding....\n");
		fsec_encode(filename);
		break;
	default:
		break;
	}

	return 0;
}

