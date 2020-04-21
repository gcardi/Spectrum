//---------------------------------------------------------------------------

#ifndef CfgRegistryH
#define CfgRegistryH

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
#include <regex>
#include <functional>
#include <type_traits>
#include <array>
#include <tuple>

#include <System.Classes.hpp>
#include <System.Win.Registry.hpp>
#include <System.RTLConsts.hpp>

#include <anafestica/Cfg.h>

//---------------------------------------------------------------------------
namespace Anafestica {
//---------------------------------------------------------------------------
namespace Registry {
//---------------------------------------------------------------------------

template <typename IT>
struct ItValueType {
    typedef typename std::iterator_traits<IT>::value_type elem;
};

template <typename Container>
struct ItValueType<std::back_insert_iterator<Container>>
{
    typedef typename Container::value_type elem;
};

template <typename Container>
struct ItValueType<std::front_insert_iterator<Container>>
{
    typedef typename Container::value_type elem;
};

template <typename Container>
struct ItValueType<std::insert_iterator<Container>>
{
    typedef typename Container::value_type elem;
};

struct BuildString {
    template<typename FwIt>
    String operator ()( FwIt Begin, FwIt End ) const {
        return String(
            reinterpret_cast<TCHAR*>( &*Begin ),
            std::distance( Begin, End )
        );
    }
};

enum class TExRegDataType {
    Binary,                 // REG_BINARY
                            // Binary data in any form.

    Dword,                  // REG_DWORD
                            // A 32-bit number. (LE on x86-x64)

    ExpandSz,               // REG_EXPAND_SZ
                            // A null-terminated string that contains unexpanded
                            // references to environment variables (for example,
                            // "%PATH%"). To expand the environment variable
                            // references, use the ExpandEnvironmentStrings function.

    Link,                   // REG_LINK
                            // A null-terminated Unicode string that contains
                            // the target path of a symbolic link that was
                            // created by calling the RegCreateKeyEx function
                            // with REG_OPTION_CREATE_LINK.

    MultiSz,                // REG_MULTI_SZ
                            // A sequence of null-terminated strings, terminated
                            // by an empty string (\0).

    None,                   // REG_NONE
                            // No defined value type.

    Qword,                  // REG_QWORD
                            // A 64-bit number.

    Sz                      // REG_SZ
                            // A null-terminated string.
};

class TRegistry : public System::Win::Registry::TRegistry {
public:

    template<typename...A>
    TRegistry( A...Args )
      : System::Win::Registry::TRegistry( std::forward<A...>( Args )... ) {}

    template<typename T>
    typename std::enable_if_t<std::is_integral_v<T> && sizeof( T ) == 8,T>
    ReadQWORD( String Name );

    size_t ReadStrings( String Name, TStrings& Result ) {
        return ReadStringsTo( Name, System::back_inserter( &Result ) );
    }

    template<typename OutIt, typename BT = BuildString>
    size_t ReadStringsTo( String Name, OutIt it );

    template<typename OutIt>
    typename
        std::enable_if_t<
            std::is_integral_v<typename ItValueType<OutIt>::elem> &&
              sizeof( typename ItValueType<OutIt>::elem ) == 1,
            size_t
        >
    ReadBinaryDataTo( String Name, OutIt It );

    TBytes ReadBinaryData( String Name ) {
        DWORD Type {};
        DWORD Size {};

        if ( !CheckResult(
               ::RegQueryValueEx(
                 CurrentKey, Name.c_str(), nullptr, &Type, nullptr, &Size
               )
             )
        )
        {
            throw ERegistryException(
                &_SRegGetDataFailed, ARRAYOFCONST(( Name ))
            );
        }

        if ( Type != REG_BINARY ) {
            throw ERegistryException(
                &_SInvalidRegType, ARRAYOFCONST(( Name ))
            );
        }

        TBytes Data;
        Data.Length = Size;

        if ( !CheckResult(
               ::RegQueryValueEx(
                 CurrentKey, Name.c_str(), nullptr, &Type,&Data[0], &Size
               )
             )
        )
        {
            throw ERegistryException(
                &_SRegGetDataFailed, ARRAYOFCONST(( Name ))
            );
        }

        // check again if the type has changed in the meantime
        if ( Type != REG_BINARY ) {
            throw ERegistryException(
                &_SInvalidRegType, ARRAYOFCONST(( Name ))
            );
        }

        return Data;
    }

