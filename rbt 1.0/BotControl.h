#include "stdinc.h"

#pragma once
class BotControl
{
public:

	BotControl(LPCWSTR sPortName);

	bool move(int nSm);
	bool turn(int nAngle);
	int getDistance();
	bool StopCMD();
	
	void fileCMD(string Dir);

	~BotControl();
	
private:

	HANDLE hSerial;

	void connectPort(LPCWSTR sPortName);
	
	wstring getPortConnect(map<wstring,wstring> ports);	
	wstring findeCOMPort();

	bool setMode(int mode);
	
	string setTurnStep(int nAngle, const double Konst);

	string getMovementCMD(string com);
	string getSensorCMD(string command);
	string getModeCMD (int mode);

	string readCOM();
	string sendCMD(string cmd);
};

