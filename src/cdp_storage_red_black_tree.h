/*
 *  Copyright (c) 2024 Victor M. Barrientos <firmw.guy@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */


typedef struct _cdpRbTreeNode   cdpRbTreeNode;

struct _cdpRbTreeNode {
    cdpRbTreeNode*  left;         // Left node.
    cdpRbTreeNode*  right;        // Right node.
    cdpRbTreeNode*  tParent;      // Parent node.
    bool            isRed;        // True if node is red.
    //
    cdpRecord       record;       // Child record.
};

typedef struct {
    cdpChdStore     store;     // Parent info.
    //
    cdpRbTreeNode*  root;         // The root node.
    //cdpRbTreeNode*  maximum;      // Node holding the maximum data.
    //cdpRbTreeNode*  minimum;      // Node holding the minimum data.
} cdpRbTree;




/*
    Red-black tree implementation
*/

#define rb_tree_new()     cdp_new(cdpRbTree)
#define rb_tree_del       cdp_free


static inline cdpRbTreeNode* rb_tree_node_from_record(cdpRecord* record) {
    return cdp_ptr_dif(record, offsetof(cdpRbTreeNode, record));
}


static inline void rb_tree_rotate_left(cdpRbTree* tree, cdpRbTreeNode* x) {
    cdpRbTreeNode* y = x->right;
    x->right = y->left;
    if (y->left)
        y->left->tParent = x;
    y->tParent = x->tParent;
    if (!x->tParent) {
        tree->root = y;
    } else if (x == x->tParent->left) {
        x->tParent->left = y;
    } else {
        x->tParent->right = y;
    }
    y->left = x;
    x->tParent = y;
}

static inline void rb_tree_rotate_right(cdpRbTree* tree, cdpRbTreeNode* x) {
    cdpRbTreeNode* y = x->left;
    x->left = y->right;
    if (y->right)
        y->right->tParent = x;
    y->tParent = x->tParent;
    if (!x->tParent) {
        tree->root = y;
    } else if (x == x->tParent->right) {
        x->tParent->right = y;
    } else {
        x->tParent->left = y;
    }
    y->right = x;
    x->tParent = y;
}

static inline void rb_tree_fix_insert(cdpRbTree* tree, cdpRbTreeNode* z) {
    while (z != tree->root && z->tParent->isRed) {
        if (z->tParent == z->tParent->tParent->left) {
            cdpRbTreeNode* y = z->tParent->tParent->right;
            if (y && y->isRed) {
                z->tParent->isRed = false;
                y->isRed = false;
                z->tParent->tParent->isRed = true;
                z = z->tParent->tParent;
            } else {
                if (z == z->tParent->right) {
                    z = z->tParent;
                    rb_tree_rotate_left(tree, z);
                }
                z->tParent->isRed = false;
                z->tParent->tParent->isRed = true;
                rb_tree_rotate_right(tree, z->tParent->tParent);
            }
        } else {
            cdpRbTreeNode* y = z->tParent->tParent->left;
            if (y && y->isRed) {
                z->tParent->isRed = false;
                y->isRed = false;
                z->tParent->tParent->isRed = true;
                z = z->tParent->tParent;
            } else {
                if (z == z->tParent->left) {
                    z = z->tParent;
                    rb_tree_rotate_right(tree, z);
                }
                z->tParent->isRed = false;
                z->tParent->tParent->isRed = true;
                rb_tree_rotate_left(tree, z->tParent->tParent);
            }
        }
    }
    tree->root->isRed = false;
}


static inline void rb_tree_sorted_insert(cdpRbTree* tree, cdpRbTreeNode* tnode, cdpCompare compare, void* context) {
    if (tree->root) {
        cdpRbTreeNode* x = tree->root, *y;
        do {
            y = x;
            int cmp = compare(&tnode->record, &x->record, context);
            if (0 > cmp) {
                x = x->left;
            } else if (0 < cmp) {
                x = x->right;
            } else {
                // FixMe: delete children.
                assert(0 == cmp);
            }
        } while (x);
        tnode->tParent = y;
        if (0 > compare(&tnode->record, &y->record, context)) {
            y->left = tnode;
        } else {
            y->right = tnode;
        }
    } else {
        tree->root = tnode;
    }
    rb_tree_fix_insert(tree, tnode);
}


