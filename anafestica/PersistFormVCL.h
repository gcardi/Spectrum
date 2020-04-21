//---------------------------------------------------------------------------

#ifndef PersistFormVCLH
#define PersistFormVCLH

#include <System.Classes.hpp>
#include <Vcl.Forms.hpp>

#include <anafestica/Cfg.h>

//---------------------------------------------------------------------------
namespace Anafestica {
//---------------------------------------------------------------------------

// This class is useful to be able to override the dynamic functions
// of delphi in the context of template classes because, at least with
// clang-based compilers, the linker complains that the function,
// although defined, does not exist. As this is not a template class,
// overriding the dynamic functions of delphi is not a problem.
class TPersistBaseForm : public Vcl::Forms::TForm {
public:
	template<typename...A>
	__fastcall TPersistBaseForm( A...Args )
      : Vcl::Forms::TForm( std::forward<A...>( Args )... ) {}
protected:
	DYNAMIC void __fastcall DoShow() {
        Vcl::Forms::TForm::DoShow();
        DoRestoreState();
    }
    virtual void DoRestoreState() = 0;
private:
};

template<typename CfgSingleton>
class TPersistFormVCL : public TPersistBaseForm
{
public:
    using inherited = TPersistBaseForm;

    enum class StoreOpts {
        None, OnlySize, OnlyPos, PosAndSize,
        OnlyState, StateAndSize, StateAndPos, All
    };

    __fastcall TPersistFormVCL( System::Classes::TComponent* Owner,
                                StoreOpts StoreOptions = StoreOpts::All,
                                TConfigNode* const RootNode = nullptr );
	__fastcall TPersistFormVCL( System::Classes::TComponent* AOwner, int Dummy,
                                StoreOpts StoreOptions = StoreOpts::All,
                                TConfigNode* const RootNode = nullptr );
	__fastcall TPersistFormVCL( HWND ParentWindow,
                                StoreOpts StoreOptions = StoreOpts::All,
                                TConfigNode* const RootNode = nullptr );
    virtual void __fastcall BeforeDestruction() override;
    TConfigNode& GetConfigNode() const { return configNode_; }
    static TConfigNode& GetConfigRootNode();
    void ReadValues();
    void SaveValues();
protected:
    virtual void __fastcall DoCreate() override;
    virtual void DoRestoreState() override;
private:
    static constexpr LPCTSTR IdLeft_{ _T( "Left" ) };
    static constexpr LPCTSTR IdTop_{ _T( "Top" ) };
    static constexpr LPCTSTR IdRight_{ _T( "Right" ) };
    static constexpr LPCTSTR IdBottom_{ _T( "Bottom" ) };
    static constexpr LPCTSTR IdState_{ _T( "State" ) };

    TConfigNode& configNode_;
    StoreOpts storeOptions_;

