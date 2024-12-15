#pragma once

#include <vector>

#include <gst/gst.h>
#include <gstnvdsmeta.h>

namespace yz::common::gst {

struct LinkData {
    GstPad* sourcePeerPad = nullptr;
    GstPad* sinkPeerPad = nullptr;
};

struct UnlinkResult {
    bool success = false;
    LinkData linkData;
};

bool enumerateChildren(GstBin* bin, std::vector<GstElement*>& elements);

bool enumerateSourcePads(GstElement* element, std::vector<GstPad*>& pads);

bool enumerateSinkPads(GstElement* element, std::vector<GstPad*>& pads);

UnlinkResult unlinkElement(GstElement* element);

bool relinkElement(GstElement* element, LinkData linkData);

template <class TClass, class TResult>
bool enumerateMetadata(TClass* input, std::vector<TResult*>& result);

template <>
bool enumerateMetadata<GstBuffer, NvDsFrameMeta>(GstBuffer* buffer, std::vector<NvDsFrameMeta*>& result);

template <>
bool enumerateMetadata<NvDsBatchMeta, NvDsFrameMeta>(NvDsBatchMeta* meta, std::vector<NvDsFrameMeta*>& result);

template <>
bool enumerateMetadata<NvDsFrameMeta, NvDsDisplayMeta>(NvDsFrameMeta* meta, std::vector<NvDsDisplayMeta*>& result);

template <>
bool enumerateMetadata<NvDsFrameMeta, NvDsUserMeta>(NvDsFrameMeta* meta, std::vector<NvDsUserMeta*>& result);

template <>
bool enumerateMetadata<NvDsFrameMeta, NvDsObjectMeta>(NvDsFrameMeta* meta, std::vector<NvDsObjectMeta*>& result);

template <>
bool enumerateMetadata<NvDsObjectMeta, NvDsClassifierMeta>(NvDsObjectMeta* meta, std::vector<NvDsClassifierMeta*>& result);

template <>
bool enumerateMetadata<NvDsObjectMeta, NvDsUserMeta>(NvDsObjectMeta* meta, std::vector<NvDsUserMeta*>& result);

}  // namespace yz::common::gst
