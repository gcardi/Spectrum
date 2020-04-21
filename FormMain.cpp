//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
#include <complex>

#include <System.Win.ComObj.hpp>

#include <anafestica/FileVersionInfo.h>

#include "SysUt.Fmt.h"

#include "FormMain.h"

using std::complex;
using std::abs;

using Anafestica::TFileVersionInfo;

using SysUt::Fmt;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
//---------------------------------------------------------------------------

__fastcall TfrmMain::TfrmMain(TComponent* Owner)
  : TfrmMain( Owner, StoreOpts::All, nullptr )
{
}
//---------------------------------------------------------------------------

__fastcall TfrmMain::TfrmMain( TComponent* Owner, StoreOpts StoreOptions,
                               Anafestica::TConfigNode* const RootNode )
    : TConfigRegistryForm( Owner, StoreOptions, RootNode )
    , FFTPlan_( FFTIn_, FFTOut_ )
{
    Init();
}
//---------------------------------------------------------------------------

__fastcall TfrmMain::~TfrmMain()
{
    try {
        Destroy();
    }
    catch ( ... ) {
    }
}
//---------------------------------------------------------------------------

bool TfrmMain::GetLogView() const
{
    return actLog ? actLog->Checked : true;
}
//---------------------------------------------------------------------------

void TfrmMain::SetLogView( bool Val )
{
    if ( actLog ) {
        actLog->Checked = Val;
        if ( Val ) {
            rbtnLog->Checked = true;
        }
        else {
            rbtnLin->Checked = true;
        }
    }
}
//---------------------------------------------------------------------------

String TfrmMain::GetSource() const
{
    if ( comboboxAudioSources->ItemIndex >= 0 ) {
        return comboboxAudioSources->Items->Strings[comboboxAudioSources->ItemIndex];
    }
    return String();
}
//---------------------------------------------------------------------------

void TfrmMain::SetSource( String Val )
{
    auto Idx = comboboxAudioSources->Items->IndexOf( Val );
    if ( Idx >= 0 ) {
        comboboxAudioSources->ItemIndex = Idx;
    }
}
//---------------------------------------------------------------------------

void TfrmMain::Init()
{
    comboboxAudioSources->Items->Clear();

    WAVEINCAPS     wic;
    unsigned long  iNumDevs;

    /* Get the number of Digital Audio In devices in this computer */
    iNumDevs = waveInGetNumDevs();

    /* Go through all of those devices, displaying their names */
    for ( unsigned long i = 0; i < iNumDevs ; ++i ) {
        /* Get info about the next device */
        if ( !waveInGetDevCaps( i, &wic, sizeof wic ) ) {
            /* Display its Device ID and name */
            comboboxAudioSources->Items->Append(
                Fmt( _T( "Device ID #%u: %s\r\n" ), i, wic.szPname )
            );
        }
    }

    if ( comboboxAudioSources->Items->Count > 0 ) {
        comboboxAudioSources->ItemIndex = 0;
    }

    SetupCaption();
    RestoreProperties();
    Chart2->BottomAxis->Maximum = SampleCount - 1;
    bufferUserData_.emplace_back( 0, *Series1, *Series3 );
    bufferUserData_.emplace_back( 1, *Series2, *Series4 );
}
//---------------------------------------------------------------------------

void TfrmMain::Destroy()
{
    SaveProperties();
}
//---------------------------------------------------------------------------

String TfrmMain::GetModuleFileName()
{
    return GetModuleName( reinterpret_cast<unsigned>( HInstance ) );
}
//---------------------------------------------------------------------------

void TfrmMain::SetupCaption()
{
    TFileVersionInfo const Info( GetModuleFileName() );
    Caption =
        SysUt::Fmt( _T( "%s, Ver %s" ), Application->Title, Info.ProductVersion );
}
//---------------------------------------------------------------------------

void TfrmMain::RestoreProperties()
{
    RESTORE_LOCAL_PROPERTY( LogView );
    RESTORE_LOCAL_PROPERTY( Source );
}
//---------------------------------------------------------------------------