    void WriteQWORD( String Name, unsigned long long Value ) {
        if ( !CheckResult(
               ::RegSetValueEx(
                 CurrentKey, Name.c_str(), 0, REG_QWORD,
                 reinterpret_cast<BYTE*>( &Value ), sizeof Value
               )
             )
        )
        {
            throw ERegistryException(
                &_SRegSetDataFailed, ARRAYOFCONST(( Name ))
            );
        }
    }

    void WriteQWORD( String Name, long long Value ) {
        WriteQWORD( Name, static_cast<unsigned long long>( Value ) );
    }

    void WriteStrings( String Name, TStrings& Value ) {
        WriteStrings( Name, System::begin( &Value ), System::end( &Value ) );
    }

    void WriteStrings( String Name, std::vector<String> const & Value ) {
        WriteStrings( Name, std::begin( Value ), std::end( Value ) );
    }

    template<typename InIt>
    void WriteStrings( String Name, InIt Begin, InIt End );

    void WriteBinaryData( String Name, TBytes Value ) {
        if ( !CheckResult(
               ::RegSetValueEx(
                 CurrentKey, Name.c_str(), 0, REG_BINARY,
                 &Value[0], Value.Length
               )
             )
        )
        {
            throw ERegistryException(
                &_SRegSetDataFailed, ARRAYOFCONST(( Name ))
            );
        }
    }

    void WriteBinaryData( String Name, std::vector<Byte> const & Value ) {
        if ( !CheckResult(
               ::RegSetValueEx(
                 CurrentKey, Name.c_str(), 0, REG_BINARY,
                 Value.data(), Value.size()
               )
             )
        )
        {
            throw ERegistryException(
                &_SRegSetDataFailed, ARRAYOFCONST(( Name ))
            );
        }
    }

    std::tuple<TExRegDataType,size_t> GetExDataType( String Name ) {
        DWORD Type {};
        DWORD Size {};
        if ( !CheckResult(
               ::RegQueryValueEx(
                 CurrentKey, Name.c_str(), nullptr, &Type, nullptr, &Size
               )
             )
        )
        {
            throw ERegistryException(
                &_SRegGetDataFailed, ARRAYOFCONST(( Name ))
            );
        }
        switch ( Type ) {
            case REG_BINARY:
                return std::make_tuple( TExRegDataType::Binary, Size );
            case REG_DWORD:
                return std::make_tuple( TExRegDataType::Dword, Size );
            case REG_EXPAND_SZ:
                return std::make_tuple( TExRegDataType::ExpandSz, Size );
            case REG_LINK:
                return std::make_tuple( TExRegDataType::Link, Size );
            case REG_MULTI_SZ:
                return std::make_tuple( TExRegDataType::MultiSz, Size );
            case REG_NONE:
                return std::make_tuple( TExRegDataType::None, Size );
            case REG_QWORD:
                return std::make_tuple( TExRegDataType::Qword, Size );
            case REG_SZ:
                return std::make_tuple( TExRegDataType::Sz, Size );
            default:
                throw ERegistryException(
                    &_SInvalidRegType, ARRAYOFCONST(( Name ))
                );
        }
    }

