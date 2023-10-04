object frmSettings: TfrmSettings
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Settings'
  ClientHeight = 251
  ClientWidth = 387
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object pnlBottom: TPanel
    Left = 0
    Top = 214
    Width = 387
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    ExplicitWidth = 341
    object btnCancel: TButton
      Left = 308
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 227
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
    end
  end
  object pages: TPageControl
    Left = 0
    Top = 0
    Width = 387
    Height = 214
    ActivePage = tsTotp
    Align = alClient
    TabOrder = 0
    ExplicitWidth = 341
    object tsTotp: TTabSheet
      Caption = 'TOTP'
      ImageIndex = 2
      ExplicitWidth = 333
      object lblPassword: TLabel
        Left = 8
        Top = 8
        Width = 92
        Height = 13
        Caption = 'Password (base32)'
      end
      object edTotpPassword: TEdit
        Left = 106
        Top = 5
        Width = 270
        Height = 21
        TabOrder = 0
      end
      object chbTotpStorePasswordInFile: TCheckBox
        Left = 8
        Top = 32
        Width = 322
        Height = 17
        Caption = 'Store password in settings file (insecure, plain text!)'
        TabOrder = 1
      end
    end
    object tsGeneral: TTabSheet
      Caption = 'General'
      object chbAlwaysOnTop: TCheckBox
        Left = 3
        Top = 3
        Width = 325
        Height = 17
        Caption = 'Window always on top'
        TabOrder = 0
        OnClick = chbAlwaysOnTopClick
      end
    end
    object tsLogging: TTabSheet
      Caption = 'Logging'
      ImageIndex = 1
      object lblUiCapacity: TLabel
        Left = 5
        Top = 26
        Width = 194
        Height = 13
        Caption = 'Visible (buffered) log lines in log window:'
      end
      object cmbMaxUiLogLines: TComboBox
        Left = 216
        Top = 23
        Width = 111
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 0
        OnChange = cmbMaxUiLogLinesChange
        Items.Strings = (
          '100'
          '200'
          '500'
          '1000'
          '2000'
          '5000'
          '10000')
      end
      object chbLogToFile: TCheckBox
        Left = 5
        Top = 3
        Width = 325
        Height = 17
        Caption = 'Log to file'
        TabOrder = 1
        OnClick = chbAlwaysOnTopClick
      end
    end
  end
end
