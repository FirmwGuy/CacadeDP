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


typedef struct _cdpOctreeList   cdpOctreeList;
typedef struct _cdpOctreeNode   cdpOctreeNode;

struct _cdpOctreeList {
    cdpOctreeList*  next;           // Next child in current sector.
    cdpOctreeList*  prev;           // Previous child in current sector.
    cdpOctreeNode*  onode;          // Node owning this list.
    //cdpOctreeList*  self;           // Next self in other sectors.
    //
    cdpRecord       record;         // Child record.
};

typedef struct {
    float           subwide;        // Half the width/height/depth of the bounding space.
    float           center[3];      // Center of the bounding space (XYZ coords).
} cdpOctreeBound;

struct _cdpOctreeNode {
    cdpOctreeNode*  children[8];    // Pointers to child nodes.
    cdpOctreeNode*  parent;         // Parent node.
    cdpOctreeList*  list;           // List of records in this node.
    cdpOctreeBound  bound;          // Bounding space covered by this node.
    unsigned        index;          // Child index of this node in parent.
};

typedef struct {
    cdpStore        store;          // Storage info.
    //
    cdpOctreeNode   root;           // The root node.
    unsigned        depth;          // Maximum tree depth (ever used).
} cdpOctree;


#define EPSILON     (1e-10)




/*
    Octree implementation
*/

static inline cdpOctreeNode* octree_node_new(cdpOctreeNode* parent, cdpOctreeBound* bound, unsigned index) {
    assert(bound && bound->subwide > EPSILON);
    CDP_NEW(cdpOctreeNode, onode);
    onode->parent = parent;
    onode->bound  = *bound;
    onode->index  = index;
    return onode;
}


static inline void octree_node_del(cdpOctreeNode* onode);

static inline void octree_node_clean(cdpOctreeNode* onode) {
    for (unsigned n = 0;  n < 8;  n++) {
        if (onode->children[n])
            octree_node_del(onode->children[n]);
    }

    cdpOctreeList* list = onode->list;
    while (list) {
        cdpOctreeList* next = list->next;
        cdp_free(list);
        list = next;
    }
}


static inline void octree_node_del(cdpOctreeNode* onode) {
    octree_node_clean(onode);
    cdp_free(onode);
}


static inline cdpOctree* octree_new(cdpOctreeBound* bound) {
    assert(bound && bound->subwide > EPSILON);
    CDP_NEW(cdpOctree, octree);
    octree->root.bound = *bound;
    octree->depth = 1;
    return octree;
}


static inline void octree_del(cdpOctree* octree){
    if (!octree) return;
    octree_node_clean(&octree->root);
    cdp_free(octree);
}



static inline cdpOctreeList* octree_list_from_record(cdpRecord* record) {
    return cdp_ptr_dif(record, offsetof(cdpOctreeList, record));
}


#define BOUND_CENTER_QUADRANT(bound, onode, opX, opY, opZ)              \
    do {                                                                \
        bound.center[0] = onode->bound.center[0] opX bound.subwide;     \
        bound.center[1] = onode->bound.center[1] opY bound.subwide;     \
        bound.center[2] = onode->bound.center[2] opZ bound.subwide;     \
    } while(0)


static inline cdpRecord* octree_sorted_insert(cdpOctree* octree, cdpRecord* record, cdpCompare compare, void* context) {
    CDP_NEW(cdpOctreeList, list);
    cdp_record_transfer(record, &list->record);

    cdpOctreeNode* onode = &octree->root;
    unsigned depth = 1;
    unsigned n;
    do {
        for (n = 0;  n < 8;  n++) {
            if (onode->children[n]) {
                if (0 < compare(&list->record, context, &onode->children[n]->bound)) {
                    onode = onode->children[n];
                    depth++;
                    break;
                }
            } else {
                cdpOctreeBound bound;
                bound.subwide = onode->bound.subwide / 2.0f;
                assert(bound.subwide > EPSILON);

                switch (n) {
                  case 0:   BOUND_CENTER_QUADRANT(bound, onode, +, +, +);   break;
                  case 1:   BOUND_CENTER_QUADRANT(bound, onode, +, -, +);   break;
                  case 2:   BOUND_CENTER_QUADRANT(bound, onode, -, -, +);   break;
                  case 3:   BOUND_CENTER_QUADRANT(bound, onode, -, +, +);   break;
                  case 4:   BOUND_CENTER_QUADRANT(bound, onode, +, +, -);   break;
                  case 5:   BOUND_CENTER_QUADRANT(bound, onode, +, -, -);   break;
                  case 6:   BOUND_CENTER_QUADRANT(bound, onode, -, -, -);   break;
                  case 7:   BOUND_CENTER_QUADRANT(bound, onode, -, +, -);   break;
                }

                if (0 < compare(&list->record, context, &bound)) {
                    onode->children[n] = octree_node_new(onode, &bound, n);
                    onode = onode->children[n];
                    depth++;
                    break;
                }
            }
        }
    } while (n < 8);

    // Insert list item
    list->onode = onode;
    list->next  = onode->list;
    if (list->next)
        list->next->prev = list;
    onode->list = list;

    if (octree->depth < depth)
        octree->depth = depth;

    return &list->record;
}


