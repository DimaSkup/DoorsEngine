#pragma once

#include "../Common/Assert.h"

namespace ECS
{

struct DataHeaderRecord
{
	u32 dataBlockMarker = 0;   // KEY:   markers for data blocks
	u32 dataBlockPos = 0;      // VALUE: positions of each data block in the file
};

class DataHeader
{
public:

	DataHeader(const int numRecords) :
		recordsCount_(numRecords)
	{
		Assert::True(numRecords > 0, "wrong num of records (must be > 0)");

		try
		{
			records_ = new DataHeaderRecord[numRecords];
		}
		catch (const std::bad_alloc& e)
		{
			(void)e;
			throw LIB_Exception("can't allocate memory for a header");
		}
	}


	inline const u32 GetRecordsCount() const { return recordsCount_; }

	inline u32& GetDataBlockPos(const u32 idx)
	{
		// return a data start position for component 
		// by idx (look at the eComponentType enum)
		return records_[idx].dataBlockPos;
	}

	u32 recordsCount_ = 0;
	DataHeaderRecord* records_ = nullptr;
};

} // namespace ECS