    TConfigNode& GetOrCreateConfigNode( TConfigNode * const RootNode );
    static bool HaveToSaveOrRestoreSize( StoreOpts Val ) noexcept;
    static bool HaveToSaveOrRestoreState( StoreOpts Val ) noexcept;
    static bool HaveToSaveOrRestorePos( StoreOpts Val ) noexcept;
};
//---------------------------------------------------------------------------

template<typename CfgSingleton>
__fastcall TPersistFormVCL<CfgSingleton>::TPersistFormVCL(
                                            System::Classes::TComponent* Owner,
                                            StoreOpts StoreOptions,
                                            TConfigNode* const RootNode )
//  : Vcl::Forms::TForm( Owner )
  : inherited( Owner )
  , storeOptions_( StoreOptions )
  , configNode_( GetOrCreateConfigNode( RootNode ) )
{
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
__fastcall TPersistFormVCL<CfgSingleton>::TPersistFormVCL(
                                          System::Classes::TComponent* AOwner,
                                          int Dummy, StoreOpts StoreOptions,
                                          TConfigNode* const RootNode )
  //: Vcl::Forms::TForm( AOwner, Dummy )
  : inherited( AOwner, Dummy )
  , storeOptions_( StoreOptions )
  , configNode_( GetOrCreateConfigNode( RootNode ) )
{
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
__fastcall TPersistFormVCL<CfgSingleton>::TPersistFormVCL( HWND ParentWindow,
                                                  StoreOpts StoreOptions,
                                                  TConfigNode* const RootNode )
  //: Vcl::Forms::TForm( ParentWindow )
  : inherited( ParentWindow )
  , storeOptions_( StoreOptions )
  , configNode_( GetOrCreateConfigNode( RootNode ) )
{
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
void __fastcall TPersistFormVCL<CfgSingleton>::DoCreate()
{
    //Vcl::Forms::TForm::DoCreate();
    inherited::DoCreate();
    //ReadValues();
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
void TPersistFormVCL<CfgSingleton>::DoRestoreState()
{
    ReadValues();
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
TConfigNode& TPersistFormVCL<CfgSingleton>::GetOrCreateConfigNode(
                                                 TConfigNode * const RootNode )
{
    return RootNode ?
        const_cast<TConfigNode&>( *RootNode )
      :
        GetConfigRootNode().GetSubNode( Name );
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
TConfigNode& TPersistFormVCL<CfgSingleton>::GetConfigRootNode()
{
     return CfgSingleton().GetConfig().GetRootNode();
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
void TPersistFormVCL<CfgSingleton>::ReadValues()
{
    if ( storeOptions_ != StoreOpts::None ) {
        if ( HaveToSaveOrRestorePos( storeOptions_ ) ) {
            int pLeft { BoundsRect.Left };
            RESTORE_VALUE( configNode_, IdLeft_, pLeft );
            int pTop { BoundsRect.Top };
            RESTORE_VALUE( configNode_, IdTop_, pTop );
            if ( HaveToSaveOrRestoreSize( storeOptions_ ) ) {
                int pRight { BoundsRect.Right };
                RESTORE_VALUE( configNode_, IdRight_, pRight );
                int pBottom {BoundsRect.Bottom };
                RESTORE_VALUE( configNode_, IdBottom_, pBottom );
                BoundsRect = TRect( pLeft, pTop, pRight, pBottom );
            }
            else {
                Left = pLeft;
                Top = pTop;
            }
        }
        else if ( HaveToSaveOrRestoreSize( storeOptions_ ) ) {
            int pLeft { BoundsRect.Left };
            RESTORE_VALUE( configNode_, IdLeft_, pLeft );
            int pRight { BoundsRect.Right };
            RESTORE_VALUE( configNode_, IdRight_, pRight );
            int pTop { BoundsRect.Top };
            RESTORE_VALUE( configNode_, IdTop_, pTop );
            int pBottom { BoundsRect.Bottom };
            RESTORE_VALUE( configNode_, IdBottom_, pBottom );
            Width = pRight - pLeft;
            Height = pBottom - pTop;
        }

        if ( HaveToSaveOrRestoreState( storeOptions_ ) ) {
            RESTORE_VALUE( configNode_, IdState_, WindowState );
        }
    }
}

//---------------------------------------------------------------------------

template<typename CfgSingleton>
void __fastcall TPersistFormVCL<CfgSingleton>::BeforeDestruction()
{
    SaveValues();
    inherited::BeforeDestruction();
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
void TPersistFormVCL<CfgSingleton>::SaveValues()
{
    if ( storeOptions_ != StoreOpts::None ) {
        WINDOWPLACEMENT WndPl = {};
        WndPl.length = sizeof WndPl;

        if ( !GetWindowPlacement( Handle, &WndPl ) ) {
            RaiseLastOSError();
        }
        if ( HaveToSaveOrRestorePos( storeOptions_ ) ) {
            SAVE_VALUE(
            	configNode_, IdLeft_,
            	static_cast<int>( WndPl.rcNormalPosition.left )
            );
            SAVE_VALUE(
            	configNode_, IdTop_,
            	static_cast<int>( WndPl.rcNormalPosition.top )
            );
        }
        if ( HaveToSaveOrRestoreSize( storeOptions_ ) ) {
            SAVE_VALUE(
            	configNode_, IdRight_,
            	static_cast<int>( WndPl.rcNormalPosition.right )
            );
            SAVE_VALUE(
            	configNode_, IdBottom_,
            	static_cast<int>( WndPl.rcNormalPosition.bottom )
            );
        }
        if ( HaveToSaveOrRestoreState( storeOptions_ ) ) {
            SAVE_VALUE( configNode_, IdState_, WindowState );
        }
    }
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
bool TPersistFormVCL<CfgSingleton>::HaveToSaveOrRestoreSize( StoreOpts Val ) noexcept
{
    switch ( Val ) {
        case StoreOpts::All:
        case StoreOpts::OnlySize:
        case StoreOpts::PosAndSize:
        case StoreOpts::StateAndSize:
            return true;
        default:
            return false;
    }
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
bool TPersistFormVCL<CfgSingleton>::HaveToSaveOrRestoreState( StoreOpts Val ) noexcept
{
    switch ( Val ) {
        case StoreOpts::OnlyState:
        case StoreOpts::StateAndSize:
        case StoreOpts::StateAndPos:
        case StoreOpts::All:
            return true;
        default:
            return false;
    }
}
//---------------------------------------------------------------------------

template<typename CfgSingleton>
bool TPersistFormVCL<CfgSingleton>::HaveToSaveOrRestorePos( StoreOpts Val ) noexcept
{
    switch ( Val ) {
        case StoreOpts::OnlyPos:
        case StoreOpts::PosAndSize:
        case StoreOpts::StateAndPos:
        case StoreOpts::All:
            return true;
        default:
            return false;
    }
}
//---------------------------------------------------------------------------

#define RESTORE_LOCAL_PROPERTY( PROPERTY ) \
{\
    std::remove_reference_t< decltype( PROPERTY )> Tmp{ PROPERTY }; \
    GetConfigNode().GetItemAs( #PROPERTY, Tmp ); \
    PROPERTY = Tmp; \
}

#define SAVE_LOCAL_PROPERTY( PROPERTY ) \
    GetConfigNode().PutItem( #PROPERTY, PROPERTY )

//---------------------------------------------------------------------------
}; // End of namespace Anafestica
//---------------------------------------------------------------------------

#endif
