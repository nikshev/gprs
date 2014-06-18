object WatchDog: TWatchDog
  OldCreateOrder = False
  DisplayName = 'WatchDog'
  StartType = stManual
  OnContinue = ServiceContinue
  OnPause = ServicePause
  OnStart = ServiceStart
  OnStop = ServiceStop
  Left = 192
  Top = 107
  Height = 150
  Width = 215
  object IdIcmpClient: TIdIcmpClient
    Left = 8
  end
end
