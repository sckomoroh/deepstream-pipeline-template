#include "gst-tools.h"

#include <vector>

namespace yz::common::gst {

bool enumerateChildren(GstBin* bin, std::vector<GstElement*>& elements) {
    GstIterator* iter = gst_bin_iterate_elements(GST_BIN(bin));
    GValue item = G_VALUE_INIT;
    GValue* value = &item;
    GstIteratorResult res;
    bool result = true;

    while ((res = gst_iterator_next(iter, value)) != GST_ITERATOR_DONE) {
        if (res == GST_ITERATOR_OK) {
            GstElement* element = GST_ELEMENT(g_value_get_object(value));
            elements.push_back(element);
            g_value_reset(value);
        } else if (res == GST_ITERATOR_ERROR) {
            elements.clear();
            result = false;
            break;
        };
    }

    g_value_unset(value);  // Unset the GValue
    gst_iterator_free(iter);

    return result;
}

bool enumerateSourcePads(GstElement* element, std::vector<GstPad*>& pads) {
    auto iter = gst_element_iterate_src_pads(element);
    GValue item = G_VALUE_INIT;
    GValue* value = &item;
    GstIteratorResult res;
    auto result = true;

    while ((res = gst_iterator_next(iter, value)) != GST_ITERATOR_DONE) {
        if (res == GST_ITERATOR_OK) {
            GstPad* pad = GST_PAD(g_value_get_object(value));
            pads.push_back(pad);
            g_value_reset(value);
        } else if (res == GST_ITERATOR_ERROR) {
            pads.clear();
            result = false;
            break;
        }
    }

    g_value_unset(value);
    gst_iterator_free(iter);

    return result;
}

bool enumerateSinkPads(GstElement* element, std::vector<GstPad*>& pads) {
    auto iter = gst_element_iterate_sink_pads(element);
    GValue item = G_VALUE_INIT;
    GValue* value = &item;
    GstIteratorResult res;
    auto result = true;

    while ((res = gst_iterator_next(iter, value)) != GST_ITERATOR_DONE) {
        if (res == GST_ITERATOR_OK) {
            GstPad* pad = GST_PAD(g_value_get_object(value));
            pads.push_back(pad);
            g_value_reset(value);
        } else if (res == GST_ITERATOR_ERROR) {
            pads.clear();
            result = false;
            break;
        }
    }

    g_value_unset(value);
    gst_iterator_free(iter);

    return result;
}

struct ProbePadData {
    GstElement* element = nullptr;
    GstPad* sourcePad;
    GstPad* sinkPad;
    GstPad* sourcePeerPad;
    GstPad* sinkPeerPad;
};

GstPadProbeReturn unlinkPadCallback(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) {
    auto probePadData = static_cast<ProbePadData*>(user_data);

    do {
        if (probePadData->sinkPad != nullptr) {
            if (gst_pad_unlink(probePadData->sinkPeerPad, probePadData->sinkPad) == false) {
                GST_ERROR("Failed to unlink sink pad for %s", gst_element_get_name(probePadData->element));
                break;
            }
        }

        if (probePadData->sourcePad != nullptr) {
            if (gst_pad_unlink(probePadData->sourcePad, probePadData->sourcePeerPad) == false) {
                GST_ERROR("Failed to unlink source pad for %s", gst_element_get_name(probePadData->element));
                break;
            }
        }

        auto parent = gst_element_get_parent(probePadData->element);
        if (parent == nullptr) {
            GST_ERROR("Failed to get parent for %s", gst_element_get_name(probePadData->element));
            break;
        }

        if (gst_element_set_state(probePadData->element, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE) {
            GST_ERROR("Failed to change state to NULL for %s", gst_element_get_name(probePadData->element));
            break;
        }

        if (gst_bin_remove(GST_BIN(parent), probePadData->element) == false) {
            GST_ERROR("Failed to remove element %s from parent", gst_element_get_name(probePadData->element));
            break;
        }

        if (probePadData->sinkPeerPad != nullptr && probePadData->sourcePeerPad != nullptr) {
            if (gst_pad_link(probePadData->sinkPeerPad, probePadData->sourcePeerPad) != GST_PAD_LINK_OK) {
                GST_ERROR("Failed to link sibling of element %s", gst_element_get_name(probePadData->element));
                break;
            }
        }

    } while (false);

    if (probePadData->sinkPeerPad != nullptr) {
        gst_object_unref(probePadData->sinkPeerPad);
    }

    if (probePadData->sourcePeerPad != nullptr) {
        gst_object_unref(probePadData->sourcePeerPad);
    }

    delete probePadData;

    return GST_PAD_PROBE_REMOVE;
}

UnlinkResult unlinkElement(GstElement* element) {
    UnlinkResult result;

    std::vector<GstPad*> sourcePads, sinkPads;
    if (enumerateSinkPads(element, sinkPads) == false) {
        GST_ERROR("Failed to enumerate sink pads for %s", gst_element_get_name(element));
        return result;
    }

    if (sinkPads.size() > 1) {
        GST_ERROR("Unable to unlink element with more than 1 sink pad");
        return result;
    }

    GstPad* sinkPad = nullptr;
    if (sinkPads.empty() == false) {
        sinkPad = sinkPads[0];
    }

    if (enumerateSourcePads(element, sourcePads) == false) {
        GST_ERROR("Failed to enumerate source pads for %s", gst_element_get_name(element));
        return result;
    }

    if (sourcePads.size() > 1) {
        GST_ERROR("Unable to unlink element with more than 1 sink pad");
        return result;
    }

    GstPad* sourcePad = nullptr;
    if (sourcePads.empty() == false) {
        sourcePad = sourcePads[0];
    }

    if (sinkPad == nullptr && sourcePad == nullptr) {
        GST_ERROR("Source and sink pads not found for %s", gst_element_get_name(element));
        return result;
    }

    ProbePadData* probeData = new ProbePadData();
    probeData->element = element;
    probeData->sinkPad = sinkPad;
    probeData->sourcePad = sourcePad;
    probeData->sinkPeerPad = sinkPad == nullptr ? nullptr : gst_pad_get_peer(sinkPad);
    probeData->sourcePeerPad = sourcePad == nullptr ? nullptr : gst_pad_get_peer(sourcePad);

    if (sinkPad != nullptr) {
        // Use sink pad to probe
        gst_pad_add_probe(sinkPad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, unlinkPadCallback, probeData, nullptr);

    } else {
        // Use source pad to probe
        gst_pad_add_probe(sourcePad, GST_PAD_PROBE_TYPE_BLOCK_UPSTREAM, unlinkPadCallback, probeData, nullptr);
    }

    result.success = true;
    result.linkData.sinkPeerPad = sinkPad == nullptr ? nullptr : gst_pad_get_peer(sinkPad);
    result.linkData.sourcePeerPad = sourcePad == nullptr ? nullptr : gst_pad_get_peer(sourcePad);

    return result;
}

bool relinkElement(GstElement* element, LinkData linkData) { 
    // TODO:
    
    return false; }

template <>
bool enumerateMetadata<GstBuffer, NvDsFrameMeta>(GstBuffer* buffer, std::vector<NvDsFrameMeta*>& result) {
    NvDsBatchMeta* batchMeta = gst_buffer_get_nvds_batch_meta(buffer);
    if (batchMeta == nullptr) {
        return false;
    }

    return enumerateMetadata(batchMeta, result);
}

template <>
bool enumerateMetadata<NvDsBatchMeta, NvDsFrameMeta>(NvDsBatchMeta* meta, std::vector<NvDsFrameMeta*>& result) {
    for (NvDsMetaList* item = meta->frame_meta_list; item != nullptr; item = item->next) {
        NvDsFrameMeta* frameMeta = static_cast<NvDsFrameMeta*>(item->data);
        result.push_back(frameMeta);
    }

    return true;
}

template <>
bool enumerateMetadata<NvDsFrameMeta, NvDsDisplayMeta>(NvDsFrameMeta* meta, std::vector<NvDsDisplayMeta*>& result) {
    for (NvDsMetaList* item = meta->display_meta_list; item != nullptr; item = item->next) {
        NvDsDisplayMeta* displayMeta = static_cast<NvDsDisplayMeta*>(item->data);
        result.push_back(displayMeta);
    }

    return true;
}

template <>
bool enumerateMetadata<NvDsFrameMeta, NvDsUserMeta>(NvDsFrameMeta* meta, std::vector<NvDsUserMeta*>& result) {
    for (NvDsMetaList* item = meta->frame_user_meta_list; item != nullptr; item = item->next) {
        NvDsUserMeta* userMeta = static_cast<NvDsUserMeta*>(item->data);
        result.push_back(userMeta);
    }

    return true;
}

template <>
bool enumerateMetadata<NvDsFrameMeta, NvDsObjectMeta>(NvDsFrameMeta* meta, std::vector<NvDsObjectMeta*>& result) {
    for (NvDsMetaList* item = meta->obj_meta_list; item != nullptr; item = item->next) {
        NvDsObjectMeta* objectMeta = static_cast<NvDsObjectMeta*>(item->data);
        result.push_back(objectMeta);
    }

    return true;
}

template <>
bool enumerateMetadata<NvDsObjectMeta, NvDsClassifierMeta>(NvDsObjectMeta* meta,
                                                           std::vector<NvDsClassifierMeta*>& result) {
    for (NvDsClassifierMetaList* item = meta->classifier_meta_list; item != nullptr; item = item->next) {
        NvDsClassifierMeta* classifierMeta = static_cast<NvDsClassifierMeta*>(item->data);
        result.push_back(classifierMeta);
    }

    return true;
}

template <>
bool enumerateMetadata<NvDsObjectMeta, NvDsUserMeta>(NvDsObjectMeta* meta, std::vector<NvDsUserMeta*>& result) {
    for (NvDsUserMetaList* item = meta->obj_user_meta_list; item != nullptr; item = item->next) {
        NvDsUserMeta* userMeta = static_cast<NvDsUserMeta*>(item->data);
        result.push_back(userMeta);
    }

    return true;
}

}  // namespace yz::common::gst
