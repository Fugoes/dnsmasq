#ifndef DNTREE_H
#define DNTREE_H

#include <stdlib.h>

#define DNTREE_MALLOC malloc
#define DNTREE_LABEL_MAX_LEN 63

const char *dntree_string_alloc(const char *str);

struct dntree_dn_iter {
  const char *lo;
  const char *hi;
};

void dntree_dn_iter_init(struct dntree_dn_iter *iter, const char *domain);
int dntree_dn_iter_has_next(struct dntree_dn_iter *iter);
int dntree_dn_iter_next(struct dntree_dn_iter *iter, char *label);

struct dntree {
  const char *label;
  void *data;

  struct dntree *children;
  size_t children_size, children_capacity;
};

void dntree_init(struct dntree *root);
int dntree_set(struct dntree *root, const char *domain, void *data);
int dntree_walk(struct dntree **root, const char *label);
void **dntree_get(struct dntree *root, const char *domain);

#endif //DNTREE_H
