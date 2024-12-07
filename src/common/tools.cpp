#include "tools.h"

#include <cstdio>

#include <stdexcept>

#include <gst/gst.h>

namespace yz::tools {

void printPadInfo(GstPad* pad) {
    gchar* pad_name;
    try {
        pad_name = gst_pad_get_name(pad);
    } catch (std::exception& ex) {
        int a = 0;
    }

    GstPadDirection direction = gst_pad_get_direction(pad);
    GstCaps* caps = gst_pad_query_caps(pad, nullptr);

    // Print pad information
    printf("Pad: %s\n", pad_name);
    printf("  Direction: %s\n", (direction == GST_PAD_SRC ? "SRC" : "SINK"));

    // Print capabilities
    printf("  Capabilities:\n");
    for (guint i = 0; i < gst_caps_get_size(caps); ++i) {
        GstStructure* structure = gst_caps_get_structure(caps, i);
        gchar* structure_str = gst_structure_to_string(structure);
        printf("    %s\n", structure_str);
        g_free(structure_str);
    }

    // Cleanup
    gst_caps_unref(caps);
    g_free((gchar*)pad_name);
}

void dumpPads(GstElement* element) {
    GstIterator* pad_iter = gst_element_iterate_pads(element);
    gboolean done = FALSE;
    while (!done) {
        GValue value;
        GstPad* pad = nullptr;
        switch (gst_iterator_next(pad_iter, &value)) {
        case GST_ITERATOR_OK:
            printPadInfo(GST_PAD(g_value_get_object(&value)));
            gst_object_unref(pad);
            break;
        case GST_ITERATOR_DONE:
            done = TRUE;
            break;
        case GST_ITERATOR_ERROR:
        case GST_ITERATOR_RESYNC:
            done = TRUE;
            break;
        }
    }

    gst_iterator_free(pad_iter);
    gst_object_unref(element);
}

}  // namespace yz::tools
