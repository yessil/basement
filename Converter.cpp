#include "Converter.h"
#include "BaseFrame.h"

Converter::Converter(void)
{
	argv[0] = "dummy";
	argv[1] = "-alpha";
	argv[2] = "0.97000003";
	argv[3] = "-frate";
	argv[4] = "100";
	argv[5] = "-input_endian";
	argv[6] = "little";
	argv[7] = "-lowerf";
	argv[8] = "133.33334";
	argv[9] = "-mach_endian";
	argv[10] = "little";
	argv[11] = "-raw";
	argv[12] = "yes";
	argv[13] = "-ncep";
	argv[14] = "13";
	argv[15] = "-nchans";
	argv[16] = "1";
	argv[17] = "-nfft";
	argv[18] = "512";
	argv[19] = "-nfilt";
	argv[20] = "40";
	argv[21] = "-o";
	argv[22] = "feat\\recorded.mfc";
	argv[23] = "-samprate";
	argv[24] = "16000";
	argv[25] = "-seed";
	argv[26] = "-1";
	argv[27] = "-upperf";
	argv[28] = "6855.4976";
	argv[29] = "-warp_type";
	argv[30] = "inverse_linear";
	argv[31] = "-whichchan";
	argv[32] = "1";
	argv[33] = "-wlen";
	argv[34] = "0.025625";
	argv[35] = "-i";
	argv[36] = "wav\\recorded000.raw";
}

Converter::~Converter(void)
{
}

extern "C" int convert(int32 argc, char **argv);
#if !wxUSE_EXCEPTIONS
    #error "This sample only works with wxUSE_EXCEPTIONS == 1"
#endif // !wxUSE_EXCEPTIONS


	int Converter::Convert(wxString wfile){

		char buf[FILENAME_LENGHT];//TODO
		strcpy(buf, (char*)wfile.char_str());
		argv[36] = buf;
		try {
			convert(37, argv);
			return 0;
			//TODO
		}
		catch (...)
		{
			return -1;
		}


}