static inline cdpRecord* octree_first(cdpOctree* octree) {
    cdpOctreeNode* onode = &octree->root;
    for (;;) {
        if (onode->list)
            return &onode->list->record;

        // Check children for a node with records
        bool hasChildren = false;
        for (unsigned n = 0;  n < 8;  n++) {
            if (onode->children[n]) {
                onode = onode->children[n];
                hasChildren = true;
                break;
            }
        }
        if (!hasChildren)
            break;
    }

    return NULL;
}


static inline cdpRecord* octree_last(cdpOctree* octree) {
    cdpOctreeNode* onode = &octree->root;
    cdpOctreeList* last  = NULL;
    for (;;) {
        // Find the last node in the current node's list
        if (onode->list) {
            for (last = onode->list;  last->next;  last = last->next);
        }

        // Check children for deeper nodes
        bool hasChildren = false;
        for (int i = 7;  i >= 0;  i--) {    // Start from the last child.
            if (onode->children[i]) {
                onode = onode->children[i];
                hasChildren = true;
                break;
            }
        }
        if (!hasChildren) {
            break;
        }
    }

    return last? &last->record: NULL;
}


static inline bool octree_traverse(cdpOctree* octree, cdpTraverse func, void* context, cdpEntry* entry) {
    assert(octree && func);

    cdpOctreeNode* onode = &octree->root;

    entry->parent = octree->store.owner;
    entry->depth  = 0;
    do {
        // Process all records in the current node
        for (cdpOctreeList* list = onode->list;  list;  list = list->next) {
            if (entry->next) {
                entry->prev   = entry->record;
                entry->record = entry->next;
                entry->next   = &list->record;
                if (!func(entry, context))
                    return true;
            } else {
                entry->next = &list->record;
            }
            entry->position++;
        }

        // Move to the first child node if available
        bool hasChild = false;
        for (unsigned n = 0;  n < 8;  n++) {
            if (onode->children[n]) {
                onode = onode->children[n];
                hasChild = true;
                break;
            }
        }
        if (!hasChild) {
            // Backtrack to the next sibling or parent
            while (onode && onode->parent) {
                unsigned n = onode->index + 1;
                while (n < 8  &&  !onode->parent->children[n]) {
                    n++;
                }
                if (n < 8) {
                    onode = onode->parent->children[n];
                    break;
                }

                onode = onode->parent;
            }
        }
    } while (onode);

    entry->prev   = entry->record;
    entry->record = entry->next;
    entry->next   = NULL;
    return func(entry, context);
}


static inline cdpRecord* octree_find_by_name(cdpOctree* octree, cdpID id) {
    cdpEntry entry = {0};
    if (!octree_traverse(octree, (cdpFunc) rb_traverse_func_break_at_name, cdp_v2p(id), &entry))
        return entry.record;
    return NULL;
}


static inline cdpRecord* octree_find_by_key(cdpOctree* octree, cdpRecord* key, cdpCompare compare, void* context) {
    cdpEntry entry = {0};
    if (!octree_traverse(octree, (cdpFunc) compare, key, &entry))
        return entry.record;
    return NULL;
}


static inline cdpRecord* octree_find_by_position(cdpOctree* octree, size_t position) {
    cdpEntry entry = {0};
    if (!octree_traverse(octree, (void*) rb_traverse_func_break_at_position, cdp_v2p(position), &entry))
        return entry.record;
    return NULL;
}


