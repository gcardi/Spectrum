//---------------------------------------------------------------------------

#ifndef CfgH
#define CfgH

#include <memory>

#include <anafestica/CfgItems.h>

//---------------------------------------------------------------------------
namespace Anafestica {
//---------------------------------------------------------------------------

class TConfig {
public:
	TConfig( bool ReadOnly, bool FlushAllItems )
      : readOnly_{ ReadOnly }
      , flushAllItems_{ FlushAllItems }
      , root_( new TConfigNode )
    {}
    TConfigNode& GetRootNode() { return DoGetRootNode(); }
    void Flush() { DoFlush(); }
    TConfigNode::ValueContType CreateValueList( String KeyName ) {
        return DoCreateValueList( KeyName );
    }

    TConfigNode::NodeContType CreateNodeList( String KeyName ) {
        return DoCreateNodeList( KeyName );
    }

    void SaveValueList( String KeyName, TConfigNode::ValueContType const & Values ) {
        DoSaveValueList( KeyName, Values );
    }

    void SaveNodeList( String KeyName, TConfigNode::NodeContType const & Nodes ) {
        DoSaveNodeList( KeyName, Nodes );
    }

    void DeleteNode( String KeyName ) { DoDeleteNode( KeyName ); }

    bool GetReadOnlyFlag() const noexcept { return readOnly_; }
    bool GetAlwaysFlushNodeFlag() const noexcept { return flushAllItems_; }
protected:
    virtual TConfigNode::ValueContType DoCreateValueList( String KeyName ) = 0;
    virtual TConfigNode::NodeContType DoCreateNodeList( String KeyName ) = 0;
    virtual void DoSaveValueList( String KeyName, TConfigNode::ValueContType const & Values ) = 0;
    virtual void DoSaveNodeList( String KeyName, TConfigNode::NodeContType const & Nodes ) = 0;
    virtual TConfigNode& DoGetRootNode() { return *root_; }
    virtual void DoDeleteNode( String KeyName ) = 0;
    virtual void DoFlush() = 0;
private:
	using TConfigNodePtr = std::unique_ptr<TConfigNode>;

    bool readOnly_ {};
    bool flushAllItems_ {};
	TConfigNodePtr root_;
};

//---------------------------------------------------------------------------
} // End of namespace Anafestica
//---------------------------------------------------------------------------

#endif
