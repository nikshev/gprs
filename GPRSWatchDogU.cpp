//---------------------------------------------------------------------------
#include "GPRSWatchDogU.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TWatchDog *WatchDog;
TWatchDogThread* WatchDogThread;

//Прототип CallBack функции
VOID WINAPI RasDialFunc(
    UINT unMsg,	// type of event that has occurred
    RASCONNSTATE rasconnstate,	// connection state about to be entered
    DWORD dwError	// error that may have occurred
    );


char winPath[100]; //Переменная в которой хранится путь к системной папке
char confFileName[15]="GPRSWDSet.conf"; //Имя конфиг. файла
char confFile[115]="";//Переменная в которой хранится путь и имя конфиг файла
char debugFileName[8]="DWD.log"; //Имя отладочного файла
char debugFile[108]="";//Переменная в которой хранится путь и имя отладочного файла

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
  AnsiString temp;//Для сохранения настроек
  char Buffer[1500]; //Буффер
  UINT BufferPos; //Указатель на буфер
  UINT Result; //Переменная для хранения результатов API функций
  bool Error; //Флаг ошибки
  char tempPath[200]; //Переменная в которой хранится путь к Temp папке
  AnsiString backLogName; //Новое имя для лог файла
  int  i;

  Stream = new TMemoryStream(); //Создаём поток
  WatchDogSettings = new TWatchDogSettings();
  WatchDogConnState =new TWatchDogConnState();
  WatchDogRasCon =new TWatchDogRasCon();
  RasDialParams = new TRasDialParams();
  Connected=false;
  ConnectError=false;
  Result=GetWindowsDirectory(winPath,sizeof(winPath));
  StrCat(winPath,"\\");//Добавляем разделители
  StrCat(confFile,winPath);//Начало пути в результирующую строку
  StrCat(confFile,confFileName);//Имя файла в результрирующую строку
  Stream->LoadFromFile(confFile); //Вытаскиваем из файла строки в поток
  Stream->Seek(0,soFromBeginning);
  Stream->Read(Buffer,Stream->Size);
  BufferPos=0;
  Error=false;

  //Ищем первый параметр. Переменная PATH
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;
  //Если найлдена переменная PATH
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
   Error=true; //Если чего то не получилось то ошибка

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


 //Ищем второй параметр переменная MAXROW
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;

  //Если найдена переменная MAXROW
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
    Error=true; //Если чего то не получилось то ошибка

 //Ищем третий параметр переменная CONN
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;

  //Если найдена переменная CONN
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
   Error=true; //Если чего то не получилось то ошибка

 //Ищем четвёртый параметр переменная CALLS
 while (Buffer[BufferPos]!='<')
  BufferPos++;
 BufferPos++;

 //Если найдена переменная CONN
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
   Error=true; //Если чего то не получилось то ошибка

 //Ищем пятый параметр переменная RAC
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;

 //Если найдена переменная RAC
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
   Error=true; //Если чего то не получилось то ошибка

//Ищем шестой параметр переменная SERVER
  while (Buffer[BufferPos]!='<')
   BufferPos++;
  BufferPos++;

 //Если найдена переменная SERV
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
   Error=true; //Если чего то не получилось то ошибка


 temp=DateToStr(Date());
 temp+=" ";
 temp+=TimeToStr(Time());

 if (!Error) //Если не было ошибок
 {
  temp+="-Установки считаны успешно! Сервис запущен!\r\n";
  Stream->Clear();
  AddToLogFile(temp);
 }
 else
  {
   temp+="-Неправильный кофиг! Сервис будет остановлен!";
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
//Функция проверяет активные соединения и в случае удачи возвращает true
bool __fastcall TWatchDogThread::VerifyActiveConnection()
{
 UINT Result;
 int i;
 AnsiString tmpStr, tmpStr1;
  ///Проверяем активные подключения
 WatchDogRasCon->rasConn[0].dwSize=sizeof (RASCONN);
 WatchDogRasCon->ConBufferSize=sizeof(WatchDogRasCon->rasConn)*10; //Устанавливаем размер буфера максимум десять сетевых подключений
 Result=RasEnumConnections(&WatchDogRasCon->rasConn[0],&WatchDogRasCon->ConBufferSize,&WatchDogRasCon->Connections);
 if (Result==0 && WatchDogRasCon->Connections>0) //Если существуют активные подключения
  { //Перебираем их и ищем выбранное в списке
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
   tmpStr+="-Подключение:\"";
   tmpStr+=WatchDogSettings->RAC;
   tmpStr+="\" активно!\r\n";
   AddToLogFile( tmpStr);
   return true;
  }
 }


//---------------------------------------------------------------------------
//Функция пингует заданный сервер в конфиге и если пинг не проходит
//возвращает false, а в случе удачи возвращает true
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
     tStr+="- Эхо запрос к серверу: ";
     tStr+=WatchDogSettings->SERV;
     tStr+=" завершился удачно!\r\n";
     AddToLogFile( tStr);
     RetValue=true;
    }
   else
    {
     tStr=DateToStr(Date());
     tStr+=" ";
     tStr+=TimeToStr(Time());
     tStr+="- Эхо запрос к серверу: ";
     tStr+=WatchDogSettings->SERV;
     tStr+=" завершился не удачно! Превышен интервал ожидания\r\n";
     AddToLogFile( tStr);
     RetValue=false;
    }
  }
  catch (Exception &exception)
  {
   tStr=DateToStr(Date());
   tStr+=" ";
   tStr+=TimeToStr(Time());
   tStr+="- Эхо запрос к серверу: ";
   tStr+=WatchDogSettings->SERV;
   tStr+=" завершился не удачно!\r\n";
   tStr+="Ошибка:"+exception.Message+"\r\n";
   AddToLogFile( tStr);
   RetValue=false;
  }
 return RetValue;
}

