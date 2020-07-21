#include "dntree.h"

#include <string.h>

const char *dntree_string_alloc(const char *str) {
  char *new_str = DNTREE_MALLOC(strlen(str) + 1);
  if (new_str == NULL) return NULL;
  strcpy(new_str, str);
  return new_str;
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

static inline uint32_t jenkins_one_at_a_time_hash(const uint8_t *key) {
  uint32_t hash = 0;
  for (; *key != '\0'; key++) {
    hash += *key;
    hash += hash << 10u;
    hash ^= hash >> 6u;
  }
  hash += hash << 3u;
  hash ^= hash >> 11u;
  hash += hash << 15u;
  return hash;
}

static uint32_t hash(const char *key) {
  return jenkins_one_at_a_time_hash((const uint8_t *) key);
}

static void dntree_ht_init(struct dntree_ht *ht) {
  ht->size = 0;
  ht->capacity = DNTREE_HT_INIT_CAPACITY;
  ht->arr = DNTREE_MALLOC(ht->capacity * sizeof(struct dntree *));
  memset(ht->arr, 0, ht->capacity * sizeof(struct dntree *));
}

static uint32_t dntree_ht_search(struct dntree_ht *ht, const char *key) {
  uint32_t idx = hash(key) % ht->capacity;
  for (uint32_t i = 0; i < ht->capacity; i++) {
    uint32_t k = (idx + i) % ht->capacity;
    if (ht->arr[k] == NULL || strcmp(key, ht->arr[k]->label) == 0)
      return k;
  }
  return UINT32_MAX;
}

static int dntree_ht_enlarge(struct dntree_ht *ht) {
  if (ht->capacity >= UINT32_MAX / 2) return -1;

  uint32_t old_capacity = ht->capacity;
  struct dntree **old_arr = ht->arr;

  ht->capacity *= 2;
  ht->arr = DNTREE_MALLOC(ht->capacity * sizeof(struct dntree *));
  memset(ht->arr, 0, ht->capacity * sizeof(struct dntree *));

  for (uint32_t i = 0; i < old_capacity; i++) {
    struct dntree *elem = old_arr[i];
    if (elem != NULL) {
      uint32_t idx = dntree_ht_search(ht, elem->label);
      ht->arr[idx] = elem;
    }
  }

  free(old_arr);

  return 0;
}

void dntree_init(struct dntree *root) {
  root->label = NULL;
  root->data = NULL;
  dntree_ht_init(&root->children);
}

int dntree_get_or_create(struct dntree *root, const char *domain,
                         struct dntree **res) {
  int err;

  if (strcmp(domain, "#") != 0) {
    struct dntree_dn_iter iter;
    char label[DNTREE_LABEL_MAX_LEN + 1];

    dntree_dn_iter_init(&iter, domain);
    while (dntree_dn_iter_has_next(&iter)) {
      err = dntree_dn_iter_next(&iter, label);
      if (err) return err;
      uint32_t idx = dntree_ht_search(&root->children, label);
      if (root->children.arr[idx] == NULL) {
        root->children.size++;
        if (root->children.size * 2 > root->children.capacity) {
          err = dntree_ht_enlarge(&root->children);
          if (err) return err;
          idx = dntree_ht_search(&root->children, label);
        }
        root->children.arr[idx] = DNTREE_MALLOC(sizeof(struct dntree));
        dntree_init(root->children.arr[idx]);
        root->children.arr[idx]->label = dntree_string_alloc(label);
      }
      root = root->children.arr[idx];
    }
  }
  *res = root;

  return 0;
}

int dntree_walk(struct dntree **root, const char *label) {
  uint32_t idx = dntree_ht_search(&(*root)->children, label);
  if ((*root)->children.arr[idx] == NULL) return -1;
  *root = (*root)->children.arr[idx];
  return 0;
}
