object frmMain: TfrmMain
  Left = 0
  Top = 0
  Caption = 'frmMain'
  ClientHeight = 714
  ClientWidth = 645
  Color = clBtnFace
  DoubleBuffered = True
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  PixelsPerInch = 96
  TextHeight = 13
  object Button3: TButton
    Left = 31
    Top = 16
    Width = 75
    Height = 25
    Action = actStart
    TabOrder = 0
  end
  object rbtnLin: TRadioButton
    Left = 128
    Top = 49
    Width = 81
    Height = 17
    Action = actLinear
    TabOrder = 1
  end
  object rbtnLog: TRadioButton
    Left = 24
    Top = 49
    Width = 73
    Height = 17
    Action = actLog
    TabOrder = 2
    TabStop = True
  end
  object Chart2: TChart
    AlignWithMargins = True
    Left = 16
    Top = 417
    Width = 613
    Height = 281
    Margins.Left = 16
    Margins.Top = 0
    Margins.Right = 16
    Margins.Bottom = 16
    Title.Text.Strings = (
      'Time')
    BottomAxis.Automatic = False
    BottomAxis.AutomaticMaximum = False
    BottomAxis.AutomaticMinimum = False
    BottomAxis.LogarithmicBase = 2.000000000000000000
    BottomAxis.Maximum = 512.000000000000000000
    LeftAxis.Automatic = False
    LeftAxis.AutomaticMaximum = False
    LeftAxis.AutomaticMinimum = False
    LeftAxis.Maximum = 34000.000000000000000000
    LeftAxis.Minimum = -34000.000000000000000000
    RightAxis.Visible = False
    View3D = False
    Zoom.Allow = False
    Align = alBottom
    BevelOuter = bvLowered
    Constraints.MinHeight = 200
    TabOrder = 3
    Anchors = [akLeft, akTop, akRight, akBottom]
    DefaultCanvas = 'TGDIPlusCanvas'
    ColorPaletteIndex = 13
    object Series3: TLineSeries
      HoverElement = [heCurrent]
      Legend.Visible = False
      SeriesColor = clBlue
      ShowInLegend = False
      Brush.BackColor = clDefault
      ClickableLine = False
      Pointer.Brush.Gradient.EndColor = 10708548
      Pointer.Gradient.EndColor = 10708548
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      XValues.Name = 'X'
      XValues.Order = loAscending
      YValues.Name = 'Y'
      YValues.Order = loNone
    end
    object Series4: TLineSeries
      HoverElement = [heCurrent]
      Legend.Visible = False
      Active = False
      SeriesColor = clBlue
      ShowInLegend = False
      Brush.BackColor = clDefault
      ClickableLine = False
      Pointer.Brush.Gradient.EndColor = 10708548
      Pointer.Gradient.EndColor = 10708548
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      XValues.Name = 'X'
      XValues.Order = loAscending
      YValues.Name = 'Y'
      YValues.Order = loNone
    end
  end
  object Chart1: TChart
    AlignWithMargins = True
    Left = 16
    Top = 80
    Width = 613
    Height = 321
    Margins.Left = 16
    Margins.Top = 80
    Margins.Right = 16
    Margins.Bottom = 16
    Title.Text.Strings = (
      'Freq')
    BottomAxis.Automatic = False
    BottomAxis.AutomaticMaximum = False
    BottomAxis.AutomaticMinimum = False
    BottomAxis.LogarithmicBase = 2.000000000000000000
    BottomAxis.Maximum = 11025.000000000000000000
    BottomAxis.Minimum = 60.000000000000000000
    LeftAxis.Automatic = False
    LeftAxis.AutomaticMaximum = False
    LeftAxis.AutomaticMinimum = False
    LeftAxis.Maximum = 10.000000000000000000
    LeftAxis.Minimum = -144.000000000000000000
    RightAxis.Visible = False
    View3D = False
    Zoom.Allow = False
    Align = alClient
    BevelOuter = bvLowered
    Constraints.MinHeight = 200
    TabOrder = 4
    DefaultCanvas = 'TGDIPlusCanvas'
    ColorPaletteIndex = 13
    object Series1: TLineSeries
      HoverElement = [heCurrent]
      Legend.Visible = False
      SeriesColor = clBlue
      ShowInLegend = False
      Brush.BackColor = clDefault
      ClickableLine = False
      Pointer.Brush.Gradient.EndColor = 10708548
      Pointer.Gradient.EndColor = 10708548
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      XValues.Name = 'X'
      XValues.Order = loAscending
      YValues.Name = 'Y'
      YValues.Order = loNone
    end
    object Series2: TLineSeries
      HoverElement = [heCurrent]
      Legend.Visible = False
      Active = False
      SeriesColor = clBlue
      ShowInLegend = False
      Brush.BackColor = clDefault
      ClickableLine = False
      Pointer.InflateMargins = True
      Pointer.Style = psRectangle
      XValues.Name = 'X'
      XValues.Order = loAscending
      YValues.Name = 'Y'
      YValues.Order = loNone
    end
  end
  object Button4: TButton
    Left = 120
    Top = 16
    Width = 75
    Height = 25
    Action = actStop
    TabOrder = 5
  end
  object comboboxAudioSources: TComboBoxEx
    Left = 312
    Top = 18
    Width = 281
    Height = 22
    ItemsEx = <>
    TabOrder = 6
    Text = 'comboboxAudioSources'
  end
  object ActionManager1: TActionManager
    Left = 432
    Top = 72
    StyleName = 'Platform Default'
    object actStart: TAction
      Caption = 'Start'
      OnExecute = actStartExecute
      OnUpdate = actStartUpdate
    end
    object actStop: TAction
      Caption = 'Stop'
      OnExecute = actStopExecute
      OnUpdate = actStopUpdate
    end
    object actLog: TAction
      AutoCheck = True
      Caption = 'dB'
      Checked = True
      GroupIndex = 1
      OnExecute = actLogExecute
    end
    object actLinear: TAction
      AutoCheck = True
      Caption = 'Linear'
      GroupIndex = 1
      OnExecute = actLinearExecute
    end
  end
  object ApplicationEvents1: TApplicationEvents
    OnIdle = ApplicationEvents1Idle
    Left = 224
    Top = 16
  end
end
