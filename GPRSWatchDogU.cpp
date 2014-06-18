//---------------------------------------------------------------------------
#include "GPRSWatchDogU.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TWatchDog *WatchDog;
TWatchDogThread* WatchDogThread;

//�������� CallBack �������
VOID WINAPI RasDialFunc(
    UINT unMsg,	// type of event that has occurred
    RASCONNSTATE rasconnstate,	// connection state about to be entered
    DWORD dwError	// error that may have occurred
    );


char winPath[100]; //���������� � ������� �������� ���� � ��������� �����
char confFileName[15]="GPRSWDSet.conf"; //��� ������. �����
char confFile[115]="";//���������� � ������� �������� ���� � ��� ������ �����
char debugFileName[8]="DWD.log"; //��� ����������� �����
char debugFile[108]="";//���������� � ������� �������� ���� � ��� ����������� �����

//---------------------------------------------------------------------------
__fastcall TWatchDog::TWatchDog(TComponent* Owner)
	: TService(Owner)
{

}

TServiceController __fastcall TWatchDog::GetServiceController(void)
{
	return (TServiceController) ServiceController;
}

void __stdcall ServiceController(unsigned CtrlCode)
{
	WatchDog->Controller(CtrlCode);
}
//---------------------------------------------------------------------------
__fastcall TWatchDogThread::TWatchDogThread(bool CreateSuspended)
                           :TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
 void __fastcall TWatchDogThread::Execute()
 {
  AnsiString temp;//��� ���������� ��������
  char Buffer[1500]; //������
  UINT BufferPos; //��������� �� �����
  UINT Result; //���������� ��� �������� ����������� API �������
  bool Error; //���� ������
  char tempPath[200]; //���������� � ������� �������� ���� � Temp �����
  AnsiString backLogName; //����� ��� ��� ��� �����
  int  i;

  Stream = new TMemoryStream(); //������ �����
  WatchDogSettings = new TWatchDogSettings();
  WatchDogConnState =new TWatchDogConnState();
  WatchDogRasCon =new TWatchDogRasCon();
  RasDialParams = new TRasDialParams();
  Connected=false;
  ConnectError=false;
  Result=GetWindowsDirectory(winPath,sizeof(winPath));
  StrCat(winPath,"\\");//��������� �����������
  StrCat(confFile,winPath);//������ ���� � �������������� ������
  StrCat(confFile,confFileName);//��� ����� � ��������������� ������
  Stream->LoadFromFile(confFile); //����������� �� ����� ������ � �����
  Stream->Seek(0,soFromBeginning);
  Stream->Read(Buffer,Stream->Size);
  BufferPos=0;
  Error=false;

  //���� ������ ��������. ���������� PATH
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;
  //���� �������� ���������� PATH
  if (Buffer[BufferPos]=='P' && Buffer[BufferPos+1]=='A' &&
      Buffer[BufferPos+2]=='T' && Buffer[BufferPos+3]=='H' &&
      Buffer[BufferPos+4]=='=')
   {
    BufferPos+=5;
    WatchDogSettings->LogFile=Buffer[BufferPos];
    BufferPos++;
    while (Buffer[BufferPos]!='>')
     {
      WatchDogSettings->LogFile+=Buffer[BufferPos];
      BufferPos++;
     }
    WatchDogSettings->LogFile+="\GPRSWatchDog.log";
   }
  else
   Error=true; //���� ���� �� �� ���������� �� ������

 Result=GetTempPath(sizeof(tempPath),tempPath);

 if (Result!=0)
  {
   backLogName="\\GWD";
   backLogName=tempPath+backLogName;
   temp=DateToStr(Date());
   for (i=1;i<temp.Length();i++)
    if (temp[i]=='.') temp[i]='_';
   temp+="_";
   temp+=TimeToStr(Time());
   for (i=1;i<temp.Length();i++)
    if (temp[i]==':') temp[i]='_';
   backLogName+=temp;
   backLogName+=".log";
   CopyFile(WatchDogSettings->LogFile.c_str(),backLogName.c_str(),false);
  }


 //���� ������ �������� ���������� MAXROW
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;

  //���� ������� ���������� MAXROW
  if (Buffer[BufferPos]=='M' && Buffer[BufferPos+1]=='A' &&
      Buffer[BufferPos+2]=='X' && Buffer[BufferPos+3]=='R' &&
      Buffer[BufferPos+4]=='O' && Buffer[BufferPos+5]=='W' &&
      Buffer[BufferPos+6]=='=' && !Error)
   {
    BufferPos+=7;
    temp=Buffer[BufferPos];
    BufferPos++;
    while (Buffer[BufferPos]!='>')
     {
      temp+=Buffer[BufferPos];
      BufferPos++;
     }
    WatchDogSettings->LogMaxRow=StrToInt(temp);
   }
   else
    Error=true; //���� ���� �� �� ���������� �� ������

 //���� ������ �������� ���������� CONN
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;

  //���� ������� ���������� CONN
  if (Buffer[BufferPos]=='C' && Buffer[BufferPos+1]=='O' &&
      Buffer[BufferPos+2]=='N' && Buffer[BufferPos+3]=='N' &&
      Buffer[BufferPos+4]=='=' && !Error)
   {
    BufferPos+=5;
    temp=Buffer[BufferPos];
    BufferPos++;
    while (Buffer[BufferPos]!='>')
     {
      temp+=Buffer[BufferPos];
      BufferPos++;
     }
    if (StrToInt(temp)<10)
     WatchDogSettings->Connects=StrToInt(temp);
    else
     WatchDogSettings->Connects=10;
   }
  else
   Error=true; //���� ���� �� �� ���������� �� ������

 //���� �������� �������� ���������� CALLS
 while (Buffer[BufferPos]!='<')
  BufferPos++;
 BufferPos++;

 //���� ������� ���������� CONN
 if (Buffer[BufferPos]=='C' && Buffer[BufferPos+1]=='A' &&
     Buffer[BufferPos+2]=='L' && Buffer[BufferPos+3]=='L' &&
     Buffer[BufferPos+4]=='S' && Buffer[BufferPos+5]=='=' && !Error)
  {
   BufferPos+=6;
   temp=Buffer[BufferPos];
   BufferPos++;
   while (Buffer[BufferPos]!='>')
    {
     temp+=Buffer[BufferPos];
     BufferPos++;
    }
   WatchDogSettings->Calls=StrToInt(temp);
  }
  else
   Error=true; //���� ���� �� �� ���������� �� ������

 //���� ����� �������� ���������� RAC
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;

 //���� ������� ���������� RAC
 if (Buffer[BufferPos]=='R' && Buffer[BufferPos+1]=='A' &&
     Buffer[BufferPos+2]=='C' && Buffer[BufferPos+3]=='=' && !Error)
  {
   BufferPos+=4;
   temp=Buffer[BufferPos];
   BufferPos++;
   while (Buffer[BufferPos]!='>')
    {
     temp+=Buffer[BufferPos];
     BufferPos++;
    }
   WatchDogSettings->RAC=temp;
   WatchDogRasCon->RAC=temp;
  }
  else
   Error=true; //���� ���� �� �� ���������� �� ������

//���� ������ �������� ���������� SERVER
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;

 //���� ������� ���������� SERV
 if (Buffer[BufferPos]=='S' && Buffer[BufferPos+1]=='E' &&
     Buffer[BufferPos+2]=='R' && Buffer[BufferPos+3]=='V' &&
     Buffer[BufferPos+4]=='=' && !Error)
  {
   BufferPos+=5;
   temp=Buffer[BufferPos];
   BufferPos++;
   while (Buffer[BufferPos]!='>')
    {
     temp+=Buffer[BufferPos];
     BufferPos++;
    }
   WatchDogSettings->SERV=temp;
  }
  else
   Error=true; //���� ���� �� �� ���������� �� ������


 temp=DateToStr(Date());
 temp+=" ";
 temp+=TimeToStr(Time());

 if (!Error) //���� �� ���� ������
 {
  temp+="-��������� ������� �������! ������ �������!\r\n";
  Stream->Clear();
  AddToLogFile(temp);
 }
 else
  {
   temp+="-������������ �����! ������ ����� ����������!";
   Stream->Clear();
   AddToLogFile(temp);
   //Terminate();
  }


  while (!Terminated)
  {
   TimerOverflow();
   Idglobal::Sleep(60000);
  }

}