static inline cdpRecord* rb_tree_add(cdpRbTree* tree, cdpRecord* parent, const cdpRecord* record) {
    assert(cdp_record_is_dict_or_cat(parent));

    CDP_NEW(cdpRbTreeNode, tnode);
    tnode->isRed = true;
    cdpRecord* child = &tnode->record;
    *child = *record;

    if (cdp_record_is_dictionary(parent)) {
        rb_tree_sorted_insert(tree, tnode, record_compare_by_name, NULL);
    } else if (cdp_record_is_catalog(parent)) {
        rb_tree_sorted_insert(tree, tnode, tree->store.sorter->compare, tree->store.sorter->context);
    }

    return child;
}


static inline cdpRecord* rb_tree_first(cdpRbTree* tree) {
    cdpRbTreeNode* tnode = tree->root;
    while (tnode->left)   tnode = tnode->left;
    return &tnode->record;
}


static inline cdpRecord* rb_tree_last(cdpRbTree* tree) {
    cdpRbTreeNode* tnode = tree->root;
    while (tnode->right)  tnode = tnode->right;
    return &tnode->record;
}


static inline bool rb_tree_traverse(cdpRbTree* tree, cdpRecord* book, unsigned maxDepth, cdpRecordTraverse func, void* context, cdpBookEntry* entry) {
  cdpRbTreeNode* tnode = tree->root, *tnodePrev = NULL;
  cdpRbTreeNode* stack[maxDepth];
  int top = -1;  // Stack index initialized to empty.

  entry->parent = book;
  do {
      if (tnode) {
          assert(top < ((int)maxDepth - 1));
          stack[++top] = tnode;
          tnode = tnode->left;
      } else {
          tnode = stack[top--];
          if (tnodePrev) {
              entry->next = &tnode->record;
              entry->record = &tnodePrev->record;
              if (!func(entry, 0, context))
                  return false;
              entry->position++;
              entry->prev = entry->record;
          }
          tnodePrev = tnode;
          tnode = tnode->right;
      }
  } while (top != -1 || tnode);

  entry->next = NULL;
  entry->record = &tnodePrev->record;
  return func(entry, 0, context);
}


static inline int rb_traverse_func_break_at_name(cdpBookEntry* entry, unsigned u, uintptr_t id) {
    return (entry->record->metadata.id != id);
}

static inline cdpRecord* rb_tree_find_by_name(cdpRbTree* tree, cdpID id, const cdpRecord* book) {
    if (cdp_record_is_dictionary(book)) {
        cdpRecord key = {.metadata.id = id};
        cdpRbTreeNode* tnode = tree->root;
        do {
            int cmp = record_compare_by_name(&key, &tnode->record, NULL);
            if (0 > cmp) {
                tnode = tnode->left;
            } else if (0 < cmp) {
                tnode = tnode->right;
            } else {
                return &tnode->record;
            }
        } while (tnode);
    } else {
        cdpBookEntry entry = {0};
        if (!rb_tree_traverse(tree, CDP_P(book), cdp_bitson(tree->store.chdCount) + 2, (cdpFunc) rb_traverse_func_break_at_name, cdp_v2p(id), &entry))
            return entry.record;
    }
    return NULL;
}


static inline cdpRecord* rb_tree_find_by_key(cdpRbTree* tree, cdpRecord* key) {
    cdpRbTreeNode* tnode = tree->root;
    do {
        int cmp = tree->store.sorter->compare(key, &tnode->record, tree->store.sorter->context);
        if (0 > cmp) {
            tnode = tnode->left;
        } else if (0 < cmp) {
            tnode = tnode->right;
        } else {
            return &tnode->record;
        }
    } while (tnode);
    return NULL;
}


static inline int rb_traverse_func_break_at_position(cdpBookEntry* entry, unsigned u, uintptr_t position) {
    return (entry->position != position);
}

static inline cdpRecord* rb_tree_find_by_position(cdpRbTree* tree, size_t position, const cdpRecord* book) {
    cdpBookEntry entry = {0};
    if (!rb_tree_traverse(tree, CDP_P(book), cdp_bitson(tree->store.chdCount) + 2, (void*) rb_traverse_func_break_at_position, cdp_v2p(position), &entry))
        return entry.record;
    return NULL;
}


static inline cdpRecord* rb_tree_prev(cdpRecord* record) {
    cdpRbTreeNode* tnode = rb_tree_node_from_record(record);
    if (tnode->left) {
        tnode = tnode->left;
        while (tnode->right) tnode = tnode->right;
        return &tnode->record;
    }
    cdpRbTreeNode* tParent = tnode->tParent;
    while (tParent && tnode == tParent->left) {
        tnode = tParent;
        tParent = tParent->tParent;
    }
    return tParent? &tParent->record: NULL;
}


