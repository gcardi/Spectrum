//---------------------------------------------------------------------------

#ifndef CfgItemsH
#define CfgItemsH

#include <System.Classes.hpp>

#include <utility>
#include <memory>
#include <algorithm>
#include <map>

#include <anafestica/CfgNodeValueType.h>

//---------------------------------------------------------------------------
namespace Anafestica {
//---------------------------------------------------------------------------

class TConfigNode
{
public:
	TConfigNode() = default;
    TConfigNode( TConfigNode const & ) = delete;
    TConfigNode& operator=( TConfigNode const & ) = delete;

	enum class Operation { None, Write, Erase };

	using KeyType = String;

	using ValueType = TConfigNodeValueType;

	using ValuePairType = std::pair<ValueType,Operation>;
	using TConfigNodePtr = std::unique_ptr<TConfigNode>;

	using ValueContType = std::map<KeyType,ValuePairType>;
	using NodeContType = std::map<KeyType,TConfigNodePtr>;

    TConfigNode& GetSubNode( String Id ) {
        auto p =
            nodeItems_.insert( NodeContType::value_type( Id, TConfigNodePtr{} ) );
        if ( p.second ) {
            p.first->second = std::move( std::make_unique<TConfigNode>() );
        }
        return *p.first->second;
    }

    template<typename R>
    void Read( R& Reader, String Id );

    template<typename W>
    void Write( W& Writer, String Id ) const;

    static ValueType GetItemFrom( ValueContType& Values, String Id,
                                  ValueType DefVal, Operation Op )
    {
        auto r =
            Values.insert(
                std::make_pair( Id, std::make_pair( DefVal, Op ) )
            );
        return r.second ? DefVal : r.first->second.first;
    }

    template<typename T>
    ValueType GetItem( String Id, T&& DefVal, Operation Op = Operation::None ) {
        return GetItemFrom( valueItems_, Id, std::forward<T>( DefVal ), Op );
    }

    template<typename T>
    void GetItemAs( String Id, T& Val, Operation Op = Operation::None ) {
        GetItemAs(
            enum_tag<std::is_enum_v<std::remove_reference_t<decltype( Val )>>>{},
            Id, Val, Op
        );
    }

    template<typename T>
    bool PutItem( String Id, T const & Val, Operation Op = Operation::Write ) {
        return PutItem(
            Id, Val,
            enum_tag<std::is_enum_v<std::remove_reference_t<decltype( Val )>>>{},
            Op
        );
    }

    static bool PutItemTo( ValueContType& Values, String Id,
                           ValuePairType const & Val ) noexcept
    {
        auto r = Values.insert( make_pair( Id, Val ) );
        if ( !r.second ) {
            if ( r.first->second.first != Val.first ) {
                r.first->second = ValuePairType( Val.first, Operation::Write );
            }
        }
        return r.second;
    }

    size_t GetNodeCount() const noexcept { return nodeItems_.size(); }

	template<typename OutputIterator>
    void EnumerateNodes( OutputIterator Output ) const;

    size_t GetValueCount() const noexcept {
        return std::count_if(
            std::begin( valueItems_ ), std::end( valueItems_ ),
            []( auto const & Val ) {
                return Val.second.second != Operation::Erase;
            }
        );
    }

    template<typename OutputIterator>
    void EnumeratePairs( OutputIterator Out ) const;

    template<typename OutputIterator>
    void EnumerateValueNames( OutputIterator Out ) const;

    template<typename OutputIterator>
    void EnumerateValues( OutputIterator Out ) const;

    void DeleteItem( String Id ) {
        auto i = valueItems_.find( Id );
        if ( i != std::end( valueItems_ ) ) {
            i->second.second = Operation::Erase;
        }
    }

    void DeleteSubNode( String Id ) {
        auto i = nodeItems_.find( Id );
//        if ( i != std::end( nodeItems_ ) ) { MarkNodeDeleted( *i ); }
        if ( i != std::end( nodeItems_ ) ) { i->second->Clear(); }
    }

    bool IsDeleted() const noexcept { return deleted_; }

    bool IsModified() const noexcept {
        return IsDeleted() || ValueListModified() || NodesModified();
    }

    void Clear() {
        deleted_ = true;
        valueItems_.clear();
        for ( auto& v : nodeItems_ ) { v.second->Clear(); }
    }

    bool ItemExists( String Id ) const noexcept {
        return valueItems_.find( Id ) != std::end( valueItems_ );
    }

    bool SubNodeExists( String Id ) const noexcept {
        return nodeItems_.find( Id ) != std::end( nodeItems_ );
    }

private:
    ValueContType valueItems_;
    NodeContType nodeItems_;
    bool deleted_ {};

    bool ValueListModified() const noexcept {
        return
            std::find_if(
                std::begin( valueItems_ ), std::end( valueItems_ ),
                []( auto const & v ) {
                    switch ( v.second.second ) {
                        case Operation::Write:
                        case Operation::Erase:
                            return true;
                        default:
                            return false;
                    }
                }
            ) != std::end( valueItems_ );
    }

    bool NodesModified() const noexcept {
        return
            std::find_if(
                std::begin( nodeItems_ ), std::end( nodeItems_ ),
                []( auto const & n ) { return n.second->IsModified(); }
            ) != std::end( nodeItems_ );
    }