void __fastcall TWatchDog::ServiceStart(TService *Sender, bool &Started)
{
 WatchDogThread =new TWatchDogThread(false);
 Started=true;
}
//---------------------------------------------------------------------------

void __fastcall TWatchDog::ServiceContinue(TService *Sender,
      bool &Continued)
{
 WatchDogThread->Resume();
 Continued=true;
}
//---------------------------------------------------------------------------

void __fastcall TWatchDog::ServicePause(TService *Sender, bool &Paused)
{
 WatchDogThread->Suspend();
 Paused=true;
}
//---------------------------------------------------------------------------

void __fastcall TWatchDog::ServiceStop(TService *Sender, bool &Stopped)
{
 WatchDogThread->Terminate();
 Stopped=true;
}
//---------------------------------------------------------------------------
//������� ��������� �������� ���������� � � ������ ����� ���������� true
bool __fastcall TWatchDogThread::VerifyActiveConnection()
{
 UINT Result;
 int i;
 AnsiString tmpStr, tmpStr1;
  ///��������� �������� �����������
 WatchDogRasCon->rasConn[0].dwSize=sizeof (RASCONN);
 WatchDogRasCon->ConBufferSize=sizeof(WatchDogRasCon->rasConn)*10; //������������� ������ ������ �������� ������ ������� �����������
 Result=RasEnumConnections(&WatchDogRasCon->rasConn[0],&WatchDogRasCon->ConBufferSize,&WatchDogRasCon->Connections);
 if (Result==0 && WatchDogRasCon->Connections>0) //���� ���������� �������� �����������
  { //���������� �� � ���� ��������� � ������
   for (i=0;i<10;i++)
    {
     tmpStr=WatchDogRasCon->rasConn[i].szEntryName;
     tmpStr1=WatchDogSettings->RAC;

    if(tmpStr==tmpStr1)
     {
      WatchDogRasCon->hrasconn=WatchDogRasCon->rasConn[i].hrasconn;
     }
    }
   }
 if (WatchDogRasCon->hrasconn==NULL) return false;
 else
  {
   tmpStr=DateToStr(Date());
   tmpStr+=" ";
   tmpStr+=TimeToStr(Time());
   tmpStr+="-�����������:\"";
   tmpStr+=WatchDogSettings->RAC;
   tmpStr+="\" �������!\r\n";
   AddToLogFile( tmpStr);
   return true;
  }
 }