static inline cdpRecord* rb_tree_next(cdpRecord* record) {
    cdpRbTreeNode* tnode = rb_tree_node_from_record(record);
    if (tnode->right) {
        tnode = tnode->right;
        while (tnode->left) tnode = tnode->left;
        return &tnode->record;
    }
    cdpRbTreeNode* tParent = tnode->tParent;
    while (tParent && tnode == tParent->right) {
        tnode = tParent;
        tParent = tParent->tParent;
    }
    return tParent? &tParent->record: NULL;
}


static inline void rb_tree_transplant(cdpRbTree* tree, cdpRbTreeNode* u, cdpRbTreeNode* v) {
    if (!u->tParent) {
        tree->root = v;
    } else if (u == u->tParent->left) {
        u->tParent->left = v;
    } else {
        u->tParent->right = v;
    }
    if (v)
        v->tParent = u->tParent;
}

static inline void rb_tree_fixremove_node(cdpRbTree* tree, cdpRbTreeNode* x) {
    while (x != tree->root && !x->isRed) {
        if (x == x->tParent->left) {
            cdpRbTreeNode* w = x->tParent->right;
            if (!w) break;
            if (w->isRed) {
                w->isRed = false;
                x->tParent->isRed = true;
                rb_tree_rotate_left(tree, x->tParent);
                w = x->tParent->right;
            }
            if (!w || !w->left || !w->right) break;
            if (!w->left->isRed && !w->right->isRed) {
                w->isRed = true;
                x = x->tParent;
            } else {
                if (!w->right->isRed) {
                    w->left->isRed = false;
                    w->isRed = true;
                    rb_tree_rotate_right(tree, w);
                    w = x->tParent->right;
                }
                w->isRed = x->tParent->isRed;
                x->tParent->isRed = false;
                w->right->isRed = false;
                rb_tree_rotate_left(tree, x->tParent);
                x = tree->root;
            }
        } else {
            cdpRbTreeNode* w = x->tParent->left;
            if (!w) break;
            if (w->isRed) {
                w->isRed = false;
                x->tParent->isRed = true;
                rb_tree_rotate_right(tree, x->tParent);
                w = x->tParent->left;
            }
            if (!w || !w->right || !w->left) break;
            if (!w->right->isRed && !w->left->isRed) {
                w->isRed = true;
                x = x->tParent;
            } else {
                if (!w->left->isRed) {
                    w->right->isRed = false;
                    w->isRed = true;
                    rb_tree_rotate_left(tree, w);
                    w = x->tParent->left;
                }
                w->isRed = x->tParent->isRed;
                x->tParent->isRed = false;
                w->left->isRed = false;
                rb_tree_rotate_right(tree, x->tParent);
                x = tree->root;
            }
        }
    }
    x->isRed = false;
}

static inline void rb_tree_remove_record(cdpRbTree* tree, cdpRecord* record) {
    cdpRbTreeNode* tnode = rb_tree_node_from_record(record);
    cdpRbTreeNode* y = tnode, *x;
    bool wasRed = tnode->isRed;

    if (!tnode->left) {
        x = tnode->right;
        rb_tree_transplant(tree, tnode, x);
    } else if (!tnode->right) {
        x = tnode->left;
        rb_tree_transplant(tree, tnode, x);
    } else {
        for (y = tnode->right;  y->left;  y = y->left);
        wasRed = y->isRed;
        x = y->right;

        if (y->tParent == tnode) {
            if (x)  x->tParent = y;
        } else {
            rb_tree_transplant(tree, y, x);
            y->right = tnode->right;
            y->right->tParent = y;
        }
        rb_tree_transplant(tree, tnode, y);
        y->left = tnode->left;
        y->left->tParent = y;
        y->isRed = tnode->isRed;
    }
    if (x && !wasRed)
        rb_tree_fixremove_node(tree, x);

    cdp_free(tnode);
}


static inline void rb_tree_del_all_children_recursively(cdpRbTreeNode* tnode, unsigned maxDepth) {
    if (tnode->left)
        rb_tree_del_all_children_recursively(tnode->left, maxDepth);

    cdp_record_finalize(&tnode->record, maxDepth - 1);

    if (tnode->right)
        rb_tree_del_all_children_recursively(tnode->right, maxDepth);

    cdp_free(tnode);
}

static inline void rb_tree_del_all_children(cdpRbTree* tree, unsigned maxDepth) {
    if (tree->root) {
        rb_tree_del_all_children_recursively(tree->root, maxDepth);
        tree->root = NULL;
    }
}
