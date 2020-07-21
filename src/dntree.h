#ifndef DNTREE_H
#define DNTREE_H

#include <stdlib.h>
#include <stdint.h>

#define DNTREE_MALLOC malloc
#define DNTREE_LABEL_MAX_LEN 63
#define DNTREE_HT_INIT_CAPACITY 8

const char *dntree_string_alloc(const char *str);

struct dntree_dn_iter {
  const char *lo;
  const char *hi;
};

void dntree_dn_iter_init(struct dntree_dn_iter *iter, const char *domain);
int dntree_dn_iter_has_next(struct dntree_dn_iter *iter);
int dntree_dn_iter_next(struct dntree_dn_iter *iter, char *label);

struct dntree;

/* Open addressing hash table */
struct dntree_ht {
  struct dntree **arr;
  uint32_t size;
  uint32_t capacity;
};

struct dntree {
  const char *label;
  void *data;
  struct dntree_ht children;
};

void dntree_init(struct dntree *root);
int dntree_get_or_create(struct dntree *root, const char *domain,
                         struct dntree **res);
int dntree_walk(struct dntree **root, const char *label);

#endif //DNTREE_H
