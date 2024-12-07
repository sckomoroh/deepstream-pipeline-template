#include "engine/PiplineFactory.h"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

gboolean busCall(GstBus* bus, GstMessage* msg, gpointer data) {
    GMainLoop* loop = (GMainLoop*)data;
    auto msgType = GST_MESSAGE_TYPE(msg);

    if (msgType != GST_MESSAGE_STATE_CHANGED && msgType != GST_MESSAGE_QOS) {
        int a = 0;
    }

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        g_print("End of stream\n");
        g_main_loop_quit(loop);
        break;
    case GST_MESSAGE_ERROR: {
        gchar* debug;
        GError* error;
        gst_message_parse_error(msg, &error, &debug);
        g_printerr("ERROR from element %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);
        if (debug)
            g_printerr("Error details: %s\n", debug);
        g_free(debug);
        g_error_free(error);
        g_main_loop_quit(loop);
        break;
    }

    default:
        break;
    }
    return TRUE;
}

int main(int argc, char* argv[]) {
    gst_init(0, nullptr);

    auto loop = g_main_loop_new(nullptr, false);

    yz::PiplineFactory factory;

    YAML::Node root = YAML::LoadFile("../config/ds-app-configl.yaml");
    auto pipeline = factory.createPipeLine(root);
    if (pipeline == nullptr) {
        return -1;
    }

    auto bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    auto busWatchId = gst_bus_add_watch(bus, busCall, loop);
    gst_object_unref(bus);

    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

    gst_debug_bin_to_dot_file_with_ts(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline-dump");
    // gst_debug_bin_to_dot_file_with_ts(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, filename);

    g_main_loop_run(loop);

    return 0;
}
