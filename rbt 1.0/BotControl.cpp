#include "BotControl.h"

BotControl::BotControl(LPCWSTR sPortName)
{
	connectPort(sPortName);
}

void BotControl::connectPort(LPCWSTR sPortName)
{
	hSerial = CreateFile(sPortName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if(hSerial==INVALID_HANDLE_VALUE)
	{
		if(GetLastError()==ERROR_FILE_NOT_FOUND)
		{
			cout << "serial port does not exist.\n";
		}
		cout << "some other error occurred.\n";
	}


	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);

	if (!GetCommState(hSerial, &dcbSerialParams))
	{
		cout << "getting state error\n";
	}

	dcbSerialParams.BaudRate = CBR_115200;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.fBinary = TRUE;

	dcbSerialParams.fOutxCtsFlow = FALSE;
    dcbSerialParams.fOutxDsrFlow = FALSE;
	dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
	dcbSerialParams.fDsrSensitivity = FALSE;
    dcbSerialParams.fNull = FALSE;
    dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
    dcbSerialParams.fAbortOnError = FALSE;

	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	if(SetCommState(hSerial, &dcbSerialParams) == 0)
	{
		cout << "error setting serial port state\n";
	}

	COMMTIMEOUTS comtimeouts; 
        comtimeouts.ReadIntervalTimeout = 0xFFFFFFFF;
        comtimeouts.ReadTotalTimeoutMultiplier = 0;
        comtimeouts.ReadTotalTimeoutConstant =  0;
        comtimeouts.WriteTotalTimeoutMultiplier = 0;
        comtimeouts.WriteTotalTimeoutConstant = 1000;

	SetCommTimeouts(hSerial, &comtimeouts);

	PurgeComm(hSerial, PURGE_RXCLEAR);
	PurgeComm(hSerial, PURGE_TXCLEAR);

	//SetupComm(hSerial,2000,2000);

	//PurgeComm(hSerial,PURGE_RXCLEAR);

	int mode = 5;

	setMode(mode);
/*
	{
		ostringstream st;
		st << mode;
		string tmp = st.str();
	
		cout << "Mode is " + tmp << endl;
	}
	*/
}

bool BotControl::move(int nSm)
{
	bool rtn = 0;
	const int K = 150; //mode 5 K = 150 - 1 sm
	nSm = nSm * K;
	
	ostringstream st;
	string cmd;

	if (nSm >= 0)
	{
		st << nSm;
		string s = st.str();
		string command = "forward 500 500 " + s + " ";

		 cmd = getMovementCMD(command);
	}
	else
	{
		nSm = nSm * -1;
		st << nSm;
		string s = st.str();
		string command = "backward 250 250 " + s + " ";

		cmd = getMovementCMD(command);
	}

	string read = "";
	read = sendCMD(cmd);

	if (read != "err")
	{
		rtn = 1;
	}
	
	return rtn;
}

bool BotControl::turn(int nAngle)
{
	bool rtn = 0;
	
	const double left45 = 26;
	const double right45 = 26;

	const double left90 = 26;
	const double right90 = 26;

	string s = "";

	if (nAngle > 0)
	{	
		if (nAngle > 45)
		{
			s = setTurnStep(nAngle, left90);
		}
		else
		{
			if (nAngle <= 45)
			{
				s = setTurnStep(nAngle, left45);
			}
		}

		string command = "left 200 200 " + s + " ";

		string cmd = getMovementCMD(command);
		string read = sendCMD(cmd);

		if (read != "err")
		{
			rtn = 1;
		}
	}
	else
	{
		if (nAngle < 0)
		{	
			nAngle = nAngle * -1;
			if (nAngle > 45)
			{
				s = setTurnStep(nAngle, right90);
			}
			else
			{
				if (nAngle <= 45)
				{
					s = setTurnStep(nAngle, right45);
				}
			}
			
			
			string command = "right 200 200 " + s + " ";

			string cmd = getMovementCMD(command);
			string read = sendCMD(cmd);

			if (read != "err")
			{
				rtn = 1;
			}
		}
	}	

	return rtn;
}

int BotControl::getDistance()
{
	string cmd = getSensorCMD("3");

	string read = "";
	read = sendCMD(cmd);
	
	int rtn = -1;

	if (read != "err")
	{
		read = read.substr(0, read.find(","));			
		rtn	= atoi(read.c_str());
	}
	
	return rtn;
}

