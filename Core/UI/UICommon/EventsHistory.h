// =================================================================================
// Filename:      EventsHistory.h
// Description:   a container for editor events;
//                represents a history of events, stores commands which
//                were executed, and also stores a reverse commands for UNDO
// 
// Created:       08.02.25  by DimaSkup
// =================================================================================
#pragma once

#include "ICommand.h"
#include <string>
#include <deque>


namespace UI
{

struct HistoryCmdPair
{
	// a pair of commands: undo (ctrl+Z)
	// and alternative undo (alt+Z) to move forward in history
};

///////////////////////////////////////////////////////////

struct HistoryItem
{
	HistoryItem() {};

	HistoryItem(
		const ICommand cmd, 
		const std::string& msg, 
		const uint32_t entityID) 
		:
		cmd_(cmd),
		msg_(msg),
		entityID_(entityID) {}

	ICommand cmd_;
	std::string msg_;
	uint32_t entityID_;
};

///////////////////////////////////////////////////////////

struct EventsHistory
{
	// int historyLimit = 64;
	int cmdHistoryIdx_ = 0;     // idx of out current position in history
	std::deque<HistoryItem> history_;


	// temporal history: when we hold down a mouse button we don't want to save
	//                   any micro-changes so we store these changes into the temp
	//                   history and then sum up all these changes to create
	//                   a true history event
	std::deque<HistoryItem> tempHistory_;


	void Push(const ICommand cmd, const std::string& msg, const uint32_t entityID);
	void FlushTempHistory();
	HistoryItem Undo();

	inline bool HasHistory()     const { return !history_.empty(); }
	inline bool HasTempHistory() const { return !tempHistory_.empty(); }
};


extern EventsHistory gEventsHistory;

} // namespace UI