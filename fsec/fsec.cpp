#include "pch.h"

int main(int argc, char** argv)
{
	#if _DEBUG
		argc = 2;
	#endif // DEBUG

	const char* filename;
	int mode = -1;

    int R = 12;
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
        if (std::string(filename).find(".fsec") != std::string::npos) {
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
	fsec_encode(filename, L, R);
    fsec::TimePoint endEnc = fsec::timer_timepoint();
    fsec::timer_print(startEnc, endEnc);

    fsec::TimePoint startDec = fsec::timer_timepoint();
	printf_s("\nDecoding ...\n");
	fsec_decode(filename, L);
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
		fsec::fsec_decode(filename, L);
		break;
	case 1:
		printf_s("\nEncoding ...\n");
		fsec::fsec_encode(filename, L, R);
		break;
	}

    fsec::TimePoint endR = fsec::timer_timepoint();
    fsec::timer_print(startR, endR);
    printf_s("\nDone ...\n");

	return 0;
}

