#!python3
"""
https://eleif.net/lcd-segmentation.html (2018-10-23)

Enumerating LCD segments with OpenCV

Intended to help with reverse-engineering an undocumented LCD.
The LCD needs to be controlled in order to activate all segments/commons sequentially.
Each segment is detected and labeled from the video.
The labels can be used to map segments to display elements like seven-segment digits.
The mapping then allows writing (reading) display contents.

This is a proof of concept, not an application for end users.

Run and hit space to process at near live speed or hit right-arrow to step through frames.
There are some hard-coded parameters (marked with #TODO) and also globals ...

Requirements / Documentation
- Python 3
  https://docs.python.org/3/
  https://www.python.org/downloads/
- NumPy
  https://docs.scipy.org/doc/numpy/
  pip install numpy [--upgrade]
- OpenCV
  https://docs.opencv.org/3.4/
  https://pypi.org/project/opencv-python/
  pip install opencv-python [--upgrade]
  [pip install opencv-contrib-python]

"""

# modules
import sys              # Python standard library
import numpy as np      # required by OpenCV
import cv2 as cv        # OpenCV 3


# settings
filename = "demo.mp4"   # encoding affects possible seek positions
startframe = 0          # !!
endframe =  71          # !!
threshold = 45          # !!

segments = []           # store detected segments
counter = 0             # count consecutive frames without segments


def main():
    """ top level """

    print(versions())
    print("filename", filename)

    # frame source and background subtraction frame
    init()

    # loop
    state = "pause"
    while state != "quit":
        # process frame?
        if state in ["play","next","fast"]:
            process()

        # user input
        key = cv.waitKeyEx( int(1000 / fps) )

        # handle user input
        state = state_machine(state, key)

    # close frame source
    deinit()



def init():
    """ initialise video source and empty frame for background subtraction """
    global cap, out, frame_first_gray, sum, fc, fps # TODO
    cap = cv.VideoCapture(filename)

    fps = cap.get(cv.CAP_PROP_FPS)
    fw  = int(cap.get(cv.CAP_PROP_FRAME_WIDTH))
    fh  = int(cap.get(cv.CAP_PROP_FRAME_HEIGHT))
    fc  = int(cap.get(cv.CAP_PROP_FRAME_COUNT))
    print("width:", fw)
    print("height:", fh)
    print("fps:", fps)
    print("frames:", fc)

    # add all detected segments
    sum = np.zeros((fh, fw), np.uint8) # grey

    # seek
    res = cap.set(cv.CAP_PROP_POS_FRAMES, startframe)
    print("starting at frame:", startframe, "OK" if res else "ERROR")

    if True: # TODO
        # use first frame as background
        res, frame_first = cap.read()
        print("read frame:", startframe, "OK" if res else "ERROR (try another frame or file format)")
        if not res:
            cap.release()
            cv.destroyAllWindows()
            sys.exit()
    else:
        # use image as background
        frame_first = cv.imread('background.png')
        frame_first = cv.cvtColor(frame_first, cv.COLOR_BGRA2BGR)
        if not frame_first.shape[:2] == (fh,fw):
            print("frame size mismatch", (fh,fw), frame_first.shape[:2])
            cap.release()
            cv.destroyAllWindows()
            sys.exit()

    # display reference image
    cv.imshow("First frame", frame_first)

    # convert to gray and blur a little to remove noise
    frame_first_gray = cv.cvtColor(frame_first, cv.COLOR_BGR2GRAY)
    #frame_first_gray = cv.medianBlur(frame_first_gray,5)
    frame_first_gray = cv.GaussianBlur(frame_first_gray, (5, 5), 0)



def deinit():
    """ deinitialise OpenCV  """
    cap.release()
    cv.destroyAllWindows()



