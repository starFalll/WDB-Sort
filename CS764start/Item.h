#include "defs.h"
#include <vector>
#include <string>
#include <string.h>

typedef uint64_t RowCount;
typedef uint32_t ElementSize;

enum ItemField
{
	INCL = 0,
	MEM,
	MGMT,
	MAX_ITEM,
};

static const ItemField COMPARE_FIELD = INCL;

static const std::vector<ItemField> ITEM_FIELDS = {INCL, MEM, MGMT};

// Define class for data records
// 24 bytes
struct Item
{
	Item (const FieldType incl, const FieldType mem, const FieldType mgmt);
	FieldType fields[3]; 
	Item (const Item& other);
	Item ();
	Item (ElementSize eSize);
	bool operator < (const Item & other) const;
	const FieldType* GetItemString() const;
	~Item() = default;
};

// the minimal value of object Item
//static const Item ITEM_MIN = Item(0, 0, 0);
static const Item ITEM_MIN = Item("0", "0", "0");
// the maximal value of object Item
//static const Item ITEM_MAX = Item(UINT32_MAX, UINT32_MAX, UINT32_MAX);
static const Item ITEM_MAX = Item(std::to_string(UINT32_MAX).c_str(), std::to_string(UINT32_MAX).c_str(), std::to_string(UINT32_MAX).c_str());