//---------------------------------------------------------------------------
//������� ������� �������� ������ � ������� � ���� ���� �� ��������
//���������� false, � � ����� ����� ���������� true
bool __fastcall TWatchDogThread::TestActiveConnection()
{
 bool   RetValue;
 AnsiString tStr;
 try
  {

   WatchDog->IdIcmpClient->ReceiveTimeout=3000;
   WatchDog->IdIcmpClient->Host=WatchDogSettings->SERV;
   WatchDog->IdIcmpClient->Ping();
   if (WatchDog->IdIcmpClient->ReplyStatus.ReplyStatusType!=rsTimeOut)
    {
     tStr=DateToStr(Date());
     tStr+=" ";
     tStr+=TimeToStr(Time());
     tStr+="- ��� ������ � �������: ";
     tStr+=WatchDogSettings->SERV;
     tStr+=" ���������� ������!\r\n";
     AddToLogFile( tStr);
     RetValue=true;
    }
   else
    {
     tStr=DateToStr(Date());
     tStr+=" ";
     tStr+=TimeToStr(Time());
     tStr+="- ��� ������ � �������: ";
     tStr+=WatchDogSettings->SERV;
     tStr+=" ���������� �� ������! �������� �������� ��������\r\n";
     AddToLogFile( tStr);
     RetValue=false;
    }
  }
  catch (Exception &exception)
  {
   tStr=DateToStr(Date());
   tStr+=" ";
   tStr+=TimeToStr(Time());
   tStr+="- ��� ������ � �������: ";
   tStr+=WatchDogSettings->SERV;
   tStr+=" ���������� �� ������!\r\n";
   tStr+="������:"+exception.Message+"\r\n";
   AddToLogFile( tStr);
   RetValue=false;
  }
 return RetValue;
}