static inline cdpRecord* octree_prev(cdpRecord* record) {
    cdpOctreeList* list = octree_list_from_record(record);
    if (list->prev)
        return &list->prev->record;

    // Find the previous sibling node or parent with records
    cdpOctreeNode* onode = list->onode;
    while (onode) {
        unsigned index = onode->index;
        onode = onode->parent;
        if (!onode)
            break; // Root node reached.

        for (int i = (int)index - 1;  i >= 0;  i--) {
            if (onode->children[i]) {
                cdpOctreeNode* sibling = onode->children[i];
                while (sibling) {
                    // Find the last record in this subtree
                    if (sibling->list) {
                        cdpOctreeList* last = sibling->list;
                        while (last->next) {
                            last = last->next;
                        }
                        return &last->record;
                    }

                    // Descend into the last child node
                    bool hasChildren = false;
                    for (int j = 7;  j >= 0;  j--) {
                        if (sibling->children[j]) {
                            sibling = sibling->children[j];
                            hasChildren = true;
                            break;
                        }
                    }
                    if (!hasChildren)
                        break;
                }
            }
        }
    }

    return NULL;
}


static inline cdpRecord* octree_next(cdpRecord* record) {
    cdpOctreeList* list = octree_list_from_record(record);
    if (list->next)
        return &list->next->record;

    // Find the next sibling node or parent with records
    cdpOctreeNode* onode = list->onode;
    while (onode) {
        for (unsigned n = onode->index + 1;  n < 8;  n++) {
            if (onode->parent->children[n]) {
                cdpOctreeNode* next = onode->parent->children[n];
                while (next) {
                    if (next->list)
                        return &next->list->record;

                    bool hasChildren = false;
                    for (unsigned m = 0;  m < 8;  m++) {
                        if (next->children[m]) {
                            next = next->children[m];
                            hasChildren = true;
                            break;
                        }
                    }
                    if (!hasChildren)
                        break;
                }
            }
        }

        onode = onode->parent;
    }

    return NULL;
}


static inline void octree_remove_record(cdpOctree* octree, cdpRecord* record) {
    cdpOctreeList* list  = octree_list_from_record(record);
    cdpOctreeNode* onode = list->onode;

    // Remove list item
    if (list->prev) {
        list->prev->next = list->next;
    } else {
        list->onode->list = list->next;
    }
    if (list->next) {
        list->next->prev = list->prev;
    }

    cdp_free(list);

    // Remove empty nodes
    for(;;) {
        if (onode->list)
            break;

        bool hasChildren = false;
        for (unsigned n = 0;  n < 8;  n++) {
            if (onode->children[n]) {
                hasChildren = true;
                break;
            }
        }
        if (hasChildren)
            break;

        // If the node is empty, remove it
        cdpOctreeNode* parent = onode->parent;
        if (parent) {
            parent->children[onode->index] = NULL;
            cdp_free(onode);
            onode = parent;
        } else {
            octree->depth = 1;
            break;
        }
    }
}


static inline void octree_take(cdpOctree* octree, cdpRecord* target) {
    cdpRecord* last = octree_last(octree);
    cdp_record_transfer(last, target);
    octree_remove_record(octree, last);
}


static inline void octree_pop(cdpOctree* octree, cdpRecord* target) {
    cdpRecord* first = octree_first(octree);
    cdp_record_transfer(first, target);
    octree_remove_record(octree, first);
}


#define OCTREE_MIN_DEPTH    128

static inline void octree_del_all_children(cdpOctree* octree) {
    size_t      stackSize = octree->depth * sizeof(void*);
    cdpOctreeNode** stack = (octree->depth > OCTREE_MIN_DEPTH)? cdp_malloc(stackSize): cdp_alloca(stackSize);
    unsigned          top = 0;

    // Push all root's children onto the stack
    for (unsigned n = 0;  n < 8;  n++) {
        if (octree->root.children[n]) {
            stack[top++] = octree->root.children[n];
            octree->root.children[n] = NULL;
        }
    }

    // Delete all child nodes
    while (top > 0) {
        cdpOctreeNode* onode = stack[--top];

        // Free all records in this node
        cdpOctreeList* list = onode->list;
        while (list) {
            cdpOctreeList* next = list->next;
            cdp_record_finalize(&list->record);
            cdp_free(list);
            list = next;
        }

        // Push all children of the current node onto the stack
        for (unsigned n = 0;  n < 8;  n++) {
            if (onode->children[n]) {
                stack[top++] = onode->children[n];
                onode->children[n] = NULL;
            }
        }

        cdp_free(onode);
    }

    if (octree->depth > OCTREE_MIN_DEPTH)
        cdp_free(stack);

    octree->depth = 1;
}

