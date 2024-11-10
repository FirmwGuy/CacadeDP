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


#include <math.h>


typedef struct _cdpOctreeList   cdpOctreeList;
typedef struct _cdpOctreeNode   cdpOctreeNode;

struct _cdpOctreeList {
    cdpOctreeList*  next;           // Next child in current sector.
    //cdpOctreeList*  self;           // Next self in other sectors.
    //
    cdpRecord       record;         // Child record.
};

struct _cdpOctreeNode {
    cdpOctreeNode*  children[8];    // Pointers to child nodes.
    float           center[3];      // Center of the node space (XYZ coords).
    float           subwide;        // Half the width/height/depth of the node space.
    cdpOctreeList*  list;           // List of records in this node.
};

typedef struct {
    cdpStore        store;        // Parent info.
    //
    cdpOctreeNode   root;         // The root node.
} cdpOctree;


#define EPSILON     (1e-10)




/*
    Octree implementation
*/

static inline cdpOctree* octree_new(float* center, float subwide) {
    assert(fabs(subwide) > EPSILON);

    CDP_NEW(cdpOctree, octree);
    octree->root.subwide = subwide;
    memcpy(octree->root.center, center, sizeof((cdpOctreeNode){}.center));

    return octree;
}


static inline void octree_del(cdpOctree* octree){
    // ToDo: del all nodes!

    cdp_free(octree);
}


static inline cdpOctreeNode* octree_node_new(cdpRecord* record) {
    CDP_NEW(cdpOctreeNode, tnode);
    //cdp_record_transfer(record, &tnode->record);
    return tnode;
}


static inline cdpOctreeNode* octree_node_from_record(cdpRecord* record) {
    // return cdp_ptr_dif(record, offsetof(cdpOctreeNode, record));
    return NULL;
}



static inline void octree_sorted_insert_tnode(cdpOctree* tree, cdpOctreeNode* tnode, cdpCompare compare, void* context) {
        /*
    if (tree->root) {
        cdpOctreeNode* x = tree->root, *y;
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
        */
}


static inline cdpRecord* octree_sorted_insert(cdpOctree* tree, cdpRecord* record, cdpCompare compare, void* context) {
    cdpOctreeNode* tnode = octree_node_new(record);
    octree_sorted_insert_tnode(tree, tnode, compare, context);
    //return &tnode->record;
    return NULL;
}


static inline cdpRecord* octree_add(cdpOctree* tree, cdpRecord* parent, cdpRecord* record) {
    cdpOctreeNode* tnode = octree_node_new(record);
    octree_sorted_insert_tnode(tree, tnode, record_compare_by_name, NULL);
    //return &tnode->record;
    return NULL;
}


static inline cdpRecord* octree_add_property(cdpOctree* tree, cdpRecord* record) {
    cdpOctreeNode* tnode = octree_node_new(record);
    octree_sorted_insert_tnode(tree, tnode, record_compare_by_name, NULL);
    //return &tnode->record;
    return NULL;
}


static inline cdpRecord* octree_first(cdpOctree* tree) {
    //cdpOctreeNode* tnode = tree->root;

    // pending...

    return NULL;
}


static inline cdpRecord* octree_last(cdpOctree* tree) {
    //cdpOctreeNode* tnode = tree->root;

    // pending...

    return NULL;
}


static inline bool octree_traverse(cdpOctree* tree, unsigned maxDepth, cdpTraverse func, void* context, cdpEntry* entry) {
  /*
  cdpOctreeNode* tnode = tree->root, *tnodePrev = NULL;
  cdpOctreeNode* stack[maxDepth];
  int top = -1;  // Stack index initialized to empty.

  entry->parent = octree->store.owner;
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
  */

  return false;
}


static inline cdpRecord* octree_find_by_id(cdpOctree* tree, cdpID name) {
    /*
    cdpRecord key = {.metarecord.name = name};
    cdpOctreeNode* tnode = tree->root;
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
    */
    return NULL;
}


static inline cdpRecord* octree_find_by_name(cdpOctree* tree, cdpID id) {
    if (cdp_store_is_dictionary(&tree->store)) {
        return octree_find_by_id(tree, id);
    } else {
        cdpEntry entry = {0};
        if (!octree_traverse(tree, cdp_bitson(tree->store.chdCount) + 2, (cdpFunc) rb_traverse_func_break_at_name, cdp_v2p(id), &entry))
            return entry.record;
    }
    return NULL;
}


static inline cdpRecord* octree_find_by_key(cdpOctree* tree, cdpRecord* key, cdpCompare compare, void* context) {
    //cdpOctreeNode* tnode = tree->root;

    // pending...

    return NULL;
}


static inline cdpRecord* octree_find_by_position(cdpOctree* tree, size_t position, const cdpRecord* parent) {
    cdpEntry entry = {0};
    if (!octree_traverse(tree, cdp_bitson(tree->store.chdCount) + 2, (void*) rb_traverse_func_break_at_position, cdp_v2p(position), &entry))
        return entry.record;
    return NULL;
}


static inline cdpRecord* octree_prev(cdpRecord* record) {
    //cdpOctreeNode* tnode = octree_node_from_record(record);

    // pending...

    return NULL;
}


static inline cdpRecord* octree_next(cdpRecord* record) {
    //cdpOctreeNode* tnode = octree_node_from_record(record);

    // pending...

    return NULL;
}



static inline void octree_remove_record(cdpOctree* tree, cdpRecord* record) {
    cdpOctreeNode* tnode = octree_node_from_record(record);

    // pending...

    cdp_free(tnode);
}


static inline void octree_take(cdpOctree* tree, cdpRecord* target) {
    cdpRecord* last = octree_last(tree);
    cdp_record_transfer(last, target);
    octree_remove_record(tree, last);
}


static inline void octree_pop(cdpOctree* tree, cdpRecord* target) {
    cdpRecord* first = octree_first(tree);
    cdp_record_transfer(first, target);
    octree_remove_record(tree, first);
}


static inline void octree_del_all_children_recursively(cdpOctreeNode* tnode) {
    /*
    if (tnode->left)
        octree_del_all_children_recursively(tnode->left);

    cdp_record_finalize(&tnode->record);

    if (tnode->right)
        octree_del_all_children_recursively(tnode->right);

    */
    cdp_free(tnode);
}

static inline void octree_del_all_children(cdpOctree* octree) {

}
