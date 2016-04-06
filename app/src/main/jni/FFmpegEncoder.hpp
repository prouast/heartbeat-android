//
//  FFmpegEncoder.hpp
//  Heartbeat
//
//  Created by Philipp Rouast on 26/03/2016.
//  Copyright © 2016 Philipp Roüast. All rights reserved.
//

#ifndef FFmpegEncoder_hpp
#define FFmpegEncoder_hpp

#include <stdio.h>
#include <string>
#include <queue>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class FFmpegEncoder {

public:
    
    // Constructor
    FFmpegEncoder() : fmt(NULL), oc(NULL), st(NULL), imgConvertCtx(NULL), dst(NULL) {;}
    
    // Open file
    bool OpenFile(const char *filename, int width, int height, int bitrate, int framerate);
    
    // Write next frame.
    void WriteFrame(uint8_t *dataAddr, int64_t time);
    
    // Write buffered frames.
    void WriteBufferedFrames();
    
    // Close file and free resourses.
    void CloseFile();
    
private:
    
    int frame_count;
    int write_count;
    int buffer_count;
    std::queue<int64_t> pts_queue;
    
    AVFrame *dst;
    
    AVOutputFormat *fmt;                //
    AVFormatContext* oc;                //
    AVStream *st;                       // FFmpeg stream
    struct SwsContext *imgConvertCtx;   // FFmpeg context convert image.
};

#endif /* FFmpegEncoder_hpp */
