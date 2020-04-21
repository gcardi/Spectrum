//---------------------------------------------------------------------------

#ifndef CfgRegistrySingletonH
#define CfgRegistrySingletonH

#include <System.hpp>
#include <System.SysUtils.hpp>

#include <memory>

#include <anafestica/FileVersionInfo.h>
#include <anafestica/CfgRegistry.h>

//---------------------------------------------------------------------------
namespace Anafestica {
//---------------------------------------------------------------------------
namespace Registry {
//---------------------------------------------------------------------------

class TConfigSingleton {
public:
	Anafestica::TConfig& GetConfig() {
        static auto Cfg =
            std::make_unique<TConfig>(
                HKEY_CURRENT_USER,
                Format(
                    _T( "Software\\%s" ),
                    ARRAYOFCONST(( GetProductPath() ))
                )
            );

        return *Cfg;
    }
private:
	static String GetProductPath() {
        TFileVersionInfo const Info( ParamStr( 0 ) );

        return Format(
            _T( "%s\\%s\\%s" ),
            ARRAYOFCONST( (
                Info.CompanyName,
                Info.ProductName,
                Info.ProductVersion
            ) )
        );
    }
};

//---------------------------------------------------------------------------
} // End of namespace Registry
//---------------------------------------------------------------------------

using TConfigRegistrySingleton = Registry::TConfigSingleton;

//---------------------------------------------------------------------------
} // End of namespace Anafestica
//---------------------------------------------------------------------------

#endif
