#include "Iterator.h"
#include <unordered_map>

// Predicate enum
enum PredicateEnum
{
	EQ = 1,  // qeual
	GT,      // greater than
	LT,      // less than
	GE,      // greater or equal to
	LE       // less or equal to
};

struct Predicate
{
	ItemField field;
	PredicateEnum predicate;
	/*
	FieldType value;
	Predicate (ItemField f, PredicateEnum p, FieldType v):
		field (f), predicate (p), value (v) {}
	*/
	FieldType value;
	Predicate (ItemField f, PredicateEnum p, FieldType v):
		field (f), predicate (p), value (v) {}

};

class FilterPlan : public Plan
{
	friend class FilterIterator;
public:
	FilterPlan (Plan * const input);
	virtual ~FilterPlan ();
	Iterator * init () const;

	/**
	 * set predicate (>=, <=, >, <, ==)
	*/
	//bool SetPredicate (ItemField field, PredicateEnum predicate, FieldType value) ;
	bool SetPredicate (ItemField field, PredicateEnum predicate, FieldType value) ;

	/**
	 * apply predicate to item
	 * return: 
	 *        true means not filter
	 *        false means filter
	*/
	bool ApplyPredicate (Item & item) const;

private:
	Plan * const _input;
	std::unordered_map <ItemField, std::vector <Predicate>> _predicates;
}; // class FilterPlan

class FilterIterator : public Iterator
{
public:
	FilterIterator (FilterPlan const * const plan);
	virtual ~FilterIterator ();
	bool next () override;
	void GetRecords(std::vector<Item> ** records, uint32_t ** index) override;

private:
	bool ApplyCondition();

private:
	uint32_t _filter_index;
	std::vector<Item> _filter_records;
	FilterPlan const * const _plan;
	Iterator * const _input;
	RowCount _consumed, _produced;
}; // class FilterIterator
