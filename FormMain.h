//---------------------------------------------------------------------------

#ifndef FormMainH
#define FormMainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ActnMan.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.PlatformDefaultStyleActnCtrls.hpp>
#include <VCLTee.Chart.hpp>
#include <VCLTee.Series.hpp>
#include <VclTee.TeeGDIPlus.hpp>
#include <VCLTee.TeEngine.hpp>
#include <VCLTee.TeeProcs.hpp>

#include "anafestica/PersistFormVCL.h"
#include "anafestica/CfgRegistrySingleton.h"
#include <Vcl.AppEvnts.hpp>
#include <Vcl.ComCtrls.hpp>

#include <array>
#include <cstdint>

#include <MMSystem.h>
#include <fftw3.h>

//---------------------------------------------------------------------------
typedef Anafestica::TPersistFormVCL<Anafestica::Registry::TConfigSingleton>
        TConfigRegistryForm;

//---------------------------------------------------------------------------

template<size_t SIZE, typename T = fftw_complex>
class FFTWCont {
public:
    static size_t get_length() { return SIZE; }
    FFTWCont() : buffer_( (T*)fftw_malloc( sizeof( T ) * SIZE) ) {}
    ~FFTWCont() { fftw_free( buffer_ ); }
    operator T*() const { return buffer_; }
    FFTWCont( FFTWCont const & ) = delete;
    FFTWCont( FFTWCont&& ) = delete;
    FFTWCont& operator=( FFTWCont const & ) = delete;
    FFTWCont& operator=( FFTWCont&& ) = delete;
private:
    T* buffer_;
};

template<typename C>
class FFTWPlan {
public:
    FFTWPlan( C& In, C& Out )
      : p_( fftw_plan_dft_1d( C::get_length(), In, Out, FFTW_FORWARD, FFTW_ESTIMATE ) ) {}
    ~FFTWPlan() { fftw_destroy_plan( p_ ); }
    operator fftw_plan() const { return p_; }
    FFTWPlan( FFTWPlan const & ) = delete;
    FFTWPlan( FFTWPlan&& ) = delete;
    FFTWPlan& operator=( FFTWPlan const & ) = delete;
    FFTWPlan& operator=( FFTWPlan&& ) = delete;
private:
    fftw_plan p_;
};

class BufferUserData {
public:
    BufferUserData( unsigned Idx, TChartSeries& FreqSeries, TChartSeries& TimeSeries )
      : idx_( Idx ), freqSeries_( &FreqSeries ), timeSeries_( &TimeSeries ) {}
    int GetIdx() const { return idx_; }
    TChartSeries& GetFreqSeries() const { return *freqSeries_; }
    TChartSeries& GetTimeSeries() const { return *timeSeries_; }
    BufferUserData( BufferUserData const & ) = delete;
    BufferUserData( BufferUserData&& Rhs ) noexcept
      : idx_( Rhs.idx_ ), freqSeries_( Rhs.freqSeries_ )
      , timeSeries_( Rhs.timeSeries_ )
    {
        Rhs.freqSeries_ = nullptr;
        Rhs.timeSeries_ = nullptr;
    }
    BufferUserData& operator=( BufferUserData const & ) = delete;
    BufferUserData& operator=( BufferUserData&& ) = delete;
private:
    int idx_;
    TChartSeries* freqSeries_;
    TChartSeries* timeSeries_;
};

class TfrmMain : public TConfigRegistryForm
{
__published:	// IDE-managed Components
    TButton *Button3;
    TRadioButton *rbtnLin;
    TRadioButton *rbtnLog;
    TChart *Chart2;
    TLineSeries *Series3;
    TLineSeries *Series4;
    TChart *Chart1;
    TLineSeries *Series1;
    TLineSeries *Series2;
    TButton *Button4;
    TActionManager *ActionManager1;
    TAction *actStart;
    TAction *actStop;
    TAction *actLog;
    TAction *actLinear;
    TApplicationEvents *ApplicationEvents1;
    TComboBoxEx *comboboxAudioSources;
    void __fastcall actStartExecute(TObject *Sender);
    void __fastcall actStartUpdate(TObject *Sender);
    void __fastcall actStopExecute(TObject *Sender);
    void __fastcall actStopUpdate(TObject *Sender);
    void __fastcall actLogExecute(TObject *Sender);
    void __fastcall actLinearExecute(TObject *Sender);
    void __fastcall ApplicationEvents1Idle(TObject *Sender, bool &Done);
private:	// User declarations

    static int constexpr SampleCount = 512;

    //using BufferCont = std::vector<int16_t>;
    using BufferCont = std::array<int16_t,SampleCount>;
    using FFTWContN = FFTWCont<SampleCount>;
    using BufferUserDataCont = std::vector<BufferUserData>;

    WAVEFORMATEX waveFormat_;
    HWAVEIN waveHandle_;
    bool stopped_ { true };
    BufferCont waveData_[2];
    WAVEHDR waveHeader_[2];
    BufferUserDataCont bufferUserData_;
    TChartSeries* latestFreqSeriesShown_ {};
    TChartSeries* latestTimeSeriesShown_ {};

    FFTWContN FFTIn_;
    FFTWContN FFTOut_;
    FFTWPlan<FFTWContN> FFTPlan_;

    void Init();
    void Destroy();
    static String GetModuleFileName();
    void SetupCaption();
    void RestoreProperties();
    void SaveProperties() const;

    bool IsStillSampling() const;

    bool GetLogView() const;
    void SetLogView( bool Val );

    String GetSource() const;
    void SetSource( String Val );

    __property bool LogView = { read = GetLogView, write = SetLogView };
    __property String Source = { read = GetSource, write = SetSource };
protected:
    void __fastcall WndProc( Winapi::Messages::TMessage &Message ) override;

public:		// User declarations
    virtual __fastcall TfrmMain(TComponent* Owner) override;
    __fastcall TfrmMain( TComponent* Owner, StoreOpts StoreOptions,
                         Anafestica::TConfigNode* const RootNode = 0 );
    __fastcall ~TfrmMain() /* throw() */;
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
