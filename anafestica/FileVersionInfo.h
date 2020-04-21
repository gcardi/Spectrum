//---------------------------------------------------------------------------

#ifndef FileVersionInfoH
#define FileVersionInfoH

#include <vector>

#include <System.hpp>
#include <System.SysUtils.hpp>

//---------------------------------------------------------------------------
namespace Anafestica {
//---------------------------------------------------------------------------

class TFileVersionInfo {
public:
    explicit TFileVersionInfo( String FileName ) : info_( GetInfoSize( FileName ) ) {
        if ( !GetFileVersionInfo( FileName.c_str(), 0, info_.size(), &info_[0] ) ) {
            RaiseLastOSError();
        }
        langCharset_ = GetLanguageCharset();
    }

    __property String CompanyName = { read = GetCompanyName };
    __property String ProductName = { read = GetProductName };
    __property String ProductVersion = { read = GetProductVersion };
    __property String FileDescription = { read = GetFileDescription };
    __property String FileVersion = { read = GetFileVersion };
    __property String InternalName = { read = GetInternalName };
    __property String LegalCopyright = { read = GetLegalCopyright };
    __property String OriginalFilename = { read = GetOriginalFilename };
    __property String Comments = { read = GetComments };
private:
    std::vector<char> info_;
    String langCharset_;
    //static String const StringFileInfo;

    DWORD GetInfoSize( String FileName ) const {
        DWORD Handle;
        auto const Size = GetFileVersionInfoSize( FileName.c_str(), &Handle );
        if ( !Size ) {
            RaiseLastOSError();
        }
        return Size;
    }

    WORD const * GetTranslationTable() const {
        UINT Len;
        LPVOID Ptr;

        if ( !VerQueryValue(
                (LPVOID)&info_[0], _T( "\\VarFileInfo\\Translation" ),
                &Ptr, &Len
             )
        ) {
            RaiseLastOSError();
        }
        return static_cast<WORD*>( Ptr );
    }

    String GetLanguageCharset() const {
        auto const TranslationTable = GetTranslationTable();
        if ( !TranslationTable ) {
            throw Exception(
                _T( "Translation table not found (Version Info)" )
            );
        }
        return IntToHex( TranslationTable[0], 4 ) +
               IntToHex( TranslationTable[1] ,4 );
    }

    String GetValue( String SubBlockId ) const {
        auto SubBlock =
            Format(
                _T( "\\StringFileInfo\\%s\\%s" ),
                ARRAYOFCONST(( langCharset_, SubBlockId ))
            );

        UINT Len;
        LPVOID Ptr;

        if ( !VerQueryValue( (LPVOID)&info_[0], SubBlock.c_str(), &Ptr, &Len ) ) {
           RaiseLastOSError();
        }
        return String( static_cast<LPTSTR>( Ptr ), Len - 1 );
    }

    String GetCompanyName() const { return GetValue( _T( "CompanyName" ) ); }
    String GetProductName() const { return GetValue( _T( "ProductName" ) ); }
    String GetProductVersion() const { return GetValue( _T( "ProductVersion" ) ); }
    String GetFileDescription() const { return GetValue( _T( "FileDescription" ) ); }
    String GetFileVersion() const { return GetValue( _T( "FileVersion" ) ); }
    String GetInternalName() const { return GetValue( _T( "InternalName" ) ); }
    String GetLegalCopyright() const { return GetValue( _T( "LegalCopyright" ) ); }
    String GetOriginalFilename() const { return GetValue( _T( "OriginalFilename" ) ); }
    String GetComments() const { return GetValue( _T( "Comments" ) ); }
};

//---------------------------------------------------------------------------
} // End of namespace Anafestica
//---------------------------------------------------------------------------

#endif



