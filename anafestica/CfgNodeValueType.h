//---------------------------------------------------------------------------

#ifndef CfgNodeValueTypeH
#define CfgNodeValueTypeH

#include <System.Classes.hpp>

#include <memory>
#include <vector>

#include <boost/variant.hpp>

//---------------------------------------------------------------------------
namespace Anafestica {
//---------------------------------------------------------------------------

using TConfigNodeValueType =
    boost::variant<
        int
      , unsigned int
      , long
      , unsigned long
      , char
      , unsigned char
      , short
      , unsigned short
      , long long
      , unsigned long long
      , bool
      , System::UnicodeString
      , System::TDateTime
      , float
      , double
      , System::Currency
      , std::shared_ptr<TStrings>
      , std::vector<String>
      , TBytes
      , std::vector<Byte>
    >;

//---------------------------------------------------------------------------
} // End of namespace Anafestica
//---------------------------------------------------------------------------
#endif
