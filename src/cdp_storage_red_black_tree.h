/*
 *  Copyright (c) 2024 Victor M. Barrientos (https://github.com/FirmwGuy/CacadeDP)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so.
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
    cdpChdStore     store;        // Parent info.
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


static inline cdpRbTreeNode* rb_tree_node_new(cdpRecord* record) {
    CDP_NEW(cdpRbTreeNode, tnode);
    tnode->isRed = true;
    cdp_record_transfer(record, &tnode->record);
    return tnode;
}


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


static inline void rb_tree_sorted_insert_tnode(cdpRbTree* tree, cdpRbTreeNode* tnode, cdpCompare compare, void* context) {
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


static inline cdpRecord* rb_tree_sorted_insert(cdpRbTree* tree, cdpRecord* record, cdpCompare compare, void* context) {
    cdpRbTreeNode* tnode = rb_tree_node_new(record);
    rb_tree_sorted_insert_tnode(tree, tnode, compare, context);
    return &tnode->record;
}


static inline cdpRecord* rb_tree_add(cdpRbTree* tree, cdpRecord* parent, cdpRecord* record) {
    assert(cdp_record_is_dictionary(parent));
    cdpRbTreeNode* tnode = rb_tree_node_new(record);
    rb_tree_sorted_insert_tnode(tree, tnode, record_compare_by_name, NULL);
    return &tnode->record;
}


static inline cdpRecord* rb_tree_add_property(cdpRbTree* tree, cdpRecord* record) {
    cdpRbTreeNode* tnode = rb_tree_node_new(record);
    rb_tree_sorted_insert_tnode(tree, tnode, record_compare_by_name, NULL);
    return &tnode->record;
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


static inline bool rb_tree_traverse(cdpRbTree* tree, cdpRecord* parent, unsigned maxDepth, cdpTraverse func, void* context, cdpBookEntry* entry) {
  cdpRbTreeNode* tnode = tree->root, *tnodePrev = NULL;
  cdpRbTreeNode* stack[maxDepth];
  int top = -1;  // Stack index initialized to empty.

  entry->parent = parent;
  entry->depth  = 0;
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
              if (!func(entry, context))
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
  return func(entry, context);
}


static inline int rb_traverse_func_break_at_name(cdpBookEntry* entry, uintptr_t name) {
    return (entry->record->metarecord.name != name);
}


static inline cdpRecord* rb_tree_find_by_id(cdpRbTree* tree, cdpID name) {
    cdpRecord key = {.metarecord.name = name};
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
    return NULL;
}


static inline cdpRecord* rb_tree_find_by_name(cdpRbTree* tree, cdpID id, const cdpRecord* parent) {
    if (cdp_record_is_dictionary(parent)) {
        return rb_tree_find_by_id(tree, id);
    } else {
        cdpBookEntry entry = {0};
        if (!rb_tree_traverse(tree, CDP_P(parent), cdp_bitson(tree->store.chdCount) + 2, (cdpFunc) rb_traverse_func_break_at_name, cdp_v2p(id), &entry))
            return entry.record;
    }
    return NULL;
}


static inline cdpRecord* rb_tree_find_by_key(cdpRbTree* tree, cdpRecord* key, cdpCompare compare, void* context) {
    cdpRbTreeNode* tnode = tree->root;
    do {
        int cmp = compare(key, &tnode->record, context);
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


static inline int rb_traverse_func_break_at_position(cdpBookEntry* entry, uintptr_t position) {
    return (entry->position != position);
}

static inline cdpRecord* rb_tree_find_by_position(cdpRbTree* tree, size_t position, const cdpRecord* parent) {
    cdpBookEntry entry = {0};
    if (!rb_tree_traverse(tree, CDP_P(parent), cdp_bitson(tree->store.chdCount) + 2, (void*) rb_traverse_func_break_at_position, cdp_v2p(position), &entry))
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


static inline void rb_tree_take(cdpRbTree* tree, cdpRecord* target) {
    cdpRecord* last = rb_tree_last(tree);
    cdp_record_transfer(last, target);
    rb_tree_remove_record(tree, last);
}


static inline void rb_tree_pop(cdpRbTree* tree, cdpRecord* target) {
    cdpRecord* first = rb_tree_first(tree);
    cdp_record_transfer(first, target);
    rb_tree_remove_record(tree, first);
}


static inline void rb_tree_del_all_children_recursively(cdpRbTreeNode* tnode) {
    if (tnode->left)
        rb_tree_del_all_children_recursively(tnode->left);

    cdp_record_finalize(&tnode->record);

    if (tnode->right)
        rb_tree_del_all_children_recursively(tnode->right);

    cdp_free(tnode);
}

static inline void rb_tree_del_all_children(cdpRbTree* tree) {
    if (tree->root) {
        rb_tree_del_all_children_recursively(tree->root);
        tree->root = NULL;
    }
}