void TfrmMain::SaveProperties() const
{
    SAVE_LOCAL_PROPERTY( LogView );
    SAVE_LOCAL_PROPERTY( Source );
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::WndProc( Winapi::Messages::TMessage &Message )
{
    TConfigRegistryForm::WndProc( Message );
    const bool Log = LogView;
    switch ( Message.Msg ) {
        case MM_WIM_OPEN:
            //::OutputDebugString( _T( "MM_WIM_OPEN" ) );
            break;
        case MM_WIM_DATA:
            if ( WAVEHDR* Hdr = (WAVEHDR *)Message.LParam ) {
                if ( DWORD BytesRecorded = Hdr->dwBytesRecorded ) {
                    auto const UserData =
                        reinterpret_cast<BufferUserData const *>( Hdr->dwUser );

                    TChartSeries& CurrentFreqSeries = UserData->GetFreqSeries();
                    CurrentFreqSeries.BeginUpdate();

                    TChartSeries& CurrentTimeSeries = UserData->GetTimeSeries();
                    CurrentTimeSeries.BeginUpdate();

                    // Apply windowing (Hanning) & plot the unwindowed
                    // time domain signal
                    for ( int Idx = 0 ; Idx < SampleCount ; ++Idx ) {
                        const double Signal = waveData_[UserData->GetIdx()][Idx];
                        const double W = 1.0 - cos( 2.0 * M_PI * Idx / SampleCount );
                        //CurrentTimeSeries.AddY( W * Signal );
                        CurrentTimeSeries.AddY( Signal );
                        FFTIn_[Idx][0] = W * Signal;
                    }

                    // real FFT
                    fftw_execute( FFTPlan_ );

                    // freq plot
                    auto const Sf = 2.0 / double( SampleCount );
                    for ( int Idx = 0 ; Idx < SampleCount / 2 ; ++Idx ) {
                        double Re = FFTOut_[Idx][0] / 32768.0;
                        double Im = FFTOut_[Idx][1] / 32768.0;
                        complex<double> Sample{ Re, Im };

                        double Freq = 22050.0 * Idx / SampleCount;
                        CurrentFreqSeries.AddXY(
                            Freq,
                            Log ?
                              20.0 * log10( abs( Sample ) * Sf + 1e-15 )
                            :
                              abs( Sample ) * Sf
                            );
                    }

                    CurrentFreqSeries.EndUpdate();

                    CurrentFreqSeries.Active = true;

                    CurrentTimeSeries.EndUpdate();

                    CurrentTimeSeries.Active = true;

                    if ( latestFreqSeriesShown_ ) {
                        latestFreqSeriesShown_->Active = false;
                        latestFreqSeriesShown_->Clear();
                    }

                    if ( latestTimeSeriesShown_ ) {
                        latestTimeSeriesShown_->Active = false;
                        latestTimeSeriesShown_->Clear();
                    }

                    latestFreqSeriesShown_ = &CurrentFreqSeries;
                    latestTimeSeriesShown_ = &CurrentTimeSeries;
                }
                if ( IsStillSampling() ) {
                    waveInAddBuffer( waveHandle_, Hdr, sizeof *Hdr );
                }
                else {
                    waveInClose( waveHandle_ );
                }
            }
            break;
        case MM_WIM_CLOSE:
            //::OutputDebugString( _T( "MM_WIM_CLOSE" ) );
            break;
    }
}
//---------------------------------------------------------------------------

bool TfrmMain::IsStillSampling() const
{
    return !stopped_;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actStartExecute(TObject *Sender)
{
    stopped_ = false;

    waveFormat_.wFormatTag      = WAVE_FORMAT_PCM;
    waveFormat_.nChannels       = 1;
    waveFormat_.nSamplesPerSec  = 22050;
    waveFormat_.wBitsPerSample  = 16;
    waveFormat_.nAvgBytesPerSec = 22050 * 2;
    waveFormat_.nBlockAlign     = 2;
    waveFormat_.cbSize          = 0;
    auto Res = ::waveInOpen(
        &waveHandle_, comboboxAudioSources->ItemIndex,  &waveFormat_, 0, 0,
        WAVE_FORMAT_QUERY
    );
    if ( Res ) {
        RaiseLastOSError();
    }
    Res = ::waveInOpen(
        &waveHandle_, comboboxAudioSources->ItemIndex,
        &waveFormat_, reinterpret_cast<DWORD_PTR>( Handle ),
        0, CALLBACK_WINDOW
    );
    if ( Res ) {
        RaiseLastOSError();
    }

    waveHeader_[0].dwBufferLength = waveData_[0].size() * sizeof( BufferCont::value_type );
    waveHeader_[0].dwFlags        = 0;
    waveHeader_[0].lpData         = reinterpret_cast<LPSTR>( &waveData_[0][0] );
    waveHeader_[0].dwUser         = reinterpret_cast<DWORD_PTR>( &bufferUserData_[0] );

    Res = ::waveInPrepareHeader(
        waveHandle_, &waveHeader_[0], sizeof waveHeader_[0] );
    if ( Res )
        RaiseLastOSError();

    waveHeader_[1].dwBufferLength = waveData_[1].size() * sizeof( BufferCont::value_type );
    waveHeader_[1].dwFlags        = 0;
    waveHeader_[1].lpData         = reinterpret_cast<LPSTR>( &waveData_[1][0] );
    waveHeader_[1].dwUser         = reinterpret_cast<DWORD_PTR>( &bufferUserData_[1] );

    Res = ::waveInPrepareHeader(
            waveHandle_, &waveHeader_[1], sizeof waveHeader_[1]
        );
    if ( Res ) {
        RaiseLastOSError();
    }

    Res = ::waveInAddBuffer(
        waveHandle_, &waveHeader_[0], sizeof waveHeader_[0]
    );
    if ( Res ) {
        RaiseLastOSError();
    }

    Res = ::waveInAddBuffer(
        waveHandle_, &waveHeader_[1], sizeof waveHeader_[1]
    );
    if ( Res ) {
        RaiseLastOSError();
    }

    Res = ::waveInStart( waveHandle_ );
    if ( Res ) {
        RaiseLastOSError();
    }

}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actStartUpdate(TObject *Sender)
{
    TAction& Act = static_cast<TAction&>( *Sender );
    Act.Enabled = !IsStillSampling() && comboboxAudioSources->ItemIndex >= 0;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actStopExecute(TObject *Sender)
{
    stopped_ = true;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actStopUpdate(TObject *Sender)
{
    TAction& Act = static_cast<TAction&>( *Sender );
    Act.Enabled = IsStillSampling();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actLogExecute(TObject *Sender)
{
    Chart1->LeftAxis->Maximum = 10.0;
    Chart1->LeftAxis->Minimum = -144.0;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actLinearExecute(TObject *Sender)
{
    Chart1->LeftAxis->Maximum = 1.0;
    Chart1->LeftAxis->Minimum = 0;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::ApplicationEvents1Idle(TObject *Sender, bool &Done)
{
    comboboxAudioSources->Enabled = !IsStillSampling();
}
//---------------------------------------------------------------------------

