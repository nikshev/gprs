//---------------------------------------------------------------------------
#ifndef GPRSWatchDogUH
#define GPRSWatchDogUH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <SvcMgr.hpp>
#include <vcl.h>
#include <ExtCtrls.hpp>
#include <Ras.h>
#include <winsock.h>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdIcmpClient.hpp>
#include <IdRawBase.hpp>
#include <IdRawClient.hpp>
//---------------------------------------------------------------------------
//Объявляем структуру IPINFO
#ifndef __IPINFO_DEFINED__
#define __IPINFO_DEFINED__

typedef struct _IPINFO
{
 char Ttl;
 char Tos;
 char IPFlags;
 char OptSize;
 char Options;
} IPINFO;

#endif //_IPINFO

//Объявляем структуру ICMPECHO
#ifndef __ICMPECHO_DEFINED__
#define __ICMPECHO_DEFINED__

typedef struct _ICMPECHO
{
  DWORD  Source;
  DWORD  Status;
  DWORD  RTTime;
  DWORD  DataSize;
  DWORD  Reserved;
  Variant *pData;
  IPINFO i_ipinfo;

} ICMPECHO;

#endif //_ICMPECHO

typedef IPINFO *LPIPINFO;
typedef ICMPECHO TIcmpEcho;

struct TWatchDogSettings
{
 AnsiString LogFile;    //Путь и имя Лог файла
 AnsiString DebugFile; //Путь и имя отладочного файла
 UINT LogMaxRow; //Максимальное кол-во строк в лог файле
 UINT Connects; //Попытки соединения до разрыва
 UINT Calls; //Кол -во звонков до разрыва
 AnsiString RAC; //Имя подключения с типом "Удалённый доступ"
 AnsiString SERV; //Сервер который потом пингуется
};

struct TWatchDogRasCon
{
 AnsiString RAC; //Имя подключения с типом "Удалённый доступ"
 RASCONN rasConn[100]; //Информация об  активных подключений
 HRASCONN hrasconn; //Хендл подключения
 RASCONNSTATUS rasConStatus; //Сатус удаллённого подключения
 DWORD ConBufferSize; //Размер буффера
 DWORD Connections; //Количество подключений удаллённого доступа активных
};

struct TWatchDogConnState
{
 UINT AMsg;
 RASCONNSTATE AState;
 DWORD Error;
};

typedef RASDIALPARAMS TRasDialParams;

class TWatchDog : public TService
{
__published:    // IDE-managed Components
        TIdIcmpClient *IdIcmpClient;
        void __fastcall ServiceStart(TService *Sender, bool &Started);
        void __fastcall ServiceContinue(TService *Sender, bool &Continued);
        void __fastcall ServicePause(TService *Sender, bool &Paused);
        void __fastcall ServiceStop(TService *Sender, bool &Stopped);
private:        // User declarations

public:         // User declarations
      	__fastcall TWatchDog(TComponent* Owner);
	TServiceController __fastcall GetServiceController(void);

	friend void __stdcall ServiceController(unsigned CtrlCode);
};
//---------------------------------------------------------------------------
extern PACKAGE TWatchDog *WatchDog;
//---------------------------------------------------------------------------
class TWatchDogThread: public TThread
{
 private:
  TMemoryStream *Stream;
  TWatchDogSettings *WatchDogSettings;
  TRasDialParams *RasDialParams;
  TWatchDogConnState *WatchDogConnState;
  TWatchDogRasCon *WatchDogRasCon;
  bool Connected;
  bool ConnectError;
 protected:
  void __fastcall Execute(); //Метод выполнения потока
  bool __fastcall VerifyActiveConnection();
  bool __fastcall TestActiveConnection();
  bool __fastcall RestartConnection();
  void __fastcall TimerOverflow();
 public:
  TIdIcmpClient *IdIcmpClient;
  __fastcall TWatchDogThread(bool CreateSuspended); //Конструктор
  AnsiString __fastcall RasGetStatusString(RASCONNSTATE ConnState,DWORD Error);
  void __fastcall SetWDConnStateAMsg(UINT unMsg);
  void __fastcall SetWDConnStateAState(RASCONNSTATE rasconnstate);
  void __fastcall SetWDConnStateError(DWORD Error);
  void __fastcall AddToLogFile(AnsiString msg);
};
#endif
