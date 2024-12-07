#pragma once

#include <gst/gst.h>

namespace yz::tools {

void printPadInfo(GstPad* pad);
void dumpPads(GstElement* element);

}  // namespace yz::tools