//---------------------------------------------------------------------------
//������� ������������� ���������� � � ������ ����� ���������� true;
bool __fastcall TWatchDogThread::RestartConnection()
{
 DWORD R;
 DWORD count;
 AnsiString temp;
 int i;
 char C[600];
 int calls=0;
 //���� ���� ����� �� ���������� ����������(��� �������)
 if (!WatchDogRasCon->hrasconn==NULL)
  RasHangUp(WatchDogRasCon->hrasconn);
 //������ ������ ����� ����������
  RasDialParams->dwSize=sizeof(RASDIALPARAMS);
  temp="";
  StrCopy(RasDialParams->szEntryName,WatchDogRasCon->RAC.c_str());
  //Application->ProcessMessages();
  for (int i=0;i<WatchDogSettings->Calls;i++)
   {
    R=RasDial(NULL,NULL,RasDialParams,0,RasDialFunc,&WatchDogRasCon->hrasconn);
    if (R!=0)
     {
      RasGetErrorString(R,C,600);
      temp=DateToStr(Date());
      temp+=" ";
      temp+=TimeToStr(Time());
      temp+="-";
      temp+=C;
      temp+="\r\n";
      AddToLogFile(temp);
      if (!WatchDogRasCon->hrasconn==NULL)
       {
        RasHangUp(WatchDogRasCon->hrasconn);
        WatchDogRasCon->hrasconn=NULL;
       }
     }
    else
    {
     while (true)
      {
       if (Connected)
        {
        i=WatchDogSettings->Calls;
        break;
        }
       if (ConnectError) break;
      }
    }
   }
    return Connected;
}

void __fastcall TWatchDogThread::TimerOverflow()
{
 AnsiString temp;
 if (VerifyActiveConnection())
  {
   if (!TestActiveConnection())
    {
     if (!RestartConnection())
      {
       temp=DateToStr(Date());
       temp+=" ";
       temp+=TimeToStr(Time());
       temp+="-��������� ������� ��� �����������! �����������:\"";
       temp+=WatchDogSettings->RAC;
       temp+="\" �� �������! ";
       temp+="������ ����������! ��� ����� ������� ������������ ������!\r\n";
       AddToLogFile(temp);
       Terminate();
      }
    }
   }
 else
  {
   temp=DateToStr(Date());
   temp+=" ";
   temp+=TimeToStr(Time());
   temp+="-�����������:\"";
   temp+=WatchDogSettings->RAC;
   temp+="\" �� �������!\r\n";
   AddToLogFile(temp);
  }
}
//---------------------------------------------------------------------------
VOID WINAPI RasDialFunc(UINT unMsg,	// type of event that has occurred
                        RASCONNSTATE rasconnstate,	// connection state about to be entered
                        DWORD dwError	// error that may have occurred
                        )
{
 AnsiString temp;
 WatchDogThread->SetWDConnStateAMsg(unMsg);
 WatchDogThread->SetWDConnStateAState(rasconnstate);
 WatchDogThread->SetWDConnStateError(dwError);
 temp=DateToStr(Date());
 temp+=" ";
 temp+=TimeToStr(Time());
 temp+="-";
 temp+=WatchDogThread->RasGetStatusString(rasconnstate,dwError);
 temp+="\r\n";
 WatchDogThread->AddToLogFile(temp);
}

