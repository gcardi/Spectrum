//---------------------------------------------------------------------------

#ifndef SysUt_FmtH
#define SysUt_FmtH

#include <utility>
#include <array>
#include <type_traits>
#include <tuple>
#include <iterator>

#include <System.SysUtils.hpp>

//---------------------------------------------------------------------------

namespace SysUt {

template<typename T>
struct TypeIsFloatOrDouble {
    using Type = std::remove_cv_t<std::remove_reference_t<T>>;
    static constexpr bool value =
        std::is_same_v<Type,double> || std::is_same_v<Type,float>;
};

template<bool> struct float_tag {};

using is_not_float_tag = float_tag<false>;
using is_float_tag = float_tag<true>;

template<typename IV, typename IF, typename T>
inline IF AppendPar( is_not_float_tag, IV ItPars, IF ItFlt, T&& Arg )
{
    *ItPars = std::forward<T>( Arg );
    return ItFlt;
}

template<typename IV, typename IF>
inline IF AppendPar( is_float_tag, IV ItPars, IF ItFlt, double Arg )
{
    *ItFlt = Arg;
    *ItPars = *ItFlt;
    std::advance( ItFlt, 1 );
    return ItFlt;
}

template<typename IV, typename IF, typename T>
inline IF PopulatePars( IV ItPars, IF ItFlt, T&& Arg )
{
    return AppendPar(
        float_tag<TypeIsFloatOrDouble<T>::value>{},
        ItPars, ItFlt, std::forward<T>( Arg )
    );
}

template<typename IV, typename IF, typename T, typename...A>
inline IF PopulatePars( IV ItPars, IF ItFlt, T&& Arg, A&&...Args )
{
    ItFlt =
        AppendPar(
            float_tag<TypeIsFloatOrDouble<T>::value>{},
            ItPars++, ItFlt, std::forward<T>( Arg )
        );
    return PopulatePars( ItPars, ItFlt, std::forward<A>( Args )... );
}

template<typename T, size_t N>
struct CountFloatingPointTypesHelper {
    static constexpr size_t value =
        CountFloatingPointTypesHelper<T,N-1>::value +
        ( TypeIsFloatOrDouble<std::tuple_element_t<N,T>>::value ? 1 : 0 );
};

template<typename T>
struct CountFloatingPointTypesHelper<T,0> {
    static constexpr size_t value =
        TypeIsFloatOrDouble<std::tuple_element_t<0,T>>::value ? 1 : 0;
};

template<typename...A>
struct CountFloatingPointTypes {
    static constexpr size_t value =
        CountFloatingPointTypesHelper<
            std::tuple<A...>,
            sizeof...( A ) - 1
        >::value;
};

template<typename...A>
inline String Fmt( String FmtStr, A&&...Args )
{
    std::array<TVarRec,sizeof...( A )> Pars;
    std::array<long double,CountFloatingPointTypes<A...>::value> Flts;

    PopulatePars( std::begin( Pars ), std::begin( Flts ), std::forward<A>( Args )... );
    return System::Sysutils::Format( FmtStr, Pars.data(), Pars.size() - 1 );
}

template<typename...A>
inline void OutputDebugString( String FmtStr, A&&...Args )
{
    ::OutputDebugString( Fmt( FmtStr, std::forward<A>( Args )... ).c_str() );
}

} // End of namespace SysUt

//---------------------------------------------------------------------------
#endif

