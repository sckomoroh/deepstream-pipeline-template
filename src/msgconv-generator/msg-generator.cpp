#include "msg-generator.h"

#include <gst/gst.h>

#include <gstnvdsmeta.h>
#include <json/json.h>
#include <nvdsmeta_schema.h>

#include "user-data.h"

NvDsMsg2pCtx* nvds_msg2p_ctx_create(const gchar* file, NvDsPayloadType type) {
    g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_ctx_create\n");

    auto context = new NvDsMsg2pCtx();
    context->payloadType = type;
    g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_ctx_create: Payload type: %d\n", type);

    return context;
}

void nvds_msg2p_ctx_destroy(NvDsMsg2pCtx* ctx) {
    g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_ctx_destroy\n");

    delete ctx;
}

NvDsPayload* nvds_msg2p_generate(NvDsMsg2pCtx* ctx, NvDsEvent* events, guint size) {
    g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_generate\n");
    return nullptr;
}

NvDsPayload** nvds_msg2p_generate_multiple(NvDsMsg2pCtx* ctx, NvDsEvent* events, guint size, guint* payloadCount) {
    g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_generate_multiple\n");
    return nullptr;
}

NvDsPayload* nvds_msg2p_generate_new(NvDsMsg2pCtx* ctx, void* metadataInfo) {
    g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_generate_new\n");
    NvDsMsg2pMetaInfo* meta_info = (NvDsMsg2pMetaInfo*)metadataInfo;

    NvDsPayload* payload = nullptr;

    if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM_MINIMAL) {
        g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_generate_new: Generate custom payload\n");
        NvDsFrameMeta* frame_meta = (NvDsFrameMeta*)meta_info->frameMeta;
        NvDsObjectMeta* obj_meta = (NvDsObjectMeta*)meta_info->objMeta;

        // {
        //     NvDsUserMeta* userMeta = (NvDsUserMeta*)frame_meta->frame_user_meta_list->data;
        //     auto userMetaData = (NvDsCustomMsgInfo*)userMeta->user_meta_data;
        //     UserData* userData = (UserData*)userMetaData->message;
        //     int g = 0;
        // }

        g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_generate_new: enumerate frame objects meta list\n");
        NvDsObjectMetaList* obj_meta_list_iter = frame_meta->obj_meta_list;

        Json::Value root;
        root["objects"] = Json::Value(Json::arrayValue);
        root["frame_num"] = frame_meta->frame_num;
        root["source_id"] = frame_meta->source_id;
        root["ntp_timestamp"] = frame_meta->ntp_timestamp;

        for (; obj_meta_list_iter != nullptr; obj_meta_list_iter = obj_meta_list_iter->next) {
            NvDsObjectMeta* object_meta = (NvDsObjectMeta*)obj_meta_list_iter->data;
            Json::Value object;
            object["class_id"] = object_meta->class_id;
            object["obj_label"] = object_meta->obj_label;
            object["confidence"] = object_meta->confidence;
            Json::Value bbox;
            bbox["height"] = object_meta->detector_bbox_info.org_bbox_coords.height;
            bbox["left"] = object_meta->detector_bbox_info.org_bbox_coords.left;
            bbox["top"] = object_meta->detector_bbox_info.org_bbox_coords.top;
            bbox["width"] = object_meta->detector_bbox_info.org_bbox_coords.width;
            object["bbox"] = bbox;

            root["objects"].append(object);
        }

        Json::StreamWriterBuilder writer;
        writer["indentation"] = "  ";  // Add pretty-print indentation
        std::string jsonString = Json::writeString(writer, root);

        payload = (NvDsPayload*)g_malloc0(sizeof(NvDsPayload));
        payload->payload = g_memdup2(jsonString.c_str(), jsonString.length());
        payload->payloadSize = jsonString.length();

    } else {
        g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_generate_new: Only NVDS_PAYLOAD_DEEPSTREAM_MINIMAL schema supported\n");
    }

    return payload;
}

NvDsPayload** nvds_msg2p_generate_multiple_new(NvDsMsg2pCtx* ctx, void* metadataInfo, guint* payloadCount) {
    g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_generate_multiple_new\n");
    return nullptr;
}

void nvds_msg2p_release(NvDsMsg2pCtx* ctx, NvDsPayload* payload) {
    g_printerr("[DEBUG] [MSG2P]  nvds_msg2p_release \n");
    g_free(payload->payload);
    g_free(payload);
}
