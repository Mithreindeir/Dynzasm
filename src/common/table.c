#include "table.h"

struct hash_table *hash_table_init()
{
	struct hash_table *table = malloc(sizeof(struct hash_table));
	table->buckets = NULL, table->num_buckets = 0;
	return table;
}

void hash_table_destroy(struct hash_table *table)
{
	if (!table) return;
	for (int i = 0; i < table->num_buckets; i++) {
		struct hash_entry *cur = table->buckets[i], *next = NULL;
		while (cur) {
			next = cur->next;
			hash_entry_destroy(cur);
			cur = next;
		}
	}
	free(table->buckets);
	free(table);
}

struct hash_entry *hash_entry_init(char *mnemonic, void *value)
{
	struct hash_entry *entry = malloc(sizeof(struct hash_entry));
	entry->mnemonic = mnemonic, entry->value = value;
	entry->hash = hash_str(mnemonic);
	return entry;
}

void hash_entry_destroy(struct hash_entry *entry)
{
	if (!entry) return;
	free(entry);
}

void hash_table_insert(struct hash_table *table, struct hash_entry *entry)
{
	if (!table->buckets) return;
	hash_entry_insert(&table->buckets[entry->hash % table->num_buckets], entry);
}

struct hash_entry *hash_table_lookup(struct hash_table *table, const char *mnem)
{
	if (!table->buckets) return NULL;
	unsigned long hv = hash_str(mnem);
	return hash_entry_lookup(table->buckets[hv % table->num_buckets], mnem, hv);
}

void hash_entry_insert(struct hash_entry **head, struct hash_entry *entry)
{
	if (!(*head) && ((*head)=entry)) return;
	struct hash_entry *cur = *head;
	while (cur->next) cur = cur->next;
	cur->next = entry;
}

struct hash_entry *hash_entry_lookup(struct hash_entry *head, const char *mnem, unsigned long hash)
{
	while (head && (head->hash!=hash || !!strcmp(head->mnemonic, mnem))) head=head->next;
	return head;
}

unsigned long hash_str(const char *str)
{
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)) hash = ((hash << 5) + hash) + c;
	return hash;
}