    static bool IsValueDeleted( ValueContType::value_type const & Val ) noexcept {
        return Val.second.second == Operation::Erase;
    }

    template<bool> struct enum_tag {};

    using is_not_enum_tag = enum_tag<false>;
    using is_enum_tag = enum_tag<true>;

    template<typename T>
    void GetItemAs( is_not_enum_tag, String Id, T& Val, Operation Op ) {
        Val = boost::get<std::remove_reference_t<T>>( GetItem( Id, Val, Op ) );
    }

    template<typename T>
    void GetItemAs( is_enum_tag, String Id, T& Val, Operation Op );

    template<typename T>
    bool PutItem( String Id, T&& Val, is_not_enum_tag, Operation Op = Operation::Write ) noexcept {
        return PutItemTo( valueItems_, Id, make_pair( ValueType{ std::forward<T>( Val ) }, Op ) );
    }

    template<typename T>
    bool PutItem( String Id, T Val, is_enum_tag, Operation Op = Operation::Write ) noexcept
    {
        if ( auto Info = __delphirtti( decltype( Val ) ) ) {
            // save as text
            return PutItemTo(
                valueItems_, Id,
                std::make_pair(
                    ValueType{ GetEnumName( Info, static_cast<int>( Val ) ) },
                    Op
                )
            );
        }
        else {
            // save as integer
            return PutItemTo(
                valueItems_, Id,
                std::make_pair( ValueType{ static_cast<int>( Val ) }, Op )
            );
        }
    }
};
//---------------------------------------------------------------------------

template<typename T>
void TConfigNode::GetItemAs( is_enum_tag, String Id, T& Val, Operation Op )
{
    if ( auto Info = __delphirtti( decltype( Val ) ) ) {
        Val =
            static_cast<T>(
                GetEnumValue(
                    Info,
                    boost::get<String>(
                        GetItem(
                            Id,
                            GetEnumName( Info, static_cast<int>( Val ) ),
                            Op
                        )
                    )
                )
            );
    }
    else {
        Val =
            static_cast<T>(
                boost::get<int>( GetItem( Id, static_cast<int>( Val ), Op ) )
            );
    }
}
//---------------------------------------------------------------------------

template<typename OutputIterator>
inline void TConfigNode::EnumerateNodes( OutputIterator Out ) const
{
    std::transform(
        std::begin( nodeItems_ ), std::end( nodeItems_ ), Out,
        []( auto const & v ){ return v.first; }
    );
}
//---------------------------------------------------------------------------

template<typename OutputIterator>
void TConfigNode::EnumerateValueNames( OutputIterator Out ) const
{
    std::for_each(
        std::begin( valueItems_ ), std::end( valueItems_ ),
        [&Out]( auto const & Val ) {
            if ( Val.second.second != Operation::Erase ) { *Out++ = Val.first; }
        }
    );
}
//---------------------------------------------------------------------------

template<typename OutputIterator>
void TConfigNode::EnumerateValues( OutputIterator Out ) const
{
    std::for_each(
        std::begin( valueItems_ ), std::end( valueItems_ ),
        [&Out]( auto const & Val ) {
            if ( Val.second.second != Operation::Erase ) {
                *Out++ = std::make_pair( Val.first, Val.second.first );
            }
        }
    );
}
//---------------------------------------------------------------------------

template<typename R>
void TConfigNode::Read( R& Reader, String Id )
{
    valueItems_ = Reader.CreateValueList( Id );
    nodeItems_ = Reader.CreateNodeList( Id );
    for ( auto& n : nodeItems_ ) {
        n.second->Read( Reader, Id + '\\' + n.first );
    }
}
//---------------------------------------------------------------------------

template<typename W>
void TConfigNode::Write( W& Writer, String Id ) const
{
    if ( IsDeleted() ) {
        Writer.DeleteNode( Id );
    }
    if ( Writer.GetAlwaysFlushNodeFlag() || ValueListModified() ) {
        Writer.SaveValueList( Id, valueItems_ );
    }
    Writer.SaveNodeList( Id, nodeItems_ );
}

//---------------------------------------------------------------------------
} // End of namespace Anafestica
//---------------------------------------------------------------------------

#define RESTORE_PROPERTY( NODE, PROPERTY ) \
{\
    std::remove_reference_t< decltype( PROPERTY )> Tmp{ PROPERTY }; \
    ( NODE ).GetItemAs( #PROPERTY, Tmp ); \
    PROPERTY = Tmp; \
}

#define SAVE_PROPERTY( NODE, PROPERTY ) \
    ( NODE ).PutItem( #PROPERTY, PROPERTY )

//---------------------------------------------------------------------------

#define RESTORE_VALUE( NODE, KEY_NAME, VALUE ) \
{\
    std::remove_reference_t< decltype( VALUE )> Tmp{ VALUE }; \
    ( NODE ).GetItemAs( ( KEY_NAME ), Tmp ); \
    VALUE = Tmp; \
}

#define SAVE_VALUE( NODE, KEY_NAME, VALUE ) \
    ( NODE ).PutItem( ( KEY_NAME ), ( VALUE ) )

//---------------------------------------------------------------------------

#endif

