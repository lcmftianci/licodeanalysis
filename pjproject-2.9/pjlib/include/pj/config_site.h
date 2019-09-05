// Uncomment to get minimum footprint (suitable for 1-2 concurrent calls only)
//#define PJ_CONFIG_MINIMAL_SIZE

// Uncomment to get maximum performance
//#define PJ_CONFIG_MAXIMUM_SPEED

#include <pj/config_site_sample.h> 

#define PJMEDIA_HAS_FFMPEG_VID_CODEC 1
#define PJMEDIA_HAS_LIBAVFORMAT 1
//±‡“ÎŒ Ã‚ —‘Œ‡ËÚ
#define 	FF_INPUT_BUFFER_PADDING_SIZE   32