//---------------------------------------------------------------------------
AnsiString __fastcall TWatchDogThread::RasGetStatusString(RASCONNSTATE ConnState,DWORD Error)
{
 AnsiString S;
 char Buf[100];
 if (Error!=0)
  {
   RasGetErrorString(Error,Buf,100);
   ConnectError=true;
   return Buf;
  }
 else
  {
   S="";
   switch (ConnState)
    {
     case RASCS_OpenPort :
      S=" �������� ����� ";
      break;
     case RASCS_PortOpened :
      S=" ���� ������ ";
      break;
     case RASCS_ConnectDevice:
      S=" ���������� � �����������! ";
      break;
     case RASCS_DeviceConnected:
      S=" ���������� � ����������� ������ �������! ";
      break;
     case RASCS_AllDevicesConnected:
      S=" ������:RASCS_AllDevicesConnected ";
      break;
     case RASCS_Authenticate:
      S=" �������� ����� ������������ � ������! ";
      break;
     case RASCS_AuthNotify:
      S=" ������:RASCS_AuthNotify ";
      break;
     case RASCS_AuthRetry:
      S=" ������:RASCS_AuthRetry ";
      break;
     case RASCS_AuthCallback:
      S=" ������:RASCS_AuthCallback ";
      break;
     case RASCS_AuthChangePassword:
      S=" ������:RASCS_AuthChangePassword ";
      break;
     case RASCS_AuthProject:
      S=" ������:RASCS_AuthProject ";
      break;
     case RASCS_AuthLinkSpeed:
      S=" ������:RASCS_AuthLinkSpeed ";
      break;
     case RASCS_AuthAck:
      S=" ������:RASCS_AuthAck ";
      break;
     case RASCS_ReAuthenticate:
      S=" ������:RASCS_ReAuthenticate ";
      break;
     case RASCS_Authenticated:
      S=" ������:RASCS_Authenticated ";
      break;
     case RASCS_PrepareForCallback:
      S=" ������:RASCS_PrepareForCallback ";
      break;
     case RASCS_WaitForModemReset:
      S=" ������:RASCS_WaitForModemReset ";
      break;
     case RASCS_WaitForCallback:
      S=" ������:RASCS_WaitForCallback ";
      break;
     case RASCS_Projected:
      S=" ������:RASCS_Projected ";
      break;
     case RASCS_SubEntryConnected:
      S=" ������:RASCS_SubEntryConnected ";
      break;
     case RASCS_SubEntryDisconnected:
      S=" ������:RASCS_SubEntryDisconnected ";
      break;
     case RASCS_Interactive:
      S=" ������:RASCS_Interactive ";
      break;
     case RASCS_RetryAuthentication:
      S=" ������:RASCS_RetryAuthentication ";
      break;
     case RASCS_CallbackSetByCaller:
      S=" ������:RASCS_CallbackSetByCaller ";
      break;
     case RASCS_PasswordExpired:
      S=" ������:RASCS_CallbackSetByCaller ";
      break;
     case RASCS_InvokeEapUI:
      S=" ������:RASCS_InvokeEapUI ";
      break;
     case RASCS_Connected:
      S=DateToStr(Date());
      S+=" ";
      S+=TimeToStr(Time());
      S+="-���������� �����������:\"";
      S+=WatchDogSettings->RAC;
      S+="\" ������ �������!\r\n";
      Connected=true;
      break;
     case RASCS_Disconnected:
      S=DateToStr(Date());
      S+=" ";
      S+=TimeToStr(Time());
      S+="-�����������:\"";
      S+=WatchDogSettings->RAC;
      S+="\" ��������!\r\n";
      ConnectError=true;
      break;
    }
    return S;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWatchDogThread::SetWDConnStateAMsg(UINT unMsg)
{
 WatchDogConnState->AMsg = unMsg;
}
//---------------------------------------------------------------------------
void __fastcall TWatchDogThread::SetWDConnStateAState(RASCONNSTATE rasconnstate)
{
 WatchDogConnState->AState=rasconnstate;
}
//---------------------------------------------------------------------------
void __fastcall TWatchDogThread::SetWDConnStateError(DWORD Error)
{
 WatchDogConnState->Error=Error;
}
//---------------------------------------------------------------------------
void __fastcall TWatchDogThread::AddToLogFile(AnsiString msg)
{
 Stream->Write(msg.c_str(),msg.Length());
 Stream->SaveToFile(WatchDogSettings->LogFile);
}


