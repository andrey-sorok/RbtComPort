#include "BotControl.h"

int main()
{
	
	wstring wport = L"COM5";

	auto BControl = make_unique<BotControl>(wport.c_str());

	string dir = "com.txt";
	BControl->fileCMD(dir);

	//BControl->StopCMD();
	
	cout << BControl->getDistance() << endl;

//	BControl->turn(-90);

//	BControl->turn(90);

	//BControl->move(5);

	//BControl->move(-5);

	//system("pause");
}