bool BotControl::StopCMD()
{
	string cmd = "";
	cmd = "{\"cmd\":\"stop\"}";

	string read = "";
	read = sendCMD(cmd);
	
	bool rtn = 0;

	if (read != "err")
	{
		rtn	= 1;
	}
	
	return rtn;

}

void BotControl::fileCMD(string Dir)
{
	string line = "";
	ifstream inF(Dir);

	vector<string> com;

	if(inF.is_open())
	{
		while (getline(inF, line))
		{
			com.push_back(line);
		}
	}

	int comNum = 0;
	for (int i = 1; i < (int)com.size(); ++i)
	{
		++comNum;
		int pos = (int)com[i].find_first_of(" ");

		string tmp = com[i].substr(0, pos);

		com[i] = com[i].erase(0, pos+1);

		if(tmp == "move")
		{
			double nSm = 0.0;

			nSm = atof(com[i].c_str());

			move((int)nSm);

			cout << tmp << ": " << nSm << endl;
		}

		if(tmp =="rotate")
		{
			int nSm = 0;

			nSm = atoi(com[i].c_str());

			turn(nSm);

			cout << tmp << ": " << nSm << endl;
		}
		//cout << comNum << endl;	
	}
}

BotControl::~BotControl()
{
	
}

wstring BotControl::getPortConnect(map<wstring,wstring> ports)
{
	map<wstring,wstring>::iterator it;
	
	for(it = ports.begin(); it != ports.end(); ++it)
	{
		int pos = (int)it->second.find(L"BthModem0");
		if(pos != -1)
		{
			wstring rtn = it->first;
			return rtn;
		}
	}

	return L"Error";
}

wstring BotControl::findeCOMPort()
{
	map<wstring,wstring> portMap;
	
	int r = 0;
	HKEY hkey = NULL;

	r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM\\"), 0, KEY_READ, &hkey);
	if (r != ERROR_SUCCESS) 
	{
		wstring rtn = L"Err";
		return rtn;
	}
  
	unsigned long CountValues = 0, MaxValueNameLen = 0, MaxValueLen = 0;

	RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &CountValues, &MaxValueNameLen, &MaxValueLen, NULL, NULL);
	++MaxValueNameLen;

	TCHAR *bufferName = NULL, *bufferData = NULL;
	bufferName = (TCHAR*)malloc(MaxValueNameLen * sizeof(TCHAR));
	if (!bufferName)
	{
		RegCloseKey(hkey);
		wstring rtn = L"Err";

		return rtn;
	}
	bufferData = (TCHAR*)malloc((MaxValueLen + 1)*sizeof(TCHAR));
	if (!bufferData) 
	{ 
		free(bufferName); 
		RegCloseKey(hkey);

		wstring rtn = L"Err";
		return rtn;
	}
  
	unsigned long NameLen, type, DataLen;

	for (unsigned int i = 0; i < CountValues; i++)
	{
		NameLen = MaxValueNameLen;
		DataLen = MaxValueLen;
		r = RegEnumValue(hkey, i, bufferName, &NameLen, NULL, &type, (LPBYTE)bufferData, &DataLen);
		if ((r != ERROR_SUCCESS) || (type != REG_SZ))
		{
			continue;    
		}

		portMap[bufferData] = bufferName;
	}

	free(bufferName);
	free(bufferData);

	RegCloseKey(hkey);

	wstring rtn = L""; // getPortConnect(portMap);

	return rtn;
}

bool BotControl:: setMode(int mode)
{
	string cmd = getModeCMD(mode);

	string read = "";
	read = sendCMD(cmd);
	
	bool rtn = 0;

	if (read != "err")
	{
		rtn = 1;
	}
	
	return rtn;
}

string BotControl::setTurnStep(int nAngle, const double Konst)
{
	string rtn = "";
	double step = (double)nAngle;

	step = step*Konst;
	ostringstream st;
	st << (int)step;

	rtn = st.str();

	return rtn;
}

