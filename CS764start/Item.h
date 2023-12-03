#include "defs.h"
#include <vector>
#include <string>
#include <string.h>

typedef uint64_t RowCount;
typedef uint32_t RowSize;

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
	Item (RowSize row_size, char init_char = '0');
	bool operator < (const Item & other) const;
	char* GetItemString();
	~Item() = default;
};