//---------------------------------------------------------------------------
//Функция перезагружает соединение и в случае удачи возвращает true;
bool __fastcall TWatchDogThread::RestartConnection()
{
 DWORD R;
 DWORD count;
 AnsiString temp;
 int i;
 char C[600];
 int calls=0;
 //Если есть хендл то сбрасываем соединение(для отладки)
 if (!WatchDogRasCon->hrasconn==NULL)
  RasHangUp(WatchDogRasCon->hrasconn);
 //Дальше делаем новое соединение
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
       temp+="-Исчерпаны попытки для перезапуска! Подключение:\"";
       temp+=WatchDogSettings->RAC;
       temp+="\" не активно! ";
       temp+="Сервис остановлен! Для новой попытки перезпустите сервис!\r\n";
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
   temp+="-Подключение:\"";
   temp+=WatchDogSettings->RAC;
   temp+="\" не активно!\r\n";
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
      S=" Открытие порта ";
      break;
     case RASCS_PortOpened :
      S=" Порт открыт ";
      break;
     case RASCS_ConnectDevice:
      S=" Соединение с устройством! ";
      break;
     case RASCS_DeviceConnected:
      S=" Соединение с устройством прошло успешно! ";
      break;
     case RASCS_AllDevicesConnected:
      S=" Статус:RASCS_AllDevicesConnected ";
      break;
     case RASCS_Authenticate:
      S=" Проверка имени пользователя и пароля! ";
      break;
     case RASCS_AuthNotify:
      S=" Статус:RASCS_AuthNotify ";
      break;
     case RASCS_AuthRetry:
      S=" Статус:RASCS_AuthRetry ";
      break;
     case RASCS_AuthCallback:
      S=" Статус:RASCS_AuthCallback ";
      break;
     case RASCS_AuthChangePassword:
      S=" Статус:RASCS_AuthChangePassword ";
      break;
     case RASCS_AuthProject:
      S=" Статус:RASCS_AuthProject ";
      break;
     case RASCS_AuthLinkSpeed:
      S=" Статус:RASCS_AuthLinkSpeed ";
      break;
     case RASCS_AuthAck:
      S=" Статус:RASCS_AuthAck ";
      break;
     case RASCS_ReAuthenticate:
      S=" Статус:RASCS_ReAuthenticate ";
      break;
     case RASCS_Authenticated:
      S=" Статус:RASCS_Authenticated ";
      break;
     case RASCS_PrepareForCallback:
      S=" Статус:RASCS_PrepareForCallback ";
      break;
     case RASCS_WaitForModemReset:
      S=" Статус:RASCS_WaitForModemReset ";
      break;
     case RASCS_WaitForCallback:
      S=" Статус:RASCS_WaitForCallback ";
      break;
     case RASCS_Projected:
      S=" Статус:RASCS_Projected ";
      break;
     case RASCS_SubEntryConnected:
      S=" Статус:RASCS_SubEntryConnected ";
      break;
     case RASCS_SubEntryDisconnected:
      S=" Статус:RASCS_SubEntryDisconnected ";
      break;
     case RASCS_Interactive:
      S=" Статус:RASCS_Interactive ";
      break;
     case RASCS_RetryAuthentication:
      S=" Статус:RASCS_RetryAuthentication ";
      break;
     case RASCS_CallbackSetByCaller:
      S=" Статус:RASCS_CallbackSetByCaller ";
      break;
     case RASCS_PasswordExpired:
      S=" Статус:RASCS_CallbackSetByCaller ";
      break;
     case RASCS_InvokeEapUI:
      S=" Статус:RASCS_InvokeEapUI ";
      break;
     case RASCS_Connected:
      S=DateToStr(Date());
      S+=" ";
      S+=TimeToStr(Time());
      S+="-Перезапуск подключения:\"";
      S+=WatchDogSettings->RAC;
      S+="\" прошёл успешно!\r\n";
      Connected=true;
      break;
     case RASCS_Disconnected:
      S=DateToStr(Date());
      S+=" ";
      S+=TimeToStr(Time());
      S+="-Подключения:\"";
      S+=WatchDogSettings->RAC;
      S+="\" сброшено!\r\n";
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