string BotControl::getMovementCMD(string com)
{
	string command = "{\"cmd\":\"\",\"params\":{\"spd\":0,\"acc\":0,\"dis\":0}}";

	rapidjson::Document doc;
	bool isParse = (doc.Parse(command.c_str())).HasParseError();
	
	if ((doc.Parse(command.c_str())).HasParseError())
	{
		cout << "Document Parse Error" << endl;
		return "1";
	}
	assert(doc.IsObject());
//___	
	string str= "";
	int pos = (int)com.find_first_of(" ");
	str = com.substr(0 ,pos);
//___
	com = com.substr(pos + 1 , com.length());
	
	rapidjson::Value::MemberIterator& sshCMD = doc.FindMember("cmd");
	sshCMD->value.SetString(str.c_str(), doc.GetAllocator());

	for (rapidjson::Value::MemberIterator& m = doc["params"].MemberBegin(); m != doc["params"].MemberEnd(); ++m) 
	{
		pos = (int)com.find_first_of(" ");
		str = com.substr(0 ,pos);
		com = com.substr(pos+1 , com.length());
	
		m->value.SetInt(atoi(str.c_str()));
	}
	
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	
	string theJSON = buffer.GetString();

	return  theJSON;

}

string BotControl::getSensorCMD(string com)
{
	string command = "{\"cmd\":\"sensor\",\"count\":\"0\"}";

	rapidjson::Document doc;
	bool isParse = (doc.Parse(command.c_str())).HasParseError();
	if ((doc.Parse(command.c_str())).HasParseError())
	{
		cout << "Document Parse Error" << endl;
		return "1";
	}
	assert(doc.IsObject());
				
	int count = 0;
	count = atoi(com.c_str());

	rapidjson::Value::MemberIterator& sshSensor = doc.FindMember("count");
	sshSensor->value.SetInt(count);
				
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	string theJSON = buffer.GetString();
			
	return theJSON;
}

string BotControl::getModeCMD (int mode)
{
	string command = "";
	switch(mode)
	{
		case 1:
			{
				command = "{\"cmd\":\"mode\",\"MS1\":0,\"MS2\":0,\"MS3\":0}";
				break;
			}	
		case 2:
			{
				command = "{\"cmd\":\"mode\",\"MS1\":1,\"MS2\":0,\"MS3\":0}";
				break;
			}
		case 3:
			{
				command = "{\"cmd\":\"mode\",\"MS1\":0,\"MS2\":1,\"MS3\":0}";
				break;
			}
		case 4:
			{
				command = "{\"cmd\":\"mode\",\"MS1\":1,\"MS2\":1,\"MS3\":0}";
				break;
			}
		case 5:
			{
				command = "{\"cmd\":\"mode\",\"MS1\":1,\"MS2\":1,\"MS3\":1}";
				break;
			}
		}

	return command;

}

string BotControl::readCOM()
{

	COMSTAT comstat; 
	DWORD btr = 0, temp = 0, mask = 0, signal = 0;

	SetCommMask(hSerial, EV_RXCHAR);
	
	bool isExit = false;
	string rtnBuf = "";

	while (btr == 0)
	{ 
		ClearCommError(hSerial, &temp, &comstat);
		btr = comstat.cbInQue;
		
		if (btr > 0)
		{
			while (isExit != true)
			{
				char buff[64];
				DWORD dwBytesRead;
				BOOL iRewad = ReadFile(hSerial, buff, sizeof(buff), &dwBytesRead, NULL);
									
				if (dwBytesRead != 0)
				{
					string tmp = "";	
					tmp = tmp + buff;
					rtnBuf = rtnBuf + tmp.substr(0, dwBytesRead);

					if (rtnBuf[rtnBuf.length()-1] == '\n')
					{
						isExit = true;
						btr = 1;
					}
					else
					{
						ClearCommError(hSerial, &temp, &comstat);
						btr = comstat.cbInQue;
					}	
				}				
			}
		}
	}
			
	return rtnBuf;
}

string BotControl::sendCMD(string cmd)
{
	char command[60]; 
	strcpy_s(command, cmd.c_str());
		
	DWORD dwSize = sizeof(command);
	DWORD dwBytesWritten;
	BOOL iWrite = WriteFile(hSerial, command, dwSize, &dwBytesWritten, 0);

//	char buff[60];
//	DWORD dwBytesRead;
	string str = readCOM();

	return str;	
}