#include "MP3_AAC_controller.h"

using namespace AUDIO_PLAY;

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("usage ./play <filename> \n");
		return -1;
	}
	startPlayAudio(argv[1]);
	return 0;
}