def process():
    """ get and process one video frame """
    global sum, counter # TODO

    # get frame number
    fn = int(cap.get(cv.CAP_PROP_POS_FRAMES))

    # end of video reached: don't process, stop (and wait for user)
    if fn > endframe:
        state = "end"
        return

    # get frame
    res, frame = cap.read()
    if not res:
        print("ERROR (try another frame or file format)")
        deinit()
        sys.exit()  # TODO

    # convert to gray
    frame_gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

    # blur to reduce noise
    #frame_gray = cv.medianBlur(frame_gray,5)
    frame_gray = cv.GaussianBlur(frame_gray, (5, 5), 0)    # TODO parameter

    # subtract from reference frame
    difference = cv.absdiff(frame_first_gray, frame_gray)

    # threshold
    _, thresholded = cv.threshold(difference, threshold, 255, cv.THRESH_BINARY)

    # attempt to remove noise
    kernel = np.ones((3, 3), np.uint8)    # TODO parameter
    filtered = cv.erode(thresholded, kernel, iterations=1)

    # sum
    sum = cv.add(sum, filtered)

    # detect blobs
    img_contours, contours, hierarchy = cv.findContours(
        filtered, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)

    count = len(contours)

    if count > 0:
        # reset frame counter with no segments detected
        counter = 0

        # determine combined center of mass for all contours
        # do it manually because "bad" contours can cause moments to have invalid centroid
        c = 0
        cx = 0
        cy = 0
        for i, contour in enumerate(contours):
            # center of mass
            for p in contour:
                x = p[0][0]
                y = p[0][1]
                cx += x
                cy += y
                c += 1

        cx = int(cx/c)
        cy = int(cy/c)

        # "adaptive" positional limits
        if count > 1:
            # higher tolerance for labels (multiple blobs spread out)
            t = 50  # TODO parameter
        else:
            # normal tolerance for segments
            t = 25  # TODO parameter

        # prevent duplicates
        try:
            last = segments[-1]
            if last[0] in range(cx-t, cx+t) and last[1] in range(cy-t, cy+t):
                # detected previously
                pass
            else:
                print("new          {:3}".format( len(segments) ),[cx, cy])
                segments.append([cx,cy])

        except (IndexError, TypeError):
            # no previous
            print("first/missing   {:3}".format( len(segments) ), cx, cy)
            segments.append([cx,cy])

    else:
        # detect missing segments
        counter += 1
        if counter > 6: # TODO parameter - could be calculated from blink speed and video frame rate
            print("missing segment around", fn-counter, fn)
            segments.append(None)
            counter = 0


    # show detected segments in output
    sum_inv = cv.bitwise_not(sum)
    display = cv.bitwise_and(frame,frame,mask = sum_inv)

    # show info overlays
    cv.putText(display, 'frame: {}/{}'.format(fn, fc),
        ( 10,20), cv.FONT_HERSHEY_SIMPLEX, .5, (0,255,0), 1, cv.LINE_AA)

    cv.putText(display, 'segments: {}'.format( len(segments) ),
        (180,20), cv.FONT_HERSHEY_SIMPLEX, .5, (0,255,0), 1, cv.LINE_AA)

    if count:
        # green
        cv.putText(display, 'c: {}'.format(count),
            ( 10,40), cv.FONT_HERSHEY_SIMPLEX, .5, (0,255,0), 1, cv.LINE_AA)
    else:
        # red (nothing detected, could be missing segment)
        cv.putText(display, 'c: {}'.format(count),
            ( 10,40), cv.FONT_HERSHEY_SIMPLEX, .5, (0,0,255), 1, cv.LINE_AA)


    # label segments
    fontFace = cv.FONT_HERSHEY_SIMPLEX
    fontScale = .5
    thickness = 1
    linetype = cv.LINE_8 #cv.LINE_AA

    for i, s in enumerate(segments):
        try:
            # enumerate
            text = '{}'.format( i )
            retval, baseLine = cv.getTextSize( text, fontFace, fontScale, thickness )
            cv.putText(display, text,
                (segments[i][0]-int(retval[0]/2), segments[i][1]), fontFace, fontScale, (0,255,0), thickness, linetype)
            # COM/SEG address
            text = '{}.{}'.format( i//4, i%4 )
            retval, baseLine = cv.getTextSize( text, fontFace, fontScale, thickness )
            cv.putText(display, text,
                (segments[i][0]-int(retval[0]/2), segments[i][1]+int(retval[1])), fontFace, fontScale, (255,0,255), thickness, linetype)
        except TypeError as e:
            # None encountered
            pass

    # display processing steps
    cv.imshow("Current Frame", frame_gray)
    cv.imshow("difference", difference)
    cv.imshow("thresholded", thresholded)
    cv.imshow("filtered", filtered)
    cv.imshow("sum", sum)
    cv.imshow("display", display)

    # save
    if False:
        # lossless frames - use e.g. ffmpeg to create video
        cv.imwrite( "output/{:04}_g.png".format(fn), frame_gray)
        # cv.imwrite( "output/{:04}_d.png".format(fn), difference)
        # cv.imwrite( "output/{:04}_t.png".format(fn), thresholded)
        # cv.imwrite( "output/{:04}_f.png".format(fn), filtered)
        # cv.imwrite( "output/{:04}_s.png".format(fn), sum)
        # cv.imwrite( "output/{:04}.png".format(fn), display)

    # left-click on segment to print number to console; right-click for newline
    cv.setMouseCallback('display', mouse)

# mouse callback function (left-click on segment to print number to console; right-click for newline)
def mouse(event,x,y,flags,param):
    # find closest segment in range and print segment number to console
    if event == cv.EVENT_LBUTTONUP:
        # calculate distances between the mouse click and all segments
        ds = []
        for i,s in enumerate(segments):
            sx,sy = s[:2]
            dx = x-sx
            dy = y-sy
            d = (dx**2 + dy**2)**.5
            ds.append([i,d])
        # find the smallest distance, i.e. the closest segment
        m = min(ds, key=lambda _:_[1])
        # print index if clicked close enough
        if m[1] < 20:   # TODO parameter click distance in px
            print("{}, ".format(m[0]), end="" )

    # new line on RMB
    if event == cv.EVENT_RBUTTONUP:
        print()



def state_machine(state, key):
    """ determine state change from key code """
    states = {
        "pause" : {"esc":"quit", "space":"play", "right":"next", "left":"previous" },
        "play"  : {"esc":"quit", "space":"pause"},
        "end"   : {"esc":"quit"                 },
        "quit"  : {},
    }
    keys = {
        27:"esc",
        13:"enter",     # not used
        32:"space",
        115:"s",        # not used
        2424832:"left", # not used
        2555904:"right",
    }

    if key > 0:
        # key pressed
        try:
            # print("key", key, "=", keys[key])
            # print("state", state, "->", states[state][keys[key]])
            state = states[state][keys[key]]
        except KeyError as e:
            print("unknown key", key)
    else:
        # no key pressed
        if state == "next":
            # just process one frame (pause automatically)
            state = "pause"

    return state



def versions():
    """ return version strings """
    p = "Python " + sys.version
    n = "NumPy  " + np.version.version
    o = "OpenCV " + cv.__version__
    return "\n".join([p,n,o]) + "\n"



if __name__ == "__main__":
    main()
