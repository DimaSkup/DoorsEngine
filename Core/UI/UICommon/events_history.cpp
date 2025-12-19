// =================================================================================
// Filename:      EventsHistory.cpp
// 
// Created:       08.02.25  by DimaSkup
// =================================================================================
#include "events_history.h"
#include "editor_cmd.h"

namespace UI
{

// --------------------------------------------------------
// init global instance of the EventsHistory container
// --------------------------------------------------------
    EventsHistory g_EventsHistory;

//---------------------------------------------------------
// Desc:  while we're doing some stuff in editor (using mouse or keyboard)
//        we push items into the temporal history
//        so after we release mouse buttons, or keyboard keys
//        we will calculate the final history item and store it
//        SO then we will be able to properly "undo" this event
//---------------------------------------------------------
void EventsHistory::Push(
    const ICommand cmd,
    const std::string& msg,
    const uint32_t entityID)
{
	tempHistory_.push_back(HistoryItem(cmd, msg, entityID));
}

//---------------------------------------------------------
// Desc:  handle changing of entity rotation separately
//        (yes, I know this is a shit, but it works so I don't care)
//---------------------------------------------------------
HistoryItem HandleRotation(const std::deque<UI::HistoryItem>& tempHistory)
{
    DirectX::XMVECTOR finalQuat{ 0,0,0,1 };

    // accumulate all the rotation quaternions from the temp history
    // so we will get the final transformation
    for (auto it = tempHistory.begin(); it != tempHistory.end(); ++it)
    {
        const Vec4 q = it->cmd_.GetVec4();
        finalQuat = DirectX::XMQuaternionMultiply(finalQuat, { q.x, q.y, q.z, q.w });
    }

    // final rotation quaternion as Vec4
    Vec4 rotQuat = {
        DirectX::XMVectorGetX(finalQuat),
        DirectX::XMVectorGetY(finalQuat),
        DirectX::XMVectorGetZ(finalQuat),
        DirectX::XMVectorGetW(finalQuat)
    };

    const HistoryItem& item = tempHistory.front();
    const CmdChangeVec4 undoRotCmd(CHANGE_ENTITY_ROTATION, rotQuat);

    return HistoryItem(undoRotCmd, item.msg_, item.entityID_);
}

//---------------------------------------------------------
// Desc:  create an actual history item after we release mouse or keyboard
//---------------------------------------------------------
void EventsHistory::FlushTempHistory()
{
    if (tempHistory_.empty())
        return;

	HistoryItem forward;   // alternative undo
	HistoryItem finalUndoItem;      // classis undo (ctrl+z)

	if (cmdHistoryIdx_ > 64)
	{
		--cmdHistoryIdx_;
		history_.pop_front();	
	}


    // handle changing of entity rotation separately
    // (yes, I know this is a shit, but it works so I don't care)
    if (tempHistory_.front().cmd_.type_ == CHANGE_ENTITY_ROTATION)
    {
        finalUndoItem = HandleRotation(tempHistory_);
    }
    else
    {
        finalUndoItem = tempHistory_.front();
    }


    history_.push_back(finalUndoItem);
	tempHistory_.clear();         
	++cmdHistoryIdx_;
}

///////////////////////////////////////////////////////////

HistoryItem EventsHistory::Undo()
{
	if (!history_.empty())
	{
		//HistoryItem item = history_[cmdHistoryIdx_ - 1];
		HistoryItem item = history_.back();
		history_.pop_back();
		--cmdHistoryIdx_;

		return item;
	}
	
	return HistoryItem(ICommand(), "", 0);
}

} // namespace UI
