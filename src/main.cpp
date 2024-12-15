#include "engine/pipeline/PiplineFactory.h"

#include "common/gst-tools.h"

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <gstnvdsmeta.h>
#include <nvbufsurface.h>
#include <nvdsmeta_schema.h>

#include <nvds_obj_encode.h>

#include "user-data.h"

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

gpointer meta_copy_func_custom(gpointer data, gpointer user_data) {
    NvDsUserMeta* user_meta = (NvDsUserMeta*)data;
    NvDsCustomMsgInfo* srcMeta = (NvDsCustomMsgInfo*)user_meta->user_meta_data;
    NvDsCustomMsgInfo* dstMeta = NULL;

    dstMeta = (NvDsCustomMsgInfo*)g_memdup2(srcMeta, sizeof(NvDsCustomMsgInfo));

    if (srcMeta->message) {
        dstMeta->message = (gpointer)g_strdup((const char*)srcMeta->message);
    }

    dstMeta->size = srcMeta->size;

    return dstMeta;
}

void meta_free_func_custom(gpointer data, gpointer user_data) {
    NvDsUserMeta* user_meta = (NvDsUserMeta*)data;
    NvDsCustomMsgInfo* srcMeta = (NvDsCustomMsgInfo*)user_meta->user_meta_data;

    if (srcMeta->message) {
        g_free(srcMeta->message);
    }

    srcMeta->size = 0;

    g_free(user_meta->user_meta_data);
}

GstPadProbeReturn inferSrcPadProbe(GstPad* pad, GstPadProbeInfo* info, gpointer ctx) {
    using namespace yz::common;

    GstBuffer* buf = (GstBuffer*)info->data;
    std::vector<NvDsFrameMeta*> frameMetadatas;
    if (gst::enumerateMetadata(buf, frameMetadatas) == false) {
        // TODO: error
        int a = 0;
    }

    for (const auto& frameMetadata : frameMetadatas) {
        std::vector<NvDsObjectMeta*> objectMetadatas;
        if (gst::enumerateMetadata(frameMetadata, objectMetadatas) == false) {
            int a = 0;
        }

        for (const auto& objectMeta : objectMetadatas) {
            std::vector<NvDsClassifierMeta*> clsMetadatas;
            if (gst::enumerateMetadata(objectMeta, clsMetadatas) == false) {
                int a = 0;
            }
        }
    }
    // NvDsBatchMeta* batch_meta = gst_buffer_get_nvds_batch_meta(buf);

    // for (NvDsMetaList* frameListItem = batch_meta->frame_meta_list; frameListItem != NULL;
    //      frameListItem = frameListItem->next) {
    //     NvDsFrameMeta* frameMeta = (NvDsFrameMeta*)(frameListItem->data);

    // NvDsUserMeta* user_event_meta_custom = nvds_acquire_user_meta_from_pool(batch_meta);
    //     if (user_event_meta_custom == nullptr) {
    //         g_printerr("Failed to asquire user meta from batch");
    //         return GST_PAD_PROBE_OK;
    //     }

    //     UserData* user_data = (UserData*)g_malloc0(sizeof(UserData));
    //     user_data->frameNumber = frameMeta->frame_num;

    //     auto size = snprintf(nullptr, 0, "Frame Number: %d", user_data->frameNumber);
    //     user_data->frameStr = (const char*)g_malloc0(size + 1);
    //     memset((void*)user_data->frameStr, 0, size + 1);
    //     sprintf((char*)user_data->frameStr, "Frame Number: %d", user_data->frameNumber);

    // NvDsCustomMsgInfo* msg_custom_meta = (NvDsCustomMsgInfo*)g_malloc0(sizeof(NvDsCustomMsgInfo));
    //     msg_custom_meta->size = sizeof(UserData);
    //     msg_custom_meta->message = user_data;
    // user_event_meta_custom->user_meta_data = (void*)msg_custom_meta;
    //     user_event_meta_custom->base_meta.meta_type = NVDS_CUSTOM_MSG_BLOB;
    //     user_event_meta_custom->base_meta.copy_func = (NvDsMetaCopyFunc)meta_copy_func_custom;
    //     user_event_meta_custom->base_meta.release_func = (NvDsMetaReleaseFunc)meta_free_func_custom;

    //     nvds_add_user_meta_to_frame(frameMeta, user_event_meta_custom);

    //     for (NvDsMetaList* objListItem = frameMeta->obj_meta_list; objListItem != NULL;
    //          objListItem = frameListItem->next) {
    //         NvDsObjectMeta* objectMeta = (NvDsObjectMeta*)(objListItem->data);

    //         int k = 0;
    //     }
    // }

    return GST_PAD_PROBE_OK;
}

int main(int argc, char* argv[]) {
    using namespace yz::common;

    gst_init(0, nullptr);

    auto loop = g_main_loop_new(nullptr, false);

    yz::PiplineFactory factory{};

    YAML::Node root = YAML::LoadFile("config/ds-app-configl.yaml");
    auto pipeline = factory.createPipeLine(root);
    if (pipeline == nullptr) {
        return -1;
    }

    auto inferElement = gst_bin_get_by_name(GST_BIN(pipeline), "yz-infer-0-nvinfer");
    auto inferSrcPad = gst_element_get_static_pad(inferElement, "src");

    NvDsObjEncCtxHandle obj_ctx_handle = nvds_obj_enc_create_context(0);

    auto probeRes =
        gst_pad_add_probe(inferSrcPad, GST_PAD_PROBE_TYPE_BUFFER, inferSrcPadProbe, obj_ctx_handle, nullptr);

    auto bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    auto busWatchId = gst_bus_add_watch(bus, busCall, loop);
    gst_object_unref(bus);

    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

    // gst_debug_bin_to_dot_file_with_ts(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "graph");

    g_main_loop_run(loop);
    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_READY);

    return 0;
}
