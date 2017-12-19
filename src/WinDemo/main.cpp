//#include "vld.h"
#include "VoipManager.h"
#include "LocalTest.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char * argv[])
{
	// LocalTest();
	CVoipManager voipManager;
	voipManager.LoginServer("103.202.141.87", 12345, 1, 0);
	Sleep(300);
	voipManager.StartVoipTransmit();
	voipManager.AddNode(0);
	Sleep(2000);
	// getchar();
	voipManager.EndVoipTransmit();
	// Sleep(100);
	// system("pause");
	return 0;
}