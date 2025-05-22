// =================================================================================
// Filename:      EventsHistory.cpp
// 
// Created:       08.02.25  by DimaSkup
// =================================================================================
#include "EventsHistory.h"


namespace UI
{

void EventsHistory::Push(const ICommand cmd, const std::string& msg, const uint32_t entityID)
{
	tempHistory_.push_back(HistoryItem(cmd, msg, entityID));
}

///////////////////////////////////////////////////////////

void EventsHistory::FlushTempHistory()
{
	// create an actual history item after we release mouse or keyboard

    if (tempHistory_.empty())
        return;

	HistoryItem forward;   // alternative undo
	HistoryItem undo;      // classis undo (ctrl+z)

	if (cmdHistoryIdx_ > 64)
	{
		--cmdHistoryIdx_;
		history_.pop_front();	
	}

	undo = tempHistory_.front();         // starting value
	forward = tempHistory_.back();       // final change

	history_.push_back(undo);
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
