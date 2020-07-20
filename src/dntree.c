#include "dntree.h"

#include <string.h>

struct string_pool {
  struct string_pool *next;
  const char *str;
};

static struct string_pool *spool;

const char *dntree_string_alloc(const char *str) {
  for (struct string_pool *cursor = spool;
       cursor != NULL; cursor = cursor->next) {
    if (strcmp(cursor->str, str) == 0) return cursor->str;
  }
  struct string_pool *new_head = DNTREE_MALLOC(sizeof(struct string_pool));
  if (new_head == NULL) return NULL;
  new_head->str = malloc(strlen(str) + 1);
  if (new_head->str == NULL) return NULL;
  strcpy((char *) new_head->str, str);
  new_head->next = spool;
  spool = new_head;
  return spool->str;
}

void dntree_dn_iter_init(struct dntree_dn_iter *iter, const char *domain) {
  iter->lo = domain;
  iter->hi = domain + strlen(domain);
}

int dntree_dn_iter_has_next(struct dntree_dn_iter *iter) {
  return iter->lo < iter->hi;
}

int dntree_dn_iter_next(struct dntree_dn_iter *iter, char *label) {
  if (iter->lo == iter->hi) return -1;
  const char *cursor;
  for (cursor = iter->hi - 1; cursor >= iter->lo; cursor--)
    if (*cursor == '.') break;
  if (cursor >= iter->lo) {
    if (cursor + 1 < iter->hi) {
      size_t len = iter->hi - cursor - 1;
      if (len > DNTREE_LABEL_MAX_LEN) return -1;
      memcpy(label, cursor + 1, len);
      label[len] = '\0';
      iter->hi = cursor;
      return 0;
    } else {
      return -1;
    }
  } else {
    if (iter->lo < iter->hi) {
      size_t len = iter->hi - iter->lo;
      if (len > DNTREE_LABEL_MAX_LEN) return -1;
      memcpy(label, iter->lo, len);
      label[len] = '\0';
      iter->hi = iter->lo;
      return 0;
    } else {
      return -1;
    }
  }
}

void dntree_init(struct dntree *root) {
  root->label = NULL;
  root->data = NULL;

  root->children = NULL;
  root->children_size = 0;
  root->children_capacity = 0;
}

static int add_or_get_child(struct dntree **tree, const char *label) {
  size_t idx = (*tree)->children_size;
  for (size_t i = 0; i < (*tree)->children_size; i++) {
    struct dntree *cur = &(*tree)->children[i];
    int cmp = strcmp(label, cur->label);
    if (cmp == 0) {
      *tree = cur;
      return 0;
    } else if (cmp < 0) {
      idx = i;
      break;
    }
  }

  if ((*tree)->children_size == (*tree)->children_capacity) {
    if ((*tree)->children_capacity == 0) {
      (*tree)->children_capacity = 1;
      (*tree)->children = DNTREE_MALLOC(sizeof(struct dntree));
      if ((*tree)->children == NULL) return -1;
    } else {
      (*tree)->children_capacity *= 2;
      struct dntree *new_children =
          DNTREE_MALLOC(sizeof(struct dntree) * (*tree)->children_capacity);
      if (new_children == NULL) return -1;
      memcpy(new_children, (*tree)->children,
             sizeof(struct dntree) * (*tree)->children_size);
      free((*tree)->children);
      (*tree)->children = new_children;
    }
  }

  if (idx < (*tree)->children_size)
    memmove((*tree)->children + idx + 1, (*tree)->children + idx,
            sizeof(struct dntree) * ((*tree)->children_size - idx));
  (*tree)->children_size++;
  (*tree) = &(*tree)->children[idx];
  dntree_init(*tree);
  (*tree)->label = dntree_string_alloc(label);
  if ((*tree)->label == NULL) return -1;

  return 0;
}

int dntree_set(struct dntree *root, const char *domain, void *data) {
  int err;

  if (strcmp(domain, "#") != 0) {
    char label[DNTREE_LABEL_MAX_LEN + 1];
    struct dntree_dn_iter iter;
    dntree_dn_iter_init(&iter, domain);
    while (dntree_dn_iter_has_next(&iter)) {
      err = dntree_dn_iter_next(&iter, label);
      if (err) return err;
      err = add_or_get_child(&root, label);
      if (err) return err;
    }
  }
  root->data = data;

  return 0;
}

void **dntree_get(struct dntree *root, const char *domain) {
  int err;

  if (strcmp(domain, "#") != 0) {
    char label[DNTREE_LABEL_MAX_LEN + 1];
    struct dntree_dn_iter iter;
    dntree_dn_iter_init(&iter, domain);
    while (dntree_dn_iter_has_next(&iter)) {
      err = dntree_dn_iter_next(&iter, label);
      if (err) return NULL;
      err = dntree_walk(&root, label);
      if (err) return NULL;
    }
  }

  return &root->data;
}

int dntree_walk(struct dntree **root, const char *label) {
  if (*root == NULL || (*root)->children_size == 0) return -1;

  ssize_t lo = 0, hi = (*root)->children_size - 1, mi;
  while (lo <= hi) {
    mi = lo + (hi - lo) / 2;
    struct dntree *p = &(*root)->children[mi];
    int cmp = strcmp(label, p->label);
    if (cmp == 0) {
      *root = p;
      return 0;
    } else {
      if (cmp < 0) hi = mi - 1; else lo = mi + 1;
    }
  }

  return -1;
}
