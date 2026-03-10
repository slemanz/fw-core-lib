#include "CppUTest/TestHarness.h"

extern "C"
{
#include "shared/slist.h"
}

typedef struct
{
    uint32_t     id;
    slist_node_t node;
} item_t;

// Helper
#define MAKE_ITEM(var, _id) \
    item_t var; var.id = (_id); var.node.next = NULL

TEST_GROUP(SList)
{
    slist_head_t list;

    void setup()
    {
        slist_init(&list);
    }

    void teardown() {}
};

/* ---- Init -------------------------------------------------------- */

TEST(SList, EmptyAfterInit)
{
    CHECK(slist_empty(&list));
    UNSIGNED_LONGS_EQUAL(0, slist_count(&list));
}

/* ---- Push front -------------------------------------------------- */

TEST(SList, PushFrontOneElement)
{
    MAKE_ITEM(a, 1);
    slist_push_front(&list, &a.node);

    CHECK_FALSE(slist_empty(&list));
    UNSIGNED_LONGS_EQUAL(1, slist_count(&list));
}

TEST(SList, PushFrontOrderIsLIFO)
{
    MAKE_ITEM(a, 1);
    MAKE_ITEM(b, 2);

    slist_push_front(&list, &a.node);
    slist_push_front(&list, &b.node);

    item_t *first = container_of(list.first, item_t, node);
    UNSIGNED_LONGS_EQUAL(2, first->id);
}

/* ---- Push back --------------------------------------------------- */

TEST(SList, PushBackOrderIsFIFO)
{
    MAKE_ITEM(a, 1);
    MAKE_ITEM(b, 2);

    slist_push_back(&list, &a.node);
    slist_push_back(&list, &b.node);

    item_t *first = container_of(list.first, item_t, node);
    UNSIGNED_LONGS_EQUAL(1, first->id);
}


/* ---- Remove ------------------------------------------------------ */

TEST(SList, RemoveHead)
{
    MAKE_ITEM(a, 1);
    MAKE_ITEM(b, 2);

    slist_push_back(&list, &a.node);
    slist_push_back(&list, &b.node);

    slist_remove(&list, &a.node);

    UNSIGNED_LONGS_EQUAL(1, slist_count(&list));
    item_t *first = container_of(list.first, item_t, node);
    UNSIGNED_LONGS_EQUAL(2, first->id);
}

TEST(SList, RemoveMiddle)
{
    MAKE_ITEM(a, 1);
    MAKE_ITEM(b, 2);
    MAKE_ITEM(c, 3);

    slist_push_back(&list, &a.node);
    slist_push_back(&list, &b.node);
    slist_push_back(&list, &c.node);

    slist_remove(&list, &b.node);

    UNSIGNED_LONGS_EQUAL(2, slist_count(&list));

    /* Verifica que a -> c */
    POINTERS_EQUAL(&c.node, a.node.next);
}