    String ReadExpandString( String Name ) {
        std::array<TCHAR,32767> Buffer;

        auto const Ret = ::ExpandEnvironmentStrings(
            ReadString( Name ).c_str(),
            Buffer.data(), Buffer.size()
        );

        if ( Ret ) {
            return String( Buffer.data(), Ret );
        }

        throw ERegistryException(
            &_SRegGetDataFailed, ARRAYOFCONST(( Name ))
        );
    }
};
//---------------------------------------------------------------------------

template<typename T>
typename std::enable_if_t<std::is_integral_v<T> && sizeof( T ) == 8,T>
TRegistry::ReadQWORD( String Name )
{
    unsigned long long Data;
    DWORD Type {};
    DWORD Size { sizeof Data };
    if ( !CheckResult(
           ::RegQueryValueEx(
             CurrentKey, Name.c_str(), nullptr, &Type,
             reinterpret_cast<BYTE*>( &Data ),
             &Size
           )
         )
    )
    {
        throw ERegistryException(
            &_SRegGetDataFailed, ARRAYOFCONST(( Name ))
        );
    }
    if ( Type != REG_QWORD ) {
        throw ERegistryException(
            &_SInvalidRegType, ARRAYOFCONST(( Name ))
        );
    }

    return static_cast<T>( Data );
}
//---------------------------------------------------------------------------

template<typename OutIt, typename BT>
size_t TRegistry::ReadStringsTo( String Name, OutIt It )
{
    DWORD Type {};
    DWORD Size {};
    if ( !CheckResult(
           ::RegQueryValueEx(
             CurrentKey, Name.c_str(), nullptr, &Type, nullptr, &Size
           )
         )
    )
    {
        throw ERegistryException( &_SRegGetDataFailed, ARRAYOFCONST(( Name )) );
    }
    if ( Type != REG_MULTI_SZ ) {
        throw ERegistryException( &_SInvalidRegType, ARRAYOFCONST(( Name )) );
    }
    std::vector<TCHAR> Data( Size / sizeof( TCHAR ) );
    if ( !CheckResult(
           ::RegQueryValueEx(
             CurrentKey, Name.c_str(), nullptr, &Type,
             reinterpret_cast<LPBYTE>( Data.data() ), &Size
           )
         )
    )
    {
        throw ERegistryException( &_SRegGetDataFailed, ARRAYOFCONST(( Name )) );
    }
    // check again if the type has changed in the meantime
    if ( Type != REG_MULTI_SZ ) {
        throw ERegistryException( &_SInvalidRegType, ARRAYOFCONST(( Name )) );
    }

    // deduce il tipo di ritorno dalla funzione stessa
    decltype( ReadStringsTo( Name, It ) ) Cnt {};

    auto Start = std::begin( Data );
    auto End = std::end( Data );
    for ( auto Begin = Start ; Begin != End ; ) {
        auto OldBegin = Begin++;
        if ( !*OldBegin ) {
            if ( Begin != End ) {
                *It++ = BT{}( Start, OldBegin );
                Start = Begin;
                ++Cnt;
            }
        }
    }
    return Cnt;
}
//---------------------------------------------------------------------------

template<typename OutIt>
typename
    std::enable_if_t<
        std::is_integral_v<typename ItValueType<OutIt>::elem> &&
          sizeof( typename ItValueType<OutIt>::elem ) == 1,
        size_t
    >
TRegistry::ReadBinaryDataTo( String Name, OutIt It )
{
    DWORD Type {};
    DWORD Size {};
    if ( !CheckResult(
           ::RegQueryValueEx(
             CurrentKey, Name.c_str(), nullptr, &Type, nullptr, &Size
           )
         )
    )
    {
        throw ERegistryException( &_SRegGetDataFailed, ARRAYOFCONST(( Name )) );
    }
    if ( Type != REG_BINARY ) {
        throw ERegistryException( &_SInvalidRegType, ARRAYOFCONST(( Name )) );
    }
    std::vector<Byte> Data( Size );
    if ( !CheckResult(
           ::RegQueryValueEx(
             CurrentKey, Name.c_str(), nullptr, &Type, Data.data(), &Size
           )
         )
    )
    {
        throw ERegistryException( &_SRegGetDataFailed, ARRAYOFCONST(( Name )) );
    }
    // check again if the type has changed in the meantime
    if ( Type != REG_BINARY ) {
        throw ERegistryException( &_SInvalidRegType, ARRAYOFCONST(( Name )) );
    }

    std::copy( std::begin( Data ), std::end( Data ), It );

    return Data.size();
}
//---------------------------------------------------------------------------

template<typename InIt>
void TRegistry::WriteStrings( String Name, InIt Begin, InIt End )
{
    auto SB = std::make_unique<TStringBuilder>();
    while ( Begin != End ) {
        SB->Append( *Begin++ );
        SB->Append( _T( '\0' ) );
    }
    SB->Append( _T( '\0' ) );
    if ( !CheckResult(
           ::RegSetValueEx(
             CurrentKey, Name.c_str(), 0, REG_MULTI_SZ,
             reinterpret_cast<BYTE*>( SB->ToString().c_str() ),
             SB->Length * sizeof _T( '\0' )
           )
         )
    )
    {
        throw ERegistryException( &_SRegSetDataFailed, ARRAYOFCONST(( Name )) );
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class TConfig : public Anafestica::TConfig {
public:
	TConfig( HKEY HKey, String RootPath, bool ReadOnly = false,
             bool FlushAllItems = false )
        : Anafestica::TConfig( ReadOnly, FlushAllItems )
        , rootPath_( RootPath ) , hKey_( HKey )
    {
::OutputDebugString( _T( "CREA" ) );
        RegObjRAII Reg{ *this };
        GetRootNode().Read( *this, String() );
    }

	~TConfig() {
::OutputDebugString( _T( "DISTRUGGI" ) );
        try {
            if ( !GetReadOnlyFlag() ) {
                DoFlush();
            }
        }
        catch ( ... ) {
        }
    }

	TConfig( TConfig const & ) = delete;
	TConfig& operator=( TConfig const & ) = delete;
	//TConfig( TConfig&& ) = delete;
	//TConfig& operator=( TConfig&& ) = delete;
private:
    #if !defined(_UNICODE)
      using regex_type = std::regex;
      using cmatch_type = std::cmatch;
      using std_string = std::string;
    #else
      using regex_type = std::wregex;
      using cmatch_type = std::wcmatch;
      using std_string = std::wstring;
    #endif
protected:
	virtual TConfigNode::ValueContType DoCreateValueList( String KeyName ) override {
        regex_type re(
            _T( "" )
            "^(.*\?)(\?::(\\((\?:"
                "(i)|"  "(u)|"   "(l)|"   "(ul)|"
                "(c)|"  "(uc)|"  "(s)|"   "(us)|"
                "(ll)|" "(ull)|" "(b)|"   "(sz)|"
                "(dt)|" "(flt)|" "(dbl)|" "(cur)|"
                "(sl)|" "(sv)|"  "(dab)|" "(vb)"
            "))\\))\?$"
        );

        using RegObjType = std::remove_reference_t<decltype( *registry_ )>;

        using ValueBuilder =
            std::function<
                TConfigNodeValueType (
                    RegObjType &, // Registry Object
                    String        // Key Name
                )
            >;

        std::array<ValueBuilder,20> Builders {
            // CLASS  TAG      REG_TYPE      API
            // -----  -------  ------------  ----------------

            // i32    (i)      REG_DWORD     ReadInteger
            []( RegObjType& Reg, String KeyName ) {
                return Reg.ReadInteger( KeyName );
            },

            // u32    (u)      REG_DWORD     ReadInteger
            []( RegObjType& Reg, String KeyName ) {
                return static_cast<unsigned>( Reg.ReadInteger( KeyName ) );
            },

            // i32    (l)      REG_DWORD     ReadInteger
            []( RegObjType& Reg, String KeyName ) {
                return static_cast<long>( Reg.ReadInteger( KeyName ) );
            },

            // u32    (ul)     REG_DWORD     ReadInteger
            []( RegObjType& Reg, String KeyName ) {
                return static_cast<unsigned long>( Reg.ReadInteger( KeyName ) );
            },

            // i8     (c)      REG_DWORD     ReadInteger
            []( RegObjType& Reg, String KeyName ) {
                return static_cast<char>( Reg.ReadInteger( KeyName ) );
            },

            // u8     (uc)     REG_DWORD     ReadInteger
            []( RegObjType& Reg, String KeyName ) {
                return static_cast<unsigned char>( Reg.ReadInteger( KeyName ) );
            },

            // i16    (s)      REG_DWORD     ReadInteger
            []( RegObjType& Reg, String KeyName ) {
                return static_cast<short>( Reg.ReadInteger( KeyName ) );
            },

            // u16    (us)     REG_DWORD     ReadInteger
            []( RegObjType& Reg, String KeyName ) {
                return static_cast<unsigned short>( Reg.ReadInteger( KeyName ) );
            },

            // i64    (ll)     REG_QWORD     ReadQWORD
            []( RegObjType& Reg, String KeyName ) {
                return Reg.ReadQWORD<long long>( KeyName );
            },

            // u64    (ull)    REG_QWORD     ReadQWORD
            []( RegObjType& Reg, String KeyName ) {
                return Reg.ReadQWORD<unsigned long long>( KeyName );
            },

            // u8     (b)      REG_DWORD     ReadInteger
            []( RegObjType& Reg, String KeyName ) {
                return static_cast<bool>( Reg.ReadInteger( KeyName ) );
            },

            // sz     (sz)     REG_SZ        ReadString
            []( RegObjType& Reg, String KeyName ) {
                return Reg.ReadString( KeyName );
            },

            // b8     (dt)     REG_BINARY    ReadDateTime
            []( RegObjType& Reg, String KeyName ) {
                return Reg.ReadDateTime( KeyName );
            },

            // b8     (flt)    REG_BINARY    ReadFloat
            []( RegObjType& Reg, String KeyName ) {
                return static_cast<float>( Reg.ReadFloat( KeyName ) );
            },

            // b8     (dbl)    REG_BINARY    ReadFloat
            []( RegObjType& Reg, String KeyName ) {
                return Reg.ReadFloat( KeyName );
            },

            // b8     (cur)    REG_BINARY    ReadCurrency
            []( RegObjType& Reg, String KeyName ) {
                return Reg.ReadCurrency( KeyName );
            },

            // sl     (sl)     REG_MULTI_SZ  ReadStrings
            []( RegObjType& Reg, String KeyName ) {
                // si dovrebbe usare make_shared, ma da un AV al primo accesso
                //auto StringList = std::make_shared<TStringList>();
                auto StringList =
                    std::shared_ptr<TStringList>( new TStringList{} );
                Reg.ReadStrings( KeyName, *StringList );
                return StringList;
            },

            // sv     (sv)     REG_MULTI_SZ  ReadStringsTo
            []( RegObjType& Reg, String KeyName ) {
                std::vector<String> Strings;
                Reg.ReadStringsTo( KeyName, back_inserter( Strings ) );
                return std::move( Strings );
            },

            // dab    (dab)    REG_BINARY    ReadBinaryData
            []( RegObjType& Reg, String KeyName ) {
                return Reg.ReadBinaryData( KeyName );
            },

            // vb     (vb)     REG_BINARY    ReadBinaryDataTo
            []( RegObjType& Reg, String KeyName ) {
                auto Size = Reg.GetDataSize( KeyName );
                if ( Size < 0 ) {
                    throw Exception(
                        _T( "Registry Key %s\\%s has invalid data size" ),
                        ARRAYOFCONST((
                            Reg.CurrentPath,
                            KeyName
                        ))
                    );
                }
                auto Bytes = std::vector<Byte>( Size );
                Reg.ReadBinaryDataTo( KeyName, back_inserter( Bytes ) );
                return std::move( Bytes );
            },
        };

        struct PutItem {
            void operator()(
                TConfigNode::ValueContType& Values,
                String Name,
                TConfigNode::ValueType const & Value
            ) const {
                TConfigNode::PutItemTo(
                    Values, Name,
                    TConfigNode::ValuePairType{
                        Value, TConfigNode::Operation::None
                    }
                );
            }
        };

        TConfigNode::ValueContType Values;
        if ( OpenKeyReadOnly( KeyName ) ) {
            auto RegValues = std::make_unique<TStringList>();
            registry_->GetValueNames( RegValues.get() );
            cmatch_type ms;
            for ( auto ValueName : RegValues.get() ) {
                if ( regex_match( ValueName.c_str(), ms, re ) ) {
                    auto It = std::begin( ms );
                    std::advance( It, 1 );
                    String const Name = It->str().c_str();
                    std::advance( It, 2 );
                    It =
                        find_if(
                            It, std::end( ms ),
                            []( auto const & m ) { return m.matched; }
                        );
                    if ( It != std::end( ms ) ) {
                        auto const Idx = distance( std::begin( ms ), It ) - 3;
                        auto Val = Builders[Idx]( *registry_, ValueName );
                        PutItem{}( Values, Name, Val );
                    }
                    else {
                        auto [Type,Size] = registry_->GetExDataType( ValueName );
                        switch ( Type ) {
                            case TExRegDataType::Binary:
                                PutItem{}( Values, Name, registry_->ReadBinaryData( ValueName ) );
                                break;
                            case TExRegDataType::Dword:
                                PutItem{}( Values, Name, registry_->ReadInteger( ValueName ) );
                                break;
                            case TExRegDataType::MultiSz: {
                                    // si dovrebbe usare make_shared, ma da un AV al primo accesso
                                    //auto StringList = std::make_shared<TStringList>();
                                    auto StringList = std::shared_ptr<TStringList>( new TStringList() );
                                    registry_->ReadStrings( ValueName, *StringList );
                                    PutItem{}( Values, Name, StringList );
                                }
                                break;
                            case TExRegDataType::Qword:
                                PutItem{}(
                                    Values, Name,
                                    registry_->ReadQWORD<long long>( ValueName )
                                );
                                break;
                            case TExRegDataType::Sz:
                                PutItem{}( Values, Name, registry_->ReadString( ValueName ) );
                                break;
                            case TExRegDataType::ExpandSz:
                                PutItem{}( Values, Name, registry_->ReadExpandString( ValueName ) );
                                break;
                            default:
                                throw Exception(
                                    _T( "Registry Key %s\\%s has an invalid data type" ),
                                    ARRAYOFCONST((
                                        registry_->CurrentPath,
                                        ValueName
                                    ))
                                );
                        }
                    }
                }
            }
        }

        return Values;
    }

	virtual TConfigNode::NodeContType DoCreateNodeList( String KeyName ) override {
        TConfigNode::NodeContType Nodes;

        if ( OpenKeyReadOnly( KeyName ) ) {
            auto RegKeys = std::make_unique<TStringList>();
            registry_->GetKeyNames( RegKeys.get() );
            for ( auto const & Key : RegKeys.get() ) {
                Nodes[Key] = std::move( std::make_unique<TConfigNode>() );
            }
        }
        return Nodes;
    }

	virtual void DoSaveValueList( String KeyName, TConfigNode::ValueContType const & Values ) override {
        if ( !Values.empty() ) {
            if ( OpenKey( KeyName, true ) ) {
                for ( auto& v : Values ) {
                    auto const ValueState = v.second.second;
                    if ( GetAlwaysFlushNodeFlag() || ValueState == TConfigNode::Operation::Write ) {
                        SaveValue( v );
                    }
                    else if ( v.second.second == TConfigNode::Operation::Erase ) {
                        DeleteValue( v );
                    }
                }
            }
            else {
                throw ERegistryException(
                    _T( "Error in TConfigRegistry::DoSaveValueList: can't open key for writing" )
                );
            }
        }
    }

	virtual void DoSaveNodeList( String KeyName, TConfigNode::NodeContType const & Nodes ) override {
        for ( auto const & n : Nodes ) {
            if ( GetAlwaysFlushNodeFlag() || n.second->IsModified() ) {
                n.second->Write(
                    *this,
                    Format(
                        _T( "%s\\%s" ), ARRAYOFCONST(( KeyName, n.first ))
                    )
                );
            }
        }
    }

	virtual void DoDeleteNode( String KeyName ) override { DeleteKey( KeyName ); }

	virtual void DoFlush() override {
        RegObjRAII Reg{ *this };
        GetRootNode().Write( *this, String() );
    }

	virtual bool DoGetForcedWritesFlag() const { return false; }
private:
	String rootPath_;
	HKEY hKey_;
	std::unique_ptr<TRegistry> registry_;

	void CreateRegistryObject() {
        registry_ =
            std::move( std::make_unique<Anafestica::Registry::TRegistry>() );
        registry_->RootKey = hKey_;
    }

	void DestroyAndCloseRegistryObject() {
        if ( !registry_->CurrentPath.IsEmpty() ) {
            registry_->CloseKey();
        }
        registry_.reset();
    }

	bool OpenKey( String Path, bool CanCreate = false ) {
        String const Key = ExcludeTrailingBackslash( rootPath_ + Path );
        if ( registry_->CurrentPath != Key ) {
            if ( !registry_->CurrentPath.IsEmpty() ) {
                registry_->CloseKey();
            }
            return registry_->OpenKey( Key, CanCreate );
        }
        return true;
    }

	bool OpenKeyReadOnly( String Path ) {
        String const Key = ExcludeTrailingBackslash( rootPath_ + Path );
        if ( registry_->CurrentPath != Key ) {
            if ( !registry_->CurrentPath.IsEmpty() ) {
                registry_->CloseKey();
            }
            return registry_->OpenKeyReadOnly( Key );
        }
        return true;
    }

	void SaveValue( TConfigNode::ValueContType::value_type const & v ) {
        boost::apply_visitor(
            SaveVisitor{ *registry_, v.first }, v.second.first
        );
    }

	template<typename PairType>
	void DeleteValue( PairType const & v ) { registry_->DeleteValue( v.first ); }

	void DeleteKey( String Path ) {
        auto const Key = ExcludeTrailingBackslash( rootPath_ + Path );
        if ( !registry_->CurrentPath.IsEmpty() ) {
            if ( registry_->CurrentPath == Key ) {
                registry_->CloseKey();
            }
        }
        registry_->DeleteKey( Key );
    }

    class RegObjRAII {
    public:
        RegObjRAII( TConfig& Cfg ) : cfg_{ Cfg } { Cfg.CreateRegistryObject(); }
        ~RegObjRAII() noexcept {
            try { cfg_.DestroyAndCloseRegistryObject(); } catch ( ... ) {}
        }
        RegObjRAII( RegObjRAII const & ) = delete;
        RegObjRAII& operator=( RegObjRAII const & ) = delete;
    private:
        TConfig& cfg_;
    };

    class SaveVisitor : public boost::static_visitor<void> {
    public:
        SaveVisitor( TRegistry& Reg, String Name )
            : reg_{ Reg }, name_{ Name } {}
        SaveVisitor( SaveVisitor const & ) = delete;
        SaveVisitor& operator=( SaveVisitor const & ) = delete;

        // REG_BINARY    - Binary data in any form.
        // REG_DWORD     - 32-bit number.
        // REG_QWORD     - 64-bit number.
        // REG_EXPAND_SZ - Null-terminated string that contains unexpanded references to environment variables
        // REG_MULTI_SZ  - Array of null-terminated strings that are terminated by two null characters.
        // REG_SZ        - Null-terminated string.
                                                                  // CLASS  TAG    REG_TYPE      API
                                                                  // -----  -----  ------------  ----------------

        void operator()( int Val ) const {                        // i32           REG_DWORD     WriteInteger
            reg_.WriteInteger( name_, Val );
        }

        void operator()( unsigned int Val ) const {               // u32    (u)    REG_DWORD     WriteInteger
            reg_.WriteInteger(
                Format( _T( "%s:(u)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( long Val ) const {                       // i32    (l)    REG_DWORD     WriteInteger
            reg_.WriteInteger(
                Format( _T( "%s:(l)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( unsigned long Val ) const {              // u32    (ul)   REG_DWORD     WriteInteger
            reg_.WriteInteger(
                Format( _T( "%s:(ul)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( char Val ) const {                       // i8     (c)    REG_DWORD     WriteInteger
            reg_.WriteInteger(
                Format( _T( "%s:(c)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( unsigned char Val ) const {              // u8     (uc)   REG_DWORD     WriteInteger
            reg_.WriteInteger(
                Format( _T( "%s:(uc)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( short Val ) const {                      // i16    (s)    REG_DWORD     WriteInteger
            reg_.WriteInteger(
                Format( _T( "%s:(s)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( unsigned short Val ) const {             // u16    (us)   REG_DWORD     WriteInteger
            reg_.WriteInteger(
                Format( _T( "%s:(us)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( long long Val ) const {                  // i64           REG_QWORD     WriteQWORD
            reg_.WriteQWORD( name_, Val );
        }

        void operator()( unsigned long long Val ) const {         // u64    (ull)  REG_QWORD     WriteQWORD
            reg_.WriteQWORD(
                Format( _T( "%s:(ull)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( bool Val ) const {                       // u8     (b)    REG_DWORD     WriteInteger
            reg_.WriteInteger(
                Format( _T( "%s:(b)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( System::UnicodeString Val ) const {      // sz            REG_SZ        WriteString
            //reg_.WriteString(
            //  Format( _T( "%s:(sz)" ), ARRAYOFCONST(( name_ )) ), Val
            //);
            reg_.WriteString( name_, Val );
        }

        void operator()( System::TDateTime Val ) const {          // b8     (dt)   REG_BINARY    WriteDateTime
            reg_.WriteDateTime(
                Format( _T( "%s:(dt)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( float Val ) const {                      // b8     (flt)  REG_BINARY    WriteFloat
            reg_.WriteFloat(
                Format( _T( "%s:(flt)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( double Val ) const {                     // b8     (dbl)  REG_BINARY    WriteFloat
            reg_.WriteFloat(
                Format( _T( "%s:(dbl)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( System::Currency Val ) const {           // b8     (cur)  REG_BINARY    WriteCurrency
            reg_.WriteCurrency(
                Format( _T( "%s:(cur)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( std::shared_ptr<TStrings> Val ) const {  // sl            REG_MULTI_SZ  WriteStrings
            reg_.WriteStrings( name_, *Val );
        }

        void operator()( std::vector<String> const &Val ) const { // sv     (sv)   REG_MULTI_SZ  WriteStrings
            reg_.WriteStrings(
                Format( _T( "%s:(sv)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

        void operator()( TBytes Val ) const {                     // dab           REG_BINARY    WriteBinaryData
            reg_.WriteBinaryData( name_, Val );
        }

        void operator()( std::vector<Byte> const & Val ) const {  // vb     (vb)   REG_BINARY    WriteBinaryData
            reg_.WriteBinaryData(
                Format( _T( "%s:(vb)" ), ARRAYOFCONST(( name_ )) ), Val
            );
        }

    private:
        Anafestica::Registry::TRegistry& reg_;
        String name_;
    };

};

//---------------------------------------------------------------------------
} // End namespace Registry
//---------------------------------------------------------------------------
} // End of namespace Anafestica
//---------------------------------------------------------------------------

#endif
