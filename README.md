dvdclipper
==========

DVD video clip detector application

This is a simple program that uses mplayer to help decide how to clip a video extracted from a DVD.  It can be passed the DVD device or an ISO file.

It extracts and analyzes multiple frames to compute what is the best clipping region.  It looks for the maximum amount that can be clipped without removing actual frame data.

It outputs what could be clipping parameters for ogmrip/shrip.  However, I don't think they are currently correct.

