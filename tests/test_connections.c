#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include "test.h"
#include "../include/canvas.h"
#include "../include/types.h"

int main(void) {
    TEST_START();

    TEST("Connection initialization in canvas") {
        Canvas canvas;
        int result = canvas_init(&canvas, 200.0, 100.0);

        ASSERT_EQ(result, 0, "canvas_init returns 0");
        ASSERT_EQ(canvas.conn_count, 0, "Initial connection count is 0");
        ASSERT_EQ(canvas.conn_capacity, INITIAL_CONNECTION_CAPACITY, "Initial capacity is INITIAL_CONNECTION_CAPACITY");
        ASSERT_EQ(canvas.next_conn_id, 1, "Next connection ID starts at 1");
        ASSERT_NOT_NULL(canvas.connections, "Connections array is allocated");
        ASSERT(!canvas.conn_mode.active, "Connection mode is not active initially");
        ASSERT_EQ(canvas.conn_mode.source_box_id, -1, "No source box initially");

        canvas_cleanup(&canvas);
    }

    TEST("Adding a connection") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 5, "Box 2");

        int conn_id = canvas_add_connection(&canvas, id1, id2);

        ASSERT(conn_id > 0, "Connection ID is positive");
        ASSERT_EQ(canvas.conn_count, 1, "Connection count is 1");

        Connection *conn = canvas_get_connection(&canvas, conn_id);
        ASSERT_NOT_NULL(conn, "Can retrieve connection");
        ASSERT_EQ(conn->source_id, id1, "Source ID is correct");
        ASSERT_EQ(conn->dest_id, id2, "Destination ID is correct");
        ASSERT_EQ(conn->color, CONNECTION_COLOR_DEFAULT, "Default color is cyan");

        canvas_cleanup(&canvas);
    }

    TEST("Adding multiple connections") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 5, "Box 2");
        int id3 = canvas_add_box(&canvas, 90.0, 30.0, 30, 5, "Box 3");

        int conn1 = canvas_add_connection(&canvas, id1, id2);
        int conn2 = canvas_add_connection(&canvas, id2, id3);
        int conn3 = canvas_add_connection(&canvas, id1, id3);

        ASSERT(conn1 > 0, "First connection created");
        ASSERT(conn2 > 0, "Second connection created");
        ASSERT(conn3 > 0, "Third connection created");
        ASSERT_EQ(canvas.conn_count, 3, "Connection count is 3");

        /* Verify all connections are retrievable */
        ASSERT_NOT_NULL(canvas_get_connection(&canvas, conn1), "Can get connection 1");
        ASSERT_NOT_NULL(canvas_get_connection(&canvas, conn2), "Can get connection 2");
        ASSERT_NOT_NULL(canvas_get_connection(&canvas, conn3), "Can get connection 3");

        canvas_cleanup(&canvas);
    }

    TEST("Prevent duplicate connections") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 5, "Box 2");

        int conn1 = canvas_add_connection(&canvas, id1, id2);
        ASSERT(conn1 > 0, "First connection succeeds");

        /* Try to add same connection again */
        int conn2 = canvas_add_connection(&canvas, id1, id2);
        ASSERT_EQ(conn2, -1, "Duplicate connection rejected");
        ASSERT_EQ(canvas.conn_count, 1, "Still only 1 connection");

        /* Try reverse direction (also a duplicate) */
        int conn3 = canvas_add_connection(&canvas, id2, id1);
        ASSERT_EQ(conn3, -1, "Reverse duplicate connection rejected");
        ASSERT_EQ(canvas.conn_count, 1, "Still only 1 connection");

        canvas_cleanup(&canvas);
    }

    TEST("Prevent self-connections") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");

        int conn = canvas_add_connection(&canvas, id1, id1);
        ASSERT_EQ(conn, -1, "Self-connection rejected");
        ASSERT_EQ(canvas.conn_count, 0, "No connections created");

        canvas_cleanup(&canvas);
    }

    TEST("Connection to non-existent box fails") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");

        int conn = canvas_add_connection(&canvas, id1, 999);
        ASSERT_EQ(conn, -1, "Connection to non-existent box rejected");
        ASSERT_EQ(canvas.conn_count, 0, "No connections created");

        conn = canvas_add_connection(&canvas, 999, id1);
        ASSERT_EQ(conn, -1, "Connection from non-existent box rejected");
        ASSERT_EQ(canvas.conn_count, 0, "No connections created");

        canvas_cleanup(&canvas);
    }

    TEST("Removing a connection") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 5, "Box 2");
        int id3 = canvas_add_box(&canvas, 90.0, 30.0, 30, 5, "Box 3");

        int conn1 = canvas_add_connection(&canvas, id1, id2);
        int conn2 = canvas_add_connection(&canvas, id2, id3);

        ASSERT_EQ(canvas.conn_count, 2, "Have 2 connections");

        int result = canvas_remove_connection(&canvas, conn1);
        ASSERT_EQ(result, 0, "Remove succeeds");
        ASSERT_EQ(canvas.conn_count, 1, "Now have 1 connection");

        ASSERT_NULL(canvas_get_connection(&canvas, conn1), "Removed connection is gone");
        ASSERT_NOT_NULL(canvas_get_connection(&canvas, conn2), "Other connection still exists");

        canvas_cleanup(&canvas);
    }

    TEST("Remove non-existent connection") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int result = canvas_remove_connection(&canvas, 999);
        ASSERT_EQ(result, -1, "Remove non-existent returns -1");

        canvas_cleanup(&canvas);
    }

    TEST("Find connection between boxes") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 5, "Box 2");
        int id3 = canvas_add_box(&canvas, 90.0, 30.0, 30, 5, "Box 3");

        int conn = canvas_add_connection(&canvas, id1, id2);

        /* Find existing connection */
        int found = canvas_find_connection(&canvas, id1, id2);
        ASSERT_EQ(found, conn, "Found connection ID matches");

        /* Find non-existent connection */
        found = canvas_find_connection(&canvas, id1, id3);
        ASSERT_EQ(found, -1, "Non-existent connection returns -1");

        /* Find reverse direction (not found since connections are directional in storage) */
        found = canvas_find_connection(&canvas, id2, id1);
        ASSERT_EQ(found, -1, "Reverse direction not found");

        canvas_cleanup(&canvas);
    }

    TEST("Get all connections for a box") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 5, "Box 2");
        int id3 = canvas_add_box(&canvas, 90.0, 30.0, 30, 5, "Box 3");

        int conn1 = canvas_add_connection(&canvas, id1, id2);
        int conn2 = canvas_add_connection(&canvas, id3, id1);
        canvas_add_connection(&canvas, id2, id3);  /* Not involving id1 */

        int conn_ids[10];
        int count = canvas_get_box_connections(&canvas, id1, conn_ids, 10);

        ASSERT_EQ(count, 2, "Box 1 has 2 connections");

        /* Verify both connections are found */
        int found_conn1 = 0, found_conn2 = 0;
        for (int i = 0; i < count; i++) {
            if (conn_ids[i] == conn1) found_conn1 = 1;
            if (conn_ids[i] == conn2) found_conn2 = 1;
        }
        ASSERT(found_conn1, "Found connection 1");
        ASSERT(found_conn2, "Found connection 2");

        canvas_cleanup(&canvas);
    }

    TEST("Remove box also removes its connections") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 5, "Box 2");
        int id3 = canvas_add_box(&canvas, 90.0, 30.0, 30, 5, "Box 3");

        int conn1 = canvas_add_connection(&canvas, id1, id2);
        int conn2 = canvas_add_connection(&canvas, id2, id3);
        int conn3 = canvas_add_connection(&canvas, id1, id3);

        ASSERT_EQ(canvas.conn_count, 3, "Have 3 connections");

        /* Remove box 2 (involved in conn1 and conn2) */
        canvas_remove_box(&canvas, id2);

        ASSERT_EQ(canvas.conn_count, 1, "Only 1 connection remains");
        ASSERT_NULL(canvas_get_connection(&canvas, conn1), "Connection 1 removed");
        ASSERT_NULL(canvas_get_connection(&canvas, conn2), "Connection 2 removed");
        ASSERT_NOT_NULL(canvas_get_connection(&canvas, conn3), "Connection 3 still exists");

        canvas_cleanup(&canvas);
    }

    TEST("Connection mode - start and cancel") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");

        ASSERT(!canvas_in_connection_mode(&canvas), "Not in connection mode initially");

        canvas_start_connection(&canvas, id1);
        ASSERT(canvas_in_connection_mode(&canvas), "Now in connection mode");
        ASSERT_EQ(canvas.conn_mode.source_box_id, id1, "Source box set correctly");

        canvas_cancel_connection(&canvas);
        ASSERT(!canvas_in_connection_mode(&canvas), "No longer in connection mode");
        ASSERT_EQ(canvas.conn_mode.source_box_id, -1, "Source box cleared");

        canvas_cleanup(&canvas);
    }

    TEST("Connection mode - start and finish") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");
        int id2 = canvas_add_box(&canvas, 50.0, 50.0, 30, 5, "Box 2");

        canvas_start_connection(&canvas, id1);
        ASSERT(canvas_in_connection_mode(&canvas), "In connection mode");

        canvas_finish_connection(&canvas, id2);
        ASSERT(!canvas_in_connection_mode(&canvas), "No longer in connection mode");
        ASSERT_EQ(canvas.conn_count, 1, "Connection created");

        /* Verify the connection using API */
        int conn_ids[10];
        int count = canvas_get_box_connections(&canvas, id1, conn_ids, 10);
        ASSERT_EQ(count, 1, "Should have 1 connection");
        Connection *conn = canvas_get_connection(&canvas, conn_ids[0]);
        ASSERT_NOT_NULL(conn, "Can retrieve connection");
        ASSERT_EQ(conn->source_id, id1, "Source ID is correct");
        ASSERT_EQ(conn->dest_id, id2, "Destination ID is correct");

        canvas_cleanup(&canvas);
    }

    TEST("Connection mode - finish to same box cancels") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int id1 = canvas_add_box(&canvas, 10.0, 10.0, 30, 5, "Box 1");

        canvas_start_connection(&canvas, id1);
        canvas_finish_connection(&canvas, id1);  /* Same as source */

        ASSERT(!canvas_in_connection_mode(&canvas), "Mode cancelled");
        ASSERT_EQ(canvas.conn_count, 0, "No connection created");

        canvas_cleanup(&canvas);
    }

    TEST("Connection mode - start with invalid box") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        canvas_start_connection(&canvas, 999);
        ASSERT(!canvas_in_connection_mode(&canvas), "Cannot start with invalid box");

        canvas_cleanup(&canvas);
    }

    TEST("Connection array growth") {
        Canvas canvas;
        canvas_init(&canvas, 200.0, 100.0);

        int initial_capacity = canvas.conn_capacity;

        /* Create enough boxes for many connections */
        int box_ids[20];
        for (int i = 0; i < 20; i++) {
            box_ids[i] = canvas_add_box(&canvas, i * 10.0, i * 10.0, 20, 5, "Box");
        }

        /* Add more connections than initial capacity */
        for (int i = 0; i < initial_capacity + 5 && i < 19; i++) {
            canvas_add_connection(&canvas, box_ids[i], box_ids[i + 1]);
        }

        ASSERT(canvas.conn_count > initial_capacity, "More connections than initial capacity");
        ASSERT(canvas.conn_capacity > initial_capacity, "Capacity increased");

        /* Verify all connections are accessible */
        for (int i = 0; i < canvas.conn_count; i++) {
            ASSERT_NOT_NULL(&canvas.connections[i], "Can access connection after growth");
        }

        canvas_cleanup(&canvas);
    }

    TEST("NULL canvas safety") {
        /* These should not crash */
        int result = canvas_add_connection(NULL, 1, 2);
        ASSERT_EQ(result, -1, "Add connection to NULL canvas returns -1");

        result = canvas_remove_connection(NULL, 1);
        ASSERT_EQ(result, -1, "Remove connection from NULL canvas returns -1");

        Connection *conn = canvas_get_connection(NULL, 1);
        ASSERT_NULL(conn, "Get connection from NULL canvas returns NULL");

        result = canvas_find_connection(NULL, 1, 2);
        ASSERT_EQ(result, -1, "Find connection in NULL canvas returns -1");

        int conn_ids[10];
        int count = canvas_get_box_connections(NULL, 1, conn_ids, 10);
        ASSERT_EQ(count, 0, "Get box connections from NULL canvas returns 0");

        /* These should just be no-ops */
        canvas_remove_box_connections(NULL, 1);
        canvas_start_connection(NULL, 1);
        canvas_finish_connection(NULL, 1);
        canvas_cancel_connection(NULL);

        bool mode = canvas_in_connection_mode(NULL);
        ASSERT(!mode, "NULL canvas is not in connection mode");
    }

    TEST_